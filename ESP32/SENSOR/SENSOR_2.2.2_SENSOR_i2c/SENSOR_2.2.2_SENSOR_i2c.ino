#include <Wire.h>
#include "SensorManager.h"

#define SLAVE_ADDR 0x08

SensorManager sensors;  // Crea una instancia del manager

void onI2CRequest() {
    auto packet = sensors.packetize();           // Pedimos los datos empaquetados
    Wire.write(packet.data(), packet.size());    // Los enviamos al maestro
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // 1) Registrar 3 sensores tipo SoftPot en sus pines respectivos
    sensors.addSensor(SensorType::SoftPot, 34);
    sensors.addSensor(SensorType::SoftPot, 35);
    sensors.addSensor(SensorType::SoftPot, 32);

    // 2) Inicializar todos los sensores registrados
    sensors.begin();

    // 3) Configurar el I2C como esclavo
    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(onI2CRequest);
}

void loop() {
    static uint32_t last = 0;
    const uint32_t period = 10; // ms
    uint32_t now = millis();
    if (now - last >= period) {
        last = now;
        sensors.update();    // Actualizamos lecturas de todos los sensores
    }
}
