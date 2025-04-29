#ifndef SOFTPOT_READER_H
#define SOFTPOT_READER_H

#include <Arduino.h>
#include <Smoothed.h>

class SoftPotReader {
  public:
    SoftPotReader(uint8_t pin);
    void begin();
    void update();
    uint16_t getFilteredValue() const; 
    uint16_t getRawValue() const;

  private:
    uint8_t _pin;           // Pin analógico donde está conectado el SoftPot
    uint16_t _filteredVal;  // Valor filtrado (mínimo de 3 lecturas)
    uint16_t _rawVal;       // Última lectura bruta
    Smoothed<uint16_t> _smoothed;    //suavizado de lecturas ADC
     
};

#endif
