#ifndef ORPHEUS_PROCESSOR_H
#define ORPHEUS_PROCESSOR_H

#include "SensorProcessor.h"

class OrpheusProcessor : public SensorProcessor {
public:
  OrpheusProcessor();

  void update(const std::vector<uint16_t>& raw) override;
  void printDebug(uint8_t address) const override;
  bool hasChanged(uint16_t threshold) const override;
  void markAsSent() override;

private:
  uint16_t values[7];
  uint16_t lastValues[7];
};

#endif
