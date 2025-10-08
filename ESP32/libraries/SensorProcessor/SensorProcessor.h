#ifndef SENSOR_PROCESSOR_H
#define SENSOR_PROCESSOR_H

#include <Arduino.h>
#include <vector>

class SensorProcessor {
public:
  virtual void update(const std::vector<uint16_t>& raw) = 0;
  virtual void printDebug(uint8_t address) const = 0;
  virtual bool hasChanged(uint16_t threshold) const = 0;
  virtual void markAsSent() = 0;

  virtual ~SensorProcessor() {}
};

#endif
