#include "I2CHandler.h"

// Inicialización de variables estáticas
SensorManager* I2CHandler::_manager = nullptr;        // Puntero a gestor de sensores
uint8_t I2CHandler::_dataBuffer[64] = {0};            // Buffer de datos (hasta 21 sensores de 3 bytes cada uno)
size_t I2CHandler::_dataLength = 0;                   // Longitud actual del buffer
uint8_t I2CHandler::_address = 0;                     // Dirección I2C
int I2CHandler::_sdaPin = -1;				      // Pin SDA
int I2CHandler::_sclPin = -1;				      // Pin SCL

// Constructor: almacena la dirección I2C con la que se identificará este esclavo
I2CHandler::I2CHandler(uint8_t i2cAddress, int sda, int scl) {
  _address = i2cAddress;
  _sdaPin = sda;
  _sclPin = scl;
}

// Inicializa la comunicación I2C como esclavo
void I2CHandler::begin() {
  if (_sdaPin >= 0 && _sclPin >= 0) {
    Wire.begin(_sdaPin, _sclPin, _address); // SDA, SCL, dirección esclavo
  } else {
    Wire.begin(_address); // Configura la ESP32 como esclavo con la dirección especificada
  }

  Wire.onRequest(onRequest);
}         
      


// Actualiza el buffer con los valores de los sensores actuales
void I2CHandler::update(SensorManager* sm) {
  _manager = sm;                 // Guardamos el puntero al SensorManager
  _dataLength = 0;               // Reiniciamos la longitud del buffer

  // Recorremos todos los sensores registrados
  for (int i = 0; i < _manager->getSensorCount(); i++) {
    Sensor* s = _manager->getSensor(i);          // Obtenemos el sensor en la posición i
    uint8_t id = s->getId();                     // ID único del sensor
    uint16_t value = s->getValue() * 4095;       // Convertimos el valor normalizado a rango 0-4095 (12 bits)

    // Empaquetamos los datos: [ID][VAL_MSB][VAL_LSB]
    _dataBuffer[_dataLength++] = id;             // Byte 1: ID del sensor
    _dataBuffer[_dataLength++] = (value >> 8);   // Byte 2: parte alta del valor
    _dataBuffer[_dataLength++] = value & 0xFF;   // Byte 3: parte baja del valor
  }
}

// Función que se llama automáticamente cuando el maestro solicita datos
void I2CHandler::onRequest() {
  // Enviamos el contenido actual del buffer al maestro
  Wire.write(_dataBuffer, _dataLength);
}
