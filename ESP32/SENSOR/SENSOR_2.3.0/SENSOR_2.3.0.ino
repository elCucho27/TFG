#include <Wire.h>
#include <SensorManager.h>
#include "Config.h"

// Instancia global del gestor de sensores
SensorManager sensors;

void onI2CRequest() {
    auto packet = sensors.packetize();
    Wire.write(packet.data(), packet.size());
}

void setup() {
    setupHardware();
    setupSoftware();
}

void setupHardware() {
    Serial.begin(115200);
    delay(10);   // Pequeño retardo tras Serial.begin

    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(onI2CRequest);

    Serial.println("[SLAVE] I2C inicializado como esclavo.");
}

void setupSoftware() {
    sensors.addSensor(SensorType::SoftPot, 34);
    sensors.addSensor(SensorType::SoftPot, 35);
    sensors.addSensor(SensorType::SoftPot, 32);

    sensors.addSensor(SensorType::FSR, 33);
    sensors.addSensor(SensorType::FSR, 25);
    sensors.addSensor(SensorType::FSR, 26);
/*
    sensors.addSensor(SensorType::DotFSR, 5);
    sensors.addSensor(SensorType::DotFSR, 17);
    sensors.addSensor(SensorType::DotFSR, 16);
*/
    sensors.begin();
    Serial.println("[SLAVE] Sensores inicializados.");
}

void loop() {
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();

    if (now - lastUpdate >= REQUEST_INTERVAL_MS) {
        lastUpdate = now;
        sensors.update();
    }
}
