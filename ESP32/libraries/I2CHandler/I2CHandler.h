#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include "SensorManager.h"

// Tamaño máximo del buffer I2C
#define I2C_BUFFER_SIZE 64

class I2CHandler {
public:
    I2CHandler(uint8_t i2cAddress, SensorManager& sensorManager);
    void begin();   // Inicializa la comunicación I2C como esclavo
    void update();  // Actualiza internamente el buffer con los datos actuales

private:
    static void onRequestService(); // Manejador de evento de petición de datos
    static I2CHandler* instance;    // Instancia estática necesaria para el onRequest

    uint8_t _address;               // Dirección I2C del esclavo
    SensorManager& _sensorManager;  // Referencia al SensorManager
    uint8_t _buffer[I2C_BUFFER_SIZE]; // Buffer de datos a enviar
    size_t _dataLength;              // Tamaño actual de los datos en el buffer

    void fillBuffer();              // Rellena el buffer con los datos actuales
};

#endif // I2C_HANDLER_H
