#include "SoftPotReader.h"

SoftPotReader::SoftPotReader(uint8_t pin) : _pin(pin), _filteredVal(0), _rawVal(0) {}

void SoftPotReader::begin() {
  pinMode(_pin, INPUT);
  _smoothed.begin(SMOOTHED_AVERAGE, 3); // Usamos promedio de 3 lecturas
  _filteredVal = analogRead(_pin);  // Primer valor inicial
}

void SoftPotReader::update() {
  // Leer el valor actual del sensor
  uint16_t reading = analogRead(_pin);
  
  // Añadir esta nueva lectura al buffer de suavizado
  _smoothed.add(reading);

  // Actualizar los atributos
  _filteredVal = _smoothed.get();  // Valor suavizado
  _rawVal = reading;               // Valor bruto actual
}


uint16_t SoftPotReader::getFilteredValue() const {
  return _filteredVal;
}

uint16_t SoftPotReader::getRawValue() const {
  return _rawVal;
}
