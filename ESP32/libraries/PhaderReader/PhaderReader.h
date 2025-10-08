#ifndef PHADER_READER_H
#define PHADER_READER_H

#include <Arduino.h>
#include <Smoothed.h>
#include "ISensor.h"   // ¡Importante! Incluir ISensor para heredar de ella

class PhaderReader : public ISensor {
  public:
    PhaderReader(uint8_t pin);

    void begin() override;
    void update() override;
    uint16_t getValue() const override;

    uint16_t getRawValue() const;

  private:
    uint8_t _pin;
    uint16_t _filteredVal;
    uint16_t _rawVal;
    Smoothed<uint16_t> _smoothed;
};

#endif
