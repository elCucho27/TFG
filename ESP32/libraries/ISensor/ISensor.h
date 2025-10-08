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
    virtual ~ISensor() {}                     // Destructor virtual para liberar correctamente

    virtual void begin() = 0;                 // Inicializa el sensor (pinMode, estado inicial, etc.)
    virtual void update() = 0;                // Lee una nueva muestra y actualiza el estado interno
    virtual uint16_t getValue() const = 0;    // Devuelve el valor leído (filtrado o actual)

    // ✅ NUEVO MÉTODO VIRTUAL
    // Método opcional para saber si el valor del sensor ha cambiado desde la última lectura
    // Devuelve false por defecto, puede ser sobrescrito por clases que lo necesiten
    virtual bool hasChanged() const { return false; }
};

#endif // I_SENSOR_H
