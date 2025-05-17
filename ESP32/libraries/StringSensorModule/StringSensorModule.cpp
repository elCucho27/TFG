#include "StringSensorModule.h"
#include "Config.h"
#include <Arduino.h>

void StringSensorModule::begin(uint8_t address, TwoWire& wire) {
    i2cAddress = address;
    i2cBus = &wire;

    // Asignar sensores a cada cuerda: SoftPot, FSR largo, DotFSR
    cuerdas[0].begin(0, 3, 6, 40, 64);
    cuerdas[1].begin(1, 4, 7, 45, 69);
    cuerdas[2].begin(2, 5, 8, 38, 62);
}

void StringSensorModule::setNoteCallbacks(void (*noteOn)(uint8_t, uint8_t),
                                    void (*noteOff)(uint8_t),
                                    void (*modulation)(uint8_t)) {
    for (auto& c : cuerdas) {
        c.setCallbacks(noteOn, noteOff, modulation);
    }
}

void StringSensorModule::update() {
    uint8_t bytesToRequest = NUM_SENSORS * 2;
    i2cBus->requestFrom(i2cAddress, bytesToRequest);

    Serial.printf("[I2C] Solicitando %d bytes a esclavo 0x%02X\n", bytesToRequest, i2cAddress);

    uint8_t i = 0;
    while (i2cBus->available() >= 2 && i < NUM_SENSORS) {
        uint8_t hi = i2cBus->read();
        uint8_t lo = i2cBus->read();
        sensorValues[i++] = (hi << 8) | lo;
    }

    if (i < NUM_SENSORS) {
        Serial.printf("[ERROR] Lectura incompleta: %d/%d\n", i, NUM_SENSORS);
    } else {
        Serial.println("[I2C] Lectura completa.");
    }

    uint32_t now = millis();
    for (auto& c : cuerdas) {
        c.update(sensorValues, now);
    }
}

