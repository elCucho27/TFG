// SensorManager.h
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <vector>      
#include "ISensor.h"     

/// Tipos de sensores que soportaremos
enum class SensorType {
    SoftPot,
    FSR,
    // Más tipos futuros aquí...
};

class SensorManager {
public:
    SensorManager();      // Constructor
    ~SensorManager();     // Destructor que limpia la memoria.

    void addSensor(SensorType type, uint8_t pin);  // Añade un nuevo sensor según tipo y pin.
    void begin();                                  // Inicializa todos los sensores registrados.
    void update();                                 // Actualiza todos los sensores.
    const std::vector<uint16_t>& getValues() const; // Devuelve los valores actuales.
    std::vector<uint8_t> packetize() const;         // Prepara los datos en bytes para I2C.

private:
    std::vector<ISensor*> _sensors;      // Lista de sensores
    std::vector<uint16_t> _values;        // Lista de valores leídos
};

#endif // SENSOR_MANAGER_H
