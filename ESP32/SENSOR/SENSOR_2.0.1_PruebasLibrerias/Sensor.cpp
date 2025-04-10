#include "Sensor.h"
#include <Arduino.h>

// Constructor: inicializa el sensor y configura el pin como entrada.
Sensor::Sensor(int pin, const char* oscAddress, int minValue, int maxValue, float alpha)
  : _pin(pin),                      // Inicializa _pin con el valor del parámetro pin
    _oscAddress(oscAddress),        // Inicializa _oscAddress con el valor de oscAddress
    _minValue(minValue),            // Inicializa _minValue con el valor de minValue
    _maxValue(maxValue),            // Inicializa _maxValue con el valor de maxValue
    _alpha(alpha),                  // Inicializa _alpha con el valor de alpha
    _filteredValue(0),              // Inicializa _filteredValue en 0
    _lastSentValue(-1)              // Inicializa _lastSentValue en -1, para asegurar que cualquier cambio sea detectado
  {
  pinMode(_pin, INPUT); // Configura el pin como entrada
}

void Sensor::update() {
  int raw = analogRead(_pin);                      // Lee el valor crudo
  float normalized = normalize(raw);               // Lo normaliza
  _filteredValue = (_alpha * normalized) + ((1.0 - _alpha) * _filteredValue); // Filtro exponencial
}

bool Sensor::hasChanged() {
  float diff = abs(_filteredValue - _lastSentValue); // Calcula la diferencia con el último valor enviado
  if (diff > 0.01) {                                 // Si cambia más de cierto umbral (1%)
    _lastSentValue = _filteredValue;
    return true;
  }
  return false;
}

float Sensor::getValue() {
  return _filteredValue; // Devuelve el valor actual filtrado
}

const char* Sensor::getAddress() {
  return _oscAddress; // Devuelve la ruta OSC asociada
}

float Sensor::normalize(int raw) {
  raw = constrain(raw, _minValue, _maxValue); // Asegura que esté en rango
  return (float)(raw - _minValue) / (_maxValue - _minValue); // Normaliza entre 0 y 1
}
