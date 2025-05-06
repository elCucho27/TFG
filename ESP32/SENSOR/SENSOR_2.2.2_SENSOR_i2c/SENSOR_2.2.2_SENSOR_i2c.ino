#include <Wire.h>
#include "SensorManager.h"

// Dirección I²C de este esclavo
#define SLAVE_ADDR 0x08

// Instancia global del gestor de sensores
SensorManager sensors;

// Esta función se llama automáticamente cuando el maestro pide datos (Wire.requestFrom)
void onI2CRequest() {
    auto packet = sensors.packetize();           // Preparamos todos los valores de sensores en un array de bytes
    Wire.write(packet.data(), packet.size());    // Enviamos ese array de bytes al maestro
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // 1) Registrar 3 sensores tipo SoftPot
    sensors.addSensor(SensorType::SoftPot, 34);   // SoftPot 1 en pin 34
    sensors.addSensor(SensorType::SoftPot, 35);   // SoftPot 2 en pin 35
    sensors.addSensor(SensorType::SoftPot, 32);   // SoftPot 3 en pin 32

    // 2) Registrar 3 sensores tipo FSR
    sensors.addSensor(SensorType::FSR, 33);       // FSR 1 en pin 33
    sensors.addSensor(SensorType::FSR, 25);       // FSR 2 en pin 25
    sensors.addSensor(SensorType::FSR, 26);       // FSR 3 en pin 26

    // 3) Inicializar todos los sensores registrados
    sensors.begin();

    Serial.println("[SLAVE] Sensores inicializados correctamente.");

    // 4) Configurar el I2C como esclavo
    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(onI2CRequest);

    Serial.println("[SLAVE] I2C inicializado como esclavo en dirección 0x08.");
}

void loop() {
    static uint32_t last = 0;
    const uint32_t period = 10; // ms
    uint32_t now = millis();
    if (now - last >= period) {
        last = now;
        sensors.update();    // Actualizamos todas las lecturas de los sensores

        // Opcional: imprimir lecturas para depuración
        /*
        auto values = sensors.getValues();
        Serial.print("[SLAVE] Lecturas: ");
        for (auto v : values) {
            Serial.print(v);
            Serial.print(" ");
        }
        Serial.println();
        */
    }
}
