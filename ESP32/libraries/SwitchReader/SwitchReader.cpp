#include "SwitchReader.h"
#include <Arduino.h>

// Constructor: inicializa los valores y guarda el pin
SwitchReader::SwitchReader(uint8_t digitalPin)
  : _pin(digitalPin), _state(0), _lastState(0), changed(false) {}

// Inicializa el pin como entrada con pull-up (lógica activa en LOW)
void SwitchReader::begin() {
  pinMode(_pin, INPUT_PULLUP);
  _lastState = digitalRead(_pin);
  _state = _lastState;
}

// Lee el estado actual del pin y detecta si ha cambiado
void SwitchReader::update() {
  _state = digitalRead(_pin);
  changed = (_state != _lastState);   // Flag de cambio si el estado varía
  _lastState = _state;
}

// Devuelve el valor lógico como 1 (activo) o 0 (inactivo)
uint16_t SwitchReader::getValue() const {
  return (_state == LOW) ? 1 : 0;   // Activo en LOW
}

// Devuelve true si el estado cambió en la última lectura
bool SwitchReader::hasChanged() const {
  return changed;
}
