#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <vector>
#include "SoftPotReader.h"
#include "FSRReader.h"

class SensorManager {
public:
    SensorManager();

    void addSoftPot(uint8_t pin);
    void addFSR(uint8_t pin);

    void begin();
    void update();

    uint16_t getSoftPotRaw(uint8_t index) const;
    uint16_t getSoftPotFiltered(uint8_t index) const;

    uint16_t getFSRRaw(uint8_t index) const;
    uint16_t getFSRFiltered(uint8_t index) const;

    uint8_t getSoftPotCount() const { return _softPots.size(); }
    uint8_t getFSRCount() const { return _fsrs.size(); }

private:
    std::vector<SoftPotReader> _softPots;
    std::vector<FSRReader> _fsrs;
};

#endif // SENSOR_MANAGER_H
