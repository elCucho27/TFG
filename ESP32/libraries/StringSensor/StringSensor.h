#ifndef STRING_SENSOR_H
#define STRING_SENSOR_H
#include <stdint.h>
#include <Config.h>

class StringSensor {
public:
    void begin(uint8_t indexSoftpot, uint8_t indexFsr, uint8_t indexDotFsr,
               uint8_t noteMin, uint8_t noteMax);

    void update(const uint16_t* sensorValues, uint32_t now);

    void setCallbacks(void (*noteOn)(uint8_t note, uint8_t vel),
                      void (*noteOff)(uint8_t note),
                      void (*modulation)(uint8_t value));

private:
    uint8_t softpotIndex, fsrIndex, dotfsrIndex;
    uint8_t noteMin, noteMax;
    bool isPressed = false;
    uint8_t currentNote = 0;
    uint32_t lastChangeTime = 0;

    void (*cbNoteOn)(uint8_t, uint8_t) = nullptr;
    void (*cbNoteOff)(uint8_t) = nullptr;
    void (*cbModulation)(uint8_t) = nullptr;
};
#endif
