#include "I2CHandler.h"

I2CHandler* I2CHandler::instance = nullptr;

// Constructor
I2CHandler::I2CHandler(uint8_t i2cAddress, SensorManager& sensorManager)
: _address(i2cAddress), _sensorManager(sensorManager), _dataLength(0) {
    instance = this; // Guardamos la instancia para uso dentro de onRequest
}

// Inicialización
void I2CHandler::begin() {
    Wire.begin(_address); // Configuramos la ESP32 como esclavo I2C
    Wire.onRequest(onRequestService); // Asociamos el evento de solicitud
}

// Actualiza los datos disponibles para el maestro
void I2CHandler::update() {
    fillBuffer(); // Volcamos las últimas lecturas en el buffer
}

// Rellena el buffer I2C con datos actuales de los sensores
void I2CHandler::fillBuffer() {
    // Pedimos los datos empaquetados al SensorManager
    uint8_t* sensorData = _sensorManager.getSensorData(&_dataLength);

    if (sensorData != nullptr && _dataLength <= I2C_BUFFER_SIZE) {
        memcpy(_buffer, sensorData, _dataLength);
    } else {
        _dataLength = 0; // Si hay error o excedemos tamaño, enviamos buffer vacío
    }
}

// Manejador llamado automáticamente cuando el maestro hace un requestFrom()
void I2CHandler::onRequestService() {
    if (instance != nullptr) {
        Wire.write(instance->_buffer, instance->_dataLength);
    }
}
