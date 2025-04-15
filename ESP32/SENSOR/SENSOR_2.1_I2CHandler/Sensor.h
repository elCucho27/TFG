#ifndef SENSOR_H //proteccion contra inclusiones multiples
#define SENSOR_H

#include <Arduino.h>   // Necesario para analogRead, pinMode, etc.
#include <stdint.h>    // Necesario para uint8_t

class Sensor {
  public:
    Sensor(int pin, const char* OSCaddress, uint8_t I2Cid, int minValue, int maxValue, float alpha);

    void update();                 // Actualiza el valor leído desde el pin
    bool hasChanged();            // Devuelve true si el valor cambió desde la última lectura
    float getValue();             // Devuelve el valor actual filtrado y normalizado (0.0 - 1.0)
    const char* getAddress();     // Devuelve la ruta OSC asociada al sensor
    uint8_t getId();	          // Devuelve el id I2C asociado al sensor

  private:
    int _pin;                     // Pin analógico al que está conectado el sensor
    const char* _OSCaddress;      // Ruta OSC (ej. "/LAVANA/fsr1")
    int _minValue;                // Valor mínimo esperado de lectura analógica
    int _maxValue;                // Valor máximo esperado
    float _alpha;                 // Coeficiente del filtro exponencial
    float _filteredValue;        // Valor filtrado actual
    float _lastSentValue;        // Último valor enviado (para detectar cambios)
    float normalize(int raw);     // Convierte valor crudo a rango 0.0 - 1.0
    uint8_t _I2Cid;			  // identificador I2C
};

#endif
