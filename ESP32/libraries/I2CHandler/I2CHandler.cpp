#include "I2CHandler.h"

// Variable global para acceder desde funciones externas
static I2CHandler* _globalI2CHandler = nullptr;

I2CHandler::I2CHandler(SensorManager* manager)
    : _sensorManager(manager)
{}

void I2CHandler::begin(uint8_t address, int sdaPin, int sclPin) {
    if (sdaPin >= 0 && sclPin >= 0) {
        Wire.begin(sdaPin, sclPin, address);
    } else {
        Wire.begin(address);
    }

    _slaveAddress = address;
    _globalI2CHandler = this;
    Wire.onRequest(onI2CRequest);
    Wire.onReceive(onI2CReceive);
}

void I2CHandler::update() {
    prepareBuffer();
}

void I2CHandler::prepareBuffer() {
    // Cabecera
    _buffer[0] = 0xAA;
    _buffer[1] = 0x55;

    uint8_t pos = 3;  // Dejar espacio para longitud

    // Número de SoftPots
    uint8_t numSoftPots = _sensorManager->getSoftPotCount();
    _buffer[pos++] = numSoftPots;
    for (uint8_t i = 0; i < numSoftPots; i++) {
        uint16_t value = _sensorManager->getSoftPotFiltered(i);
        _buffer[pos++] = (value >> 8) & 0xFF;
        _buffer[pos++] = value & 0xFF;
    }

    // Número de FSRs
    uint8_t numFSRs = _sensorManager->getFSRCount();
    _buffer[pos++] = numFSRs;
    for (uint8_t i = 0; i < numFSRs; i++) {
        uint16_t value = _sensorManager->getFSRFiltered(i);
        _buffer[pos++] = (value >> 8) & 0xFF;
        _buffer[pos++] = value & 0xFF;
    }

    // Longitud de payload
    _buffer[2] = pos - 3;

    // Checksum
    uint8_t checksum = 0;
    for (uint8_t i = 3; i < pos; i++) {
        checksum += _buffer[i];
    }
    _buffer[pos++] = checksum;
}

void I2CHandler::onRequest() {
    Wire.write(_buffer, sizeof(_buffer));
}

void I2CHandler::onReceive(int numBytes) {
    // Actualmente no procesamos datos entrantes
}

// --- Implementaciones externas requeridas por Wire ---

void onI2CRequest() {
    if (_globalI2CHandler) _globalI2CHandler->onRequest();
}

void onI2CReceive(int numBytes) {
    if (_globalI2CHandler) _globalI2CHandler->onReceive(numBytes);
}
