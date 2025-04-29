#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include <Wire.h>
#include "SensorManager.h"

class I2CHandler {
public:
    I2CHandler(SensorManager* manager);

    // Ahora el begin() recibe la dirección y opcionalmente los pines SDA/SCL
    void begin(uint8_t address, int sdaPin = -1, int sclPin = -1);
    void update();  // Actualiza el buffer antes de que el master lo pida

    // Estas funciones son llamadas por Wire
    void onRequest();
    void onReceive(int numBytes);

private:
    SensorManager* _sensorManager;
    uint8_t _slaveAddress;
    uint8_t _buffer[64];

    void prepareBuffer();  // Rellena el buffer antes de enviarlo

};

// --- Funciones C "libres" para compatibilidad con Wire ---
void onI2CRequest();
void onI2CReceive(int numBytes);

#endif // I2C_HANDLER_H
