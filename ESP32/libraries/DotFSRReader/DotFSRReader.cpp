#include "DotFSRReader.h"

DotFSRReader::DotFSRReader(uint8_t pin)
: _pin(pin),
  _filteredVal(0),
  _rawVal(0) {
}

void DotFSRReader::begin() {
    pinMode(_pin, INPUT);
    _smoothed.begin(SMOOTHED_AVERAGE, 3);      // Media de 3 lecturas
    _filteredVal = analogRead(_pin);           // Primer valor para iniciar el filtro
}

void DotFSRReader::update() {
    uint16_t reading = analogRead(_pin);
    _smoothed.add(reading);
    _filteredVal = _smoothed.get();
    _rawVal = reading;
}

uint16_t DotFSRReader::getValue() const {
    return _filteredVal;
}

uint16_t DotFSRReader::getRawValue() const {
    return _rawVal;
}
