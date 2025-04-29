#include "SensorManager.h"

SensorManager::SensorManager() {}

void SensorManager::addSoftPot(uint8_t pin) {
    _softPots.emplace_back(pin);
}

void SensorManager::addFSR(uint8_t pin) {
    _fsrs.emplace_back(pin);
}

void SensorManager::begin() {
    for (auto& softPot : _softPots) {
        softPot.begin();
    }

    for (auto& fsr : _fsrs) {
        fsr.begin();
    }
}

void SensorManager::update() {
    for (auto& softPot : _softPots) {
        softPot.update();
    }

    for (auto& fsr : _fsrs) {
        fsr.update();
    }
}

uint16_t SensorManager::getSoftPotRaw(uint8_t index) const {
    if (index < _softPots.size()) {
        return _softPots[index].getRawValue();
    }
    return 0;
}

uint16_t SensorManager::getSoftPotFiltered(uint8_t index) const {
    if (index < _softPots.size()) {
        return _softPots[index].getFilteredValue();
    }
    return 0;
}

uint16_t SensorManager::getFSRRaw(uint8_t index) const {
    if (index < _fsrs.size()) {
        return _fsrs[index].getRawValue();
    }
    return 0;
}

uint16_t SensorManager::getFSRFiltered(uint8_t index) const {
    if (index < _fsrs.size()) {
        return _fsrs[index].getFilteredValue();
    }
    return 0;
}
