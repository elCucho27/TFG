#ifndef FSR_READER_H
#define FSR_READER_H

#include <Arduino.h>
#include <Smoothed.h>

class FSRReader {
public:
    enum FilterType {
        AVERAGE,
        EXPONENTIAL
    };

    FSRReader(uint8_t pin, uint8_t windowSize = 5, FilterType filter = AVERAGE);

    void begin();
    void update();

    uint16_t getRawValue() const;          // Valor de lectura directa
    uint16_t getFilteredValue() const;     // Valor suavizado
    float getNormalizedValue() const;      // Valor suavizado normalizado (0.0–1.0)

private:
    uint8_t _pin;
    uint8_t _windowSize;
    FilterType _filterType;
    Smoothed<uint16_t> _smoothed;
    uint16_t _filteredVal;
    uint16_t _rawVal;
};

#endif // FSR_READER_H
