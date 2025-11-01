#ifndef SENSOR_PROCESSOR_H
#define SENSOR_PROCESSOR_H

#include <Arduino.h>
#include <vector>

class SensorProcessor {
public:
  virtual ~SensorProcessor() {}

  // Interfaz base que usan Apolo3/Orpheus e I2cInManager:
  virtual void update(const std::vector<uint16_t>& raw) = 0;
  virtual void printDebug(uint8_t address) const = 0;
  virtual bool hasChanged(uint16_t threshold) const = 0;
  virtual void markAsSent() = 0;

  // Helper opcional para mapeos a MIDI (si tu sketch lo usa):
  // por defecto devuelve 0.0f y las derivadas pueden sobrescribirlo.
  virtual float getNormalized(uint8_t index) { (void)index; return 0.0f; }
};

#endif
