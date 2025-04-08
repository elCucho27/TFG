#include "Sensor.h"
#include <Arduino.h>

Sensor::Sensor(int pin, const char* oscAddress, int minValue, int maxValue, float alpha)
  : _pin(pin), _oscAddress(oscAddress), _minValue(minValue), _maxValue(maxValue), _alpha(alpha), _filteredValue(0), _lastSentValue(-1) {
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
