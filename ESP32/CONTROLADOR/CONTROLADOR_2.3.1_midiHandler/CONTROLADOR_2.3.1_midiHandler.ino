#include <Wire.h>  // Librería para comunicación I2C
#include <Adafruit_TinyUSB.h>  // Manejo de USB para dispositivos compatibles, incluyendo MIDI USB
#include <MIDI.h>  // Librería MIDI estándar para envío de mensajes
#include "Config.h"  // Archivo de configuración donde defines constantes como pines y direcciones

// === Instancia del dispositivo MIDI USB ===
Adafruit_USBD_MIDI usb_midi;  // Instancia de dispositivo MIDI sobre USB
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);  // Crea instancia MIDI que usará la librería MIDI sobre usb_midi

// === Estructura para representar una "cuerda" ===
// Cada "string" tiene: 1 SoftPot, 1 FSR largo, 1 Dot FSR, y un rango de notas
struct SensorGroup {
    uint8_t softpotIndex;     // Índice en el array de sensores para el SoftPot (posición)
    uint8_t fsrIndex;         // Índice para el FSR largo (presión/modulación)
    uint8_t dotfsrIndex;      // Índice para el Dot FSR (disparo de nota)
    uint8_t noteMin;          // Nota mínima asignada a esta cuerda
    uint8_t noteMax;          // Nota máxima asignada
    bool isPressed;           // Estado actual: ¿hay nota activa?
    uint8_t currentNote;      // Última nota activada (necesario para el NoteOff)
    uint32_t lastChangeTime;  // Momento del último cambio de estado (para debounce)
};

// === Configuración de 3 cuerdas/sensores con sus rangos y sensores asociados ===
SensorGroup stringSensor[NUM_STRINGS] = {
    {0, 5, 6, 40, 64, false, 0, 0},
    {1, 4, 7, 45, 69, false, 0, 0},
    {2, 3, 8, 38, 62, false, 0, 0}
};

// === Array donde se guardarán las lecturas analógicas recibidas del módulo sensor ===
uint16_t sensorValues[NUM_SENSORS] = {0};

// === Marca temporal para controlar cada cuánto se hace una lectura I2C ===
uint32_t lastRequestTime = 0;

// === Función principal de inicialización ===
void setup() {
    setupHardware();  // Inicializa hardware: Serial, I2C, MIDI USB
    setupSoftware();  // Inicializa la lógica MIDI
}

// === Inicialización de hardware (USB, I2C, Serial) ===
void setupHardware() {
    Serial.begin(115200);  // Puerto serie para depuración
    delay(10);  // Pequeño retardo para estabilizar

    // Inicializa el bus I2C como maestro usando los pines definidos en Config.h
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf("[MASTER] I2C iniciado SDA: %d SCL: %d\n", I2C_SDA_PIN, I2C_SCL_PIN);

    usb_midi.begin();  // Inicializa la interfaz MIDI USB

    // Espera a que el dispositivo USB esté montado, con timeout
    unsigned long startTime = millis();
    while (!TinyUSBDevice.mounted()) {
        if (millis() - startTime > 5000) {
            Serial.println("[ERROR] USB no montado.");
            break;
        }
        delay(10);
    }
}

// === Inicialización de la lógica MIDI ===
void setupSoftware() {
    MIDI.begin(MIDI_CHANNEL_OMNI);  // Escucha en todos los canales MIDI
    Serial.println("[MASTER] MIDI USB inicializado.");
}

// === Bucle principal: cada intervalo solicita datos al esclavo y procesa sensores ===
void loop() {
    uint32_t now = millis();  // Tiempo actual

    // ¿Ha pasado suficiente tiempo desde la última lectura?
    if (now - lastRequestTime >= REQUEST_INTERVAL_MS) {
        lastRequestTime = now;

        requestSensorData();  // Solicita los datos por I2C

        // Procesa cada conjunto SoftPot + FSR + DotFSR
        for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
            stringProcessor(stringSensor[i]);
        }
    }
}

// === Función que solicita datos al esclavo I2C y los guarda en sensorValues[] ===
void requestSensorData() {
    uint8_t bytesToRequest = NUM_SENSORS * 2;  // Cada sensor se transmite en 2 bytes (uint16_t)
    Wire.requestFrom(SLAVE_ADDR, bytesToRequest);  // Solicita datos al esclavo

    uint8_t i = 0;
    // Lee los datos del bus I2C mientras haya al menos 2 bytes disponibles por sensor
    while (Wire.available() >= 2 && i < NUM_SENSORS) {
        uint8_t highByte = Wire.read();
        uint8_t lowByte = Wire.read();
        sensorValues[i] = (highByte << 8) | lowByte;  // Reconstruye el uint16_t (12 bits reales)
        i++;
    }

    // Verifica si la lectura fue completa
    if (i < NUM_SENSORS) {
        Serial.printf("[ERROR] Lectura incompleta. Esperados: %d, Leídos: %d\n", NUM_SENSORS, i);
    }
}

// === Procesamiento de una cuerda/sensor ===
void stringProcessor(SensorGroup& stringSensor) {
    // Lecturas individuales de los sensores asociados
    uint16_t softpot = sensorValues[stringSensor.softpotIndex];
    uint16_t fsrMod = sensorValues[stringSensor.fsrIndex];
    uint16_t dotFsr = sensorValues[stringSensor.dotfsrIndex];
    uint32_t now = millis();

    // === Cálculo de nota MIDI basado en posición SoftPot ===
    uint8_t mappedNote = map(softpot, 0, 4095, stringSensor.noteMin, stringSensor.noteMax);

    // === Cálculo de velocidad MIDI a partir del DotFSR ===
    uint8_t velocity = map(dotFsr, FSR_THRESHOLD, 4095, 1, 127);
    velocity = constrain(velocity, 1, 127);  // Limita a rango MIDI válido

    // === Modulación enviada como CC1 (mod wheel) con FSR largo ===
    uint8_t modulation = map(fsrMod, 0, 4095, 0, 127);
    MIDI.sendControlChange(1, modulation, MIDI_CHANNEL);

    // === Gestión del estado de la nota (on/off con debounce) ===
    if (dotFsr > FSR_THRESHOLD) {
        // Si no estaba pulsado antes y ha pasado el tiempo de debounce
        if (!stringSensor.isPressed && (now - stringSensor.lastChangeTime) > DEBOUNCE_TIME_MS) {
            MIDI.sendNoteOn(mappedNote, velocity, MIDI_CHANNEL);
            stringSensor.isPressed = true;
            stringSensor.currentNote = mappedNote;
            stringSensor.lastChangeTime = now;
        }
    } else {
        // Si estaba pulsado y ya ha pasado el debounce
        if (stringSensor.isPressed && (now - stringSensor.lastChangeTime) > DEBOUNCE_TIME_MS) {
            MIDI.sendNoteOff(stringSensor.currentNote, 0, MIDI_CHANNEL);
            stringSensor.isPressed = false;
            stringSensor.lastChangeTime = now;
        }
    }
}
