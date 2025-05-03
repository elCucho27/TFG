// SoftPotReader.h
#ifndef SOFTPOT_READER_H
#define SOFTPOT_READER_H

#include <Arduino.h>
#include <Smoothed.h>  // Librería para hacer promedio de lecturas
#include "ISensor.h"   // Nuestra interfaz base

/**
 *  Clase para leer sensores de tipo SoftPot.
 */
class SoftPotReader : public ISensor {
public:
    SoftPotReader(uint8_t pin);    // Constructor: recibe el pin analógico donde está el sensor.

    void begin() override;         // Inicializa el sensor.
    void update() override;        // Lee y filtra una nueva lectura.
    uint16_t getValue() const override; // Devuelve el valor filtrado.

    uint16_t getRawValue() const;  // Devuelve la última lectura cruda (sin filtro).

private:
    uint8_t _pin;                  // Pin analógico conectado.
    uint16_t _filteredVal;          // Valor filtrado.
    uint16_t _rawVal;               // Valor bruto leído del ADC.
    Smoothed<uint16_t> _smoothed;   // Instancia de filtro de promedio.
};

#endif // SOFTPOT_READER_H
