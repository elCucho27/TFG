#include "FSRReader.h"

FSRReader::FSRReader(uint8_t pin, uint8_t windowSize, FilterType filter)
    : _pin(pin), _windowSize(windowSize), _filterType(filter), _smoothed() {
}

void FSRReader::begin() {
    pinMode(_pin, INPUT);

    if (_filterType == AVERAGE) {
        _smoothed.begin(SMOOTHED_AVERAGE, _windowSize);
    } else if (_filterType == EXPONENTIAL) {
        _smoothed.begin(SMOOTHED_EXPONENTIAL, _windowSize);
    }

    uint16_t firstReading = analogRead(_pin);
    firstReading = constrain(firstReading, 0, 4095);

    _smoothed.add(firstReading);
    _filteredVal = firstReading;
    _rawVal = firstReading;
}

void FSRReader::update() {
    uint16_t reading = analogRead(_pin);
    reading = constrain(reading, 0, 4095);

    _smoothed.add(reading);
    _filteredVal = _smoothed.get();
    _rawVal = reading;
}

uint16_t FSRReader::getRawValue() const {
    return _rawVal;
}

uint16_t FSRReader::getFilteredValue() const {
    return _filteredVal;
}

float FSRReader::getNormalizedValue() const {
    return (float)_filteredVal / 4095.0f;
}
