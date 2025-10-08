// FSRReader.cpp
#include "FSRReader.h"  


FSRReader::FSRReader(uint8_t pin)
: _pin(pin),
  _filteredVal(0),
  _rawVal(0) {
}

void FSRReader::begin() {
    pinMode(_pin, INPUT);                    // Configuramos el pin analógico como entrada
    _smoothed.begin(SMOOTHED_AVERAGE, 3);     // Inicializamos el filtro de promedio (media de 3 muestras)
    _filteredVal = analogRead(_pin);          // Hacemos una primera lectura para arrancar el sistema de filtrado
}

void FSRReader::update() {
    uint16_t reading = analogRead(_pin);      // Leemos el valor analógico actual del pin
    _smoothed.add(reading);                   // Lo añadimos al filtro de promedio
    _filteredVal = _smoothed.get();            // Obtenemos el valor promedio filtrado
    _rawVal = reading;                        // También guardamos la última lectura bruta (sin filtrar)
}

uint16_t FSRReader::getRawValue() const {
    return _rawVal;                           // Devuelve el último valor directamente leído del ADC
}


uint16_t FSRReader::getValue() const {
    return _filteredVal;                      // Devuelve el valor ya filtrado.
}