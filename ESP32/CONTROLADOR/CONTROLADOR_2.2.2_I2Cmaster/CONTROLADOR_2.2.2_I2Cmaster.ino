#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// === CONFIGURACIÓN GENERAL ===
#define SLAVE_ADDR 0x08             // Dirección I2C del esclavo
#define REQUEST_INTERVAL 10         // Intervalo de lectura en ms
#define NUM_SENSORS 6               // Total de sensores (3 SoftPot + 3 FSR)
#define FSR_THRESHOLD 3300          // Umbral de activación FSR
#define NUM_PAIRS 3                 // Número de parejas SoftPot+FSR
#define MIDI_CHANNEL 1              // Canal MIDI (1–16)
#define DEBOUNCE_TIME 50            // ms para evitar rebotes

// Define los pines I2C (ajusta según tu hardware)
#define I2C_SDA 8
#define I2C_SCL 9

uint32_t lastRequestTime = 0;       // Para controlar la frecuencia de lectura

// Instancia del dispositivo MIDI USB
Adafruit_USBD_MIDI usb_midi;

// Instancia de la librería MIDI usando Adafruit_USBD_MIDI como transporte
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// === Estructura de cada pareja SoftPot + FSR ===
struct SensorPair {
    uint8_t softpotIndex;
    uint8_t fsrIndex;
    uint8_t noteMin;
    uint8_t noteMax;
    bool isPressed;
    uint8_t currentNote;
    uint32_t lastChangeTime;
};

// === Configuración de las parejas ===
SensorPair sensorPairs[NUM_PAIRS] = {
    {0, 3, 40, 64, false, 0, 0},    // SoftPot 0 + FSR 3 → E2–E4
    {1, 4, 45, 69, false, 0, 0},    // SoftPot 1 + FSR 4 → A2–A4
    {2, 5, 38, 62, false, 0, 0}     // SoftPot 2 + FSR 5 → D2–D4
};

uint16_t sensorValues[NUM_SENSORS] = {0}; // Valores de sensores

void setup() {
    Serial.begin(115200);
    delay(100);

    // Inicializa I2C como maestro con pines definidos
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.print("[DEBUG] I2C iniciado en SDA: ");
    Serial.print(I2C_SDA);
    Serial.print(" SCL: ");
    Serial.println(I2C_SCL);

    usb_midi.begin();                   // Inicializa MIDI USB
    unsigned long startTime = millis();

    while (!TinyUSBDevice.mounted()) {
    if (millis() - startTime > 5000) { // Espera máxima de 5 segundos
        Serial.println("[ERROR] Dispositivo USB no montado.");
        break;
    }
    delay(10);
}

    MIDI.begin(MIDI_CHANNEL_OMNI);      // Inicializa MIDI Library
    Serial.println("[MASTER] Maestro I2C + MIDI USB inicializado.");
}

void loop() {
    uint32_t now = millis();
    if (now - lastRequestTime >= REQUEST_INTERVAL) {
        lastRequestTime = now;

        requestSensorData();              // Leer sensores

        for (uint8_t i = 0; i < NUM_PAIRS; i++) {
            processSensorPair(sensorPairs[i]);
        }
    }
}

// === Función que solicita datos al esclavo y llena sensorValues[] ===
void requestSensorData() {
    uint8_t numBytesToRequest = NUM_SENSORS * 2; // 2 bytes por sensor
    Wire.requestFrom(SLAVE_ADDR, numBytesToRequest);

    uint8_t i = 0;
    while (Wire.available() >= 2 && i < NUM_SENSORS) {
        uint8_t highByte = Wire.read();
        uint8_t lowByte = Wire.read();
        sensorValues[i] = (highByte << 8) | lowByte;
        i++;
    }

    if (i < NUM_SENSORS) {
        Serial.print("[ERROR] Lectura incompleta de sensores. Esperados: ");
        Serial.print(NUM_SENSORS);
        Serial.print(" Leídos: ");
        Serial.println(i);
    } else {
        Serial.print("[DEBUG] Lecturas de sensores: ");
        for (uint8_t j = 0; j < NUM_SENSORS; j++) {
            Serial.print(sensorValues[j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

// === Función que procesa cada pareja de sensores ===
void processSensorPair(SensorPair& pair) {
    uint16_t softpotValue = sensorValues[pair.softpotIndex];
    uint16_t fsrValue = sensorValues[pair.fsrIndex];
    uint32_t now = millis();

    uint8_t mappedNote = map(softpotValue, 0, 4095, pair.noteMin, pair.noteMax);
    uint8_t velocity = map(fsrValue, FSR_THRESHOLD, 4095, 1, 127);
    velocity = constrain(velocity, 1, 127);

    if (fsrValue > FSR_THRESHOLD) {
        if (!pair.isPressed && (now - pair.lastChangeTime) > DEBOUNCE_TIME) {
            sendNoteOn(mappedNote, velocity);
            pair.isPressed = true;
            pair.currentNote = mappedNote;
            pair.lastChangeTime = now;

            Serial.print("[MASTER] MIDI NOTE ON: ");
            Serial.print(mappedNote);
            Serial.print(" Velocity: ");
            Serial.println(velocity);
        }
    } else {
        if (pair.isPressed && (now - pair.lastChangeTime) > DEBOUNCE_TIME) {
            sendNoteOff(pair.currentNote);
            pair.isPressed = false;
            pair.lastChangeTime = now;

            Serial.print("[MASTER] MIDI NOTE OFF: ");
            Serial.println(pair.currentNote);
        }
    }
}

// === Funciones MIDI utilizando MIDI Library ===
void sendNoteOn(uint8_t note, uint8_t velocity) {
    MIDI.sendNoteOn(note, velocity, MIDI_CHANNEL);
}

void sendNoteOff(uint8_t note) {
    MIDI.sendNoteOff(note, 0, MIDI_CHANNEL);
}
