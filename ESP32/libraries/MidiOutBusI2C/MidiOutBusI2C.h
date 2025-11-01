#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "I2cOutManager.h"

// Nota importante:
// Este header asume que ya existen métodos internos tipo noteOn(), noteOff(), cc(), pb()
// Si tus nombres son distintos, mapea abajo en los wrappers ★ añadidos.

class MidiOutBusI2C {
public:
  // Mantén tus constructores originales. Aquí añadimos dos comunes:
  explicit MidiOutBusI2C(I2cOutManager& outMan) : manager(outMan) {}
  explicit MidiOutBusI2C(TwoWire& bus) : tempManager(bus), manager(tempManager) {}

  // --------------------------------------------------------------------------
  // API interna 
  // --------------------------------------------------------------------------
  // Estas son DECLARACIONES para que compilen los wrappers.
  // Si ya estaban en tu .h original, déjalas tal cual y elimina estas duplicadas.
  void noteOn(uint8_t ch, uint8_t note, uint8_t vel);
  void noteOff(uint8_t ch, uint8_t note, uint8_t vel);
  void cc(uint8_t ch, uint8_t num, uint8_t val);
  void pb(uint8_t ch, uint16_t v14);


  inline void sendNoteOn(uint8_t ch, uint8_t note, uint8_t vel)  { noteOn(ch, note, vel); }
  inline void sendNoteOff(uint8_t ch, uint8_t note, uint8_t vel) { noteOff(ch, note, vel); }

  inline void sendControlChange(uint8_t ch, uint8_t num, uint8_t val) { cc(ch, num, val); }

  inline void sendPitchBend(uint8_t ch, uint16_t v14) { pb(ch, v14); }


  I2cOutManager& getManager() { return manager; }

private:
  
  I2cOutManager tempManager;
  
  I2cOutManager& manager;
};
