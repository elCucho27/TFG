// ISensor.h
#ifndef I_SENSOR_H
#define I_SENSOR_H

#include <Arduino.h>  // Incluimos Arduino porque usamos tipos como uint16_t

/**
 * Interfaz genérica para cualquier tipo de sensor.
 * 
 * Todos los sensores deben implementar estos 3 métodos:
 * - begin(): para inicializarse.
 * - update(): para leer un nuevo valor.
 * - getValue(): para devolver su último valor leído.
 */
class ISensor {
public:
    virtual ~ISensor() {}                     // Destructor virtual para limpiar bien la memoria.
    virtual void begin() = 0;                  // Método para inicializar el sensor (pinMode, etc).
    virtual void update() = 0;                 // Método para leer el sensor y guardar su valor.
    virtual uint16_t getValue() const = 0;     // Método para pedir el valor leído.
};

#endif // I_SENSOR_H
