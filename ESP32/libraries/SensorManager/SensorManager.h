#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

/**
 * @file SensorManager.h
 * @brief Clase encargada de gestionar sensores físicos conectados al sistema.
 *
 * Permite registrar sensores según su tipo, inicializarlos, actualizar sus lecturas,
 * obtener los valores actuales, empaquetarlos para transmisión y comparar cambios significativos.
 * Utiliza punteros a una interfaz genérica `ISensor` que permite manejar diferentes tipos de sensores.
 *
 * Los valores leídos se almacenan como `uint16_t`, permitiendo preservar resolución completa de ADC.
 *
 * @author Roberto García
 * @date Junio 2025
 */

// ======================== LIBRERÍAS ========================
#include <Arduino.h>       ///< Tipos básicos y macros del entorno Arduino
#include <vector>          ///< Estructura de listas dinámicas para sensores y valores
#include "ISensor.h"       ///< Interfaz base común para cualquier sensor compatible

/**
 * @enum SensorType
 * @brief Enumeración de tipos de sensores que pueden ser gestionados.
 */
enum class SensorType {
    SoftPot,     // Sensor resistivo lineal (posición)
    FSR,         // Force Sensing Resistor (presión)
    DotFSR,      // Variante puntual del FSR
    Phader,      // Fader físico de alta precisión
    Switch       // Pulsador digital (estado ON/OFF)
};


/**
 * @class SensorManager
 * @brief Clase que gestiona todos los sensores: su creación, lectura y empaquetado.
 */
class SensorManager {
public:
     /**
     * @brief Constructor por defecto.
     * Inicializa una instancia vacía de gestor de sensores.
     */
    SensorManager();  

    /**
     * @brief Destructor.
     * Libera la memoria de todos los sensores registrados.
     */    
    ~SensorManager();  

    /**
     * @brief Añade un sensor al gestor.
     *
     * Crea dinámicamente un nuevo sensor del tipo especificado en el pin indicado,
     * y lo añade a la lista de sensores gestionados.
     *
     * @param type Tipo de sensor (SoftPot, FSR, etc.).
     * @param pin Pin analógico o digital al que está conectado el sensor.
     */
    void addSensor(SensorType type, uint8_t pin);

    /**
     * @brief Inicializa todos los sensores registrados.
     * Llama al método `begin()` de cada sensor y reserva espacio para lecturas.
     */   
    void begin();    

    /**
     * @brief Actualiza las lecturas de todos los sensores.
     * Llama al método `update()` de cada sensor y guarda sus valores filtrados.
     */                               
    void update();  

    /**
     * @brief Devuelve los valores actuales leídos de todos los sensores.
     *
     * @return Referencia constante a un vector de valores (`uint16_t`) en orden de registro.
     */                              
    const std::vector<uint16_t>& getValues() const; // Devuelve los valores actuales filtrados

    /**
     * @brief Empaqueta los valores actuales en formato binario para transmisión.
     *
     * Devuelve un vector de bytes con una cabecera 0xAA seguida de cada valor codificado en MSB-first.
     *
     * @return Vector de bytes listo para ser transmitido (por I2C, OSC, etc.).
     */
    std::vector<uint8_t> packetize() const;         // Devuelve los valores empaquetados como bytes

    /**
     * @brief Determina si ha habido un cambio significativo desde la última marca.
     *
     * Compara cada valor actual con su anterior y evalúa si la diferencia supera un umbral.
     *
     * @param threshold Umbral mínimo para considerar un cambio como significativo.
     * @return true si algún valor ha cambiado más que el umbral, false si no.
     */
    bool hasSignificantChange(uint16_t threshold);  // Compara valores actuales con los anteriores

    /**
     * @brief Marca los valores actuales como "últimos enviados".
     *
     * Copia los valores actuales al vector interno `_lastSentValues`, para futuras comparaciones.
     */
    void markValuesAsSent();                        // Guarda los valores actuales como "últimos enviados"

private:
    std::vector<ISensor*> _sensors;         // Lista de punteros a sensores instanciados
    std::vector<uint16_t> _values;          // Valores actuales filtrados
    std::vector<uint16_t> _lastSentValues;  // Últimos valores que se enviaron al controlador
};

#endif // SENSOR_MANAGER_H
