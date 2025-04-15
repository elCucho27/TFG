#ifndef I2CHANDLER_H
#define I2CHANDLER_H

#include <Wire.h>              // Librería para comunicación I2C
#include "SensorManager.h"    // Se incluye para poder acceder a los sensores

// Clase encargada de gestionar la comunicación I2C desde el lado del esclavo
class I2CHandler {
  public:
    // Constructor: recibe la dirección I2C con la que el esclavo responderá al maestro
    I2CHandler(uint8_t i2cAddress, int sda = -1, int scl = -1);

    // Inicializa el bus I2C como esclavo y define la función de respuesta
    void begin();

    // Prepara el buffer de datos con los valores actuales de los sensores
    void update(SensorManager* sm);

  private:
    // Función estática que se llama automáticamente cuando el maestro solicita datos
    static void onRequest();

    // Puntero estático al SensorManager para poder acceder a los sensores desde funciones estáticas
    static SensorManager* _manager;

    // Buffer donde se almacenan los datos preparados para enviar por I2C
    static uint8_t _dataBuffer[64];

    // Longitud del buffer de datos actual (cuántos bytes se van a enviar)
    static size_t _dataLength;

    // Dirección I2C que usa este esclavo
    static uint8_t _address;

    // Pin SDA configurado
    static int _sdaPin; 

    // Pin SCL configurado
    static int _sclPin;   
};

#endif
