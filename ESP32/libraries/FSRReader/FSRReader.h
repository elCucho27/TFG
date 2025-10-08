#ifndef FSR_READER_H
#define FSR_READER_H

#include <Arduino.h>
#include <Smoothed.h>
#include "ISensor.h"   // ¡Importante! Incluir ISensor para heredar de ella

/**
 *  Clase para leer sensores de tipo FSR.
 */
class FSRReader : public ISensor {  
public:
    FSRReader(uint8_t pin);

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

#endif // FSR_READER_H
