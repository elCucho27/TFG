#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "Config.h"

// Instancia del dispositivo MIDI USB
Adafruit_USBD_MIDI usb_midi;
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

SensorPair sensorPairs[NUM_PAIRS] = {
    {0, 3, 40, 64, false, 0, 0},
    {1, 4, 45, 69, false, 0, 0},
    {2, 5, 38, 62, false, 0, 0}
};

uint16_t sensorValues[NUM_SENSORS] = {0};
uint32_t lastRequestTime = 0;

void setup() {
    setupHardware();
    setupSoftware();
}

void setupHardware() {
    Serial.begin(115200);
    delay(10);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf("[MASTER] I2C iniciado SDA: %d SCL: %d\n", I2C_SDA_PIN, I2C_SCL_PIN);

    usb_midi.begin();

    unsigned long startTime = millis();
    while (!TinyUSBDevice.mounted()) {
        if (millis() - startTime > 5000) {
            Serial.println("[ERROR] USB no montado.");
            break;
        }
        delay(10);
    }
}

void setupSoftware() {
    MIDI.begin(MIDI_CHANNEL_OMNI);
    Serial.println("[MASTER] MIDI USB inicializado.");
}

void loop() {
    uint32_t now = millis();
    if (now - lastRequestTime >= REQUEST_INTERVAL_MS) {
        lastRequestTime = now;

        requestSensorData();

        for (uint8_t i = 0; i < NUM_PAIRS; ++i) {
            processSensorPair(sensorPairs[i]);
        }
    }
}

void requestSensorData() {
    uint8_t bytesToRequest = NUM_SENSORS * 2;
    Wire.requestFrom(SLAVE_ADDR, bytesToRequest);

    uint8_t i = 0;
    while (Wire.available() >= 2 && i < NUM_SENSORS) {
        uint8_t highByte = Wire.read();
        uint8_t lowByte = Wire.read();
        sensorValues[i] = (highByte << 8) | lowByte;
        i++;
    }

    if (i < NUM_SENSORS) {
        Serial.printf("[ERROR] Lectura incompleta. Esperados: %d, Leídos: %d\n", NUM_SENSORS, i);
    }
}

void processSensorPair(SensorPair& pair) {
    uint16_t softpot = sensorValues[pair.softpotIndex];
    uint16_t fsr = sensorValues[pair.fsrIndex];
    uint32_t now = millis();

    uint8_t mappedNote = map(softpot, 0, 4095, pair.noteMin, pair.noteMax);
    uint8_t velocity = map(fsr, FSR_THRESHOLD, 4095, 1, 127);
    velocity = constrain(velocity, 1, 127);

    if (fsr > FSR_THRESHOLD) {
        if (!pair.isPressed && (now - pair.lastChangeTime) > DEBOUNCE_TIME_MS) {
            MIDI.sendNoteOn(mappedNote, velocity, MIDI_CHANNEL);
            pair.isPressed = true;
            pair.currentNote = mappedNote;
            pair.lastChangeTime = now;
        }
    } else {
        if (pair.isPressed && (now - pair.lastChangeTime) > DEBOUNCE_TIME_MS) {
            MIDI.sendNoteOff(pair.currentNote, 0, MIDI_CHANNEL);
            pair.isPressed = false;
            pair.lastChangeTime = now;
        }
    }
}
