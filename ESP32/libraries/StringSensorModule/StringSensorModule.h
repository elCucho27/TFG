#ifndef STRING_SENSOR_MODULE_H
#define STRING_SENSOR_MODULE_H

#include "StringSensor.h"
#include <Wire.h>

class StringSensorModule {
public:
    void begin(uint8_t address, TwoWire& wire = Wire);
    void update();

    void setNoteCallbacks(void (*noteOn)(uint8_t, uint8_t),
                          void (*noteOff)(uint8_t),
                          void (*modulation)(uint8_t));

private:
    static const uint8_t NUM_STRINGS = 3;
    static const uint8_t NUM_SENSORS = 9;
    StringSensor cuerdas[NUM_STRINGS];
    uint16_t sensorValues[NUM_SENSORS] = {0};
    uint8_t i2cAddress;
    TwoWire* i2cBus;
};
#endif 