#ifndef SWITCH_READER_H
#define SWITCH_READER_H

#include "ISensor.h"

// Clase para leer un pulsador digital (switch) con lógica activa en LOW
class SwitchReader : public ISensor {
public:
    // Constructor: recibe el pin digital
    SwitchReader(uint8_t digitalPin);

    // Métodos obligatorios de la interfaz ISensor
    void begin() override;
    void update() override;
    uint16_t getValue() const override;
    bool hasChanged() const override;  // Ahora es válido porque ISensor lo define

private:
    uint8_t _pin;         // Pin donde está conectado el switch
    uint8_t _state;       // Estado actual (HIGH o LOW)
    uint8_t _lastState;   // Estado anterior para detectar cambios
    bool changed;         // Flag que indica si hubo cambio desde el último update
};

#endif
