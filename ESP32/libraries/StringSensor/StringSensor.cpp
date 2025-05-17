#include "StringSensor.h"
#include <Config.h>  // Para constantes como FSR_THRESHOLD y DEBOUNCE_TIME_MS
#include <Arduino.h>

void StringSensor::begin(uint8_t sIdx, uint8_t fIdx, uint8_t dIdx,
                         uint8_t nMin, uint8_t nMax) {
    softpotIndex = sIdx;
    fsrIndex = fIdx;
    dotfsrIndex = dIdx;
    noteMin = nMin;
    noteMax = nMax;
}

void StringSensor::setCallbacks(void (*noteOn)(uint8_t, uint8_t),
                                void (*noteOff)(uint8_t),
                                void (*modulation)(uint8_t)) {
    cbNoteOn = noteOn;
    cbNoteOff = noteOff;
    cbModulation = modulation;
}

void StringSensor::update(const uint16_t* sensorValues, uint32_t now) {
    uint16_t softpot = sensorValues[softpotIndex];
    uint16_t fsr = sensorValues[fsrIndex];
    uint16_t dotfsr = sensorValues[dotfsrIndex];

    uint8_t note = map(softpot, 0, 4095, noteMin, noteMax);
    uint8_t velocity = map(dotfsr, FSR_THRESHOLD, 4095, 1, 127);
    velocity = constrain(velocity, 1, 127);
    uint8_t mod = map(fsr, 0, 4095, 0, 127);

    if (cbModulation) cbModulation(mod);  // Envía CC o modulación continua

    if (dotfsr > FSR_THRESHOLD) {
        if (!isPressed && (now - lastChangeTime > DEBOUNCE_TIME_MS)) {
            if (cbNoteOn) cbNoteOn(note, velocity);
            isPressed = true;
            currentNote = note;
            lastChangeTime = now;
        }
    } else {
        if (isPressed && (now - lastChangeTime > DEBOUNCE_TIME_MS)) {
            if (cbNoteOff) cbNoteOff(currentNote);
            isPressed = false;
            lastChangeTime = now;
        }
    }
}
