// SoftPotReader.cpp
#include "SoftPotReader.h"

SoftPotReader::SoftPotReader(uint8_t pin)
: _pin(pin),
  _filteredVal(0),
  _rawVal(0) {
    
}

void SoftPotReader::begin() {
    pinMode(_pin, INPUT);                     // Configuramos el pin como entrada analógica.
    _smoothed.begin(SMOOTHED_AVERAGE, 3);      // Inicializamos el filtro de promedio a 3 muestras.
    _filteredVal = analogRead(_pin);           // Primera lectura inicial para arrancar.
}

void SoftPotReader::update() {
    uint16_t reading = analogRead(_pin);       // Leer el valor analógico.
    _smoothed.add(reading);                    // Añadir al filtro de promedio.
    _filteredVal = _smoothed.get();             // Actualizar el valor filtrado.
    _rawVal = reading;                         // Guardar el valor crudo también.
}

uint16_t SoftPotReader::getValue() const {
    return _filteredVal;                      // Devuelve el valor ya filtrado.
}

uint16_t SoftPotReader::getRawValue() const {
    return _rawVal;                           // Devuelve el último valor bruto leído (opcional).
}
