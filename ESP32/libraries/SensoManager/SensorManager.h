#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>       // Librería base para tipos y funciones estándar de Arduino
#include <vector>          // Permite usar vectores dinámicos (listas) de datos
#include "ISensor.h"       // Interfaz base común para cualquier sensor

// Enumeración para identificar tipos de sensores posibles
enum class SensorType {
    SoftPot,     // Sensor resistivo lineal (posición)
    FSR,         // Force Sensing Resistor (presión)
    DotFSR       // Variante puntual del FSR
};

// Clase que gestiona todos los sensores: su creación, lectura y empaquetado
class SensorManager {
public:
    SensorManager();    // Constructor: se llama automáticamente al crear una instancia
    ~SensorManager();   // Destructor: libera memoria reservada para sensores

    void addSensor(SensorType type, uint8_t pin);   // Añade un sensor de cierto tipo en un pin dado
    void begin();                                   // Inicializa todos los sensores registrados
    void update();                                  // Actualiza las lecturas de todos los sensores

    const std::vector<uint16_t>& getValues() const; // Devuelve los valores actuales filtrados
    std::vector<uint8_t> packetize() const;         // Devuelve los valores empaquetados como bytes

    bool hasSignificantChange(uint16_t threshold);  // Compara valores actuales con los anteriores
    void markValuesAsSent();                        // Guarda los valores actuales como "últimos enviados"

private:
    std::vector<ISensor*> _sensors;         // Lista de punteros a sensores instanciados
    std::vector<uint16_t> _values;          // Valores actuales filtrados
    std::vector<uint16_t> _lastSentValues;  // Últimos valores que se enviaron al controlador
};

#endif // SENSOR_MANAGER_H
