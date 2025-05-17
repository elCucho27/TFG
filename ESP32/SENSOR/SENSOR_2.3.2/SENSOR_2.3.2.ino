#include <Wire.h>
#include <SensorManager.h>
#include "Config.h"

#define DEBUG_MODE 1

SensorManager sensors;

#if DEBUG_MODE
#include <array>
uint32_t lastDebugPrint = 0;
const uint16_t CHANGE_THRESHOLD = 10;
std::array<uint16_t, 9> lastValues = {0};
#endif

std::vector<uint8_t> sensorPacket;
bool packetReady = false;

void onI2CRequest() {
    Serial.println("[SLAVE] Petición recibida por I2C.");
    if (packetReady) {
        Wire.write(sensorPacket.data(), sensorPacket.size());
    } else {
        uint8_t empty = 0;
        Wire.write(&empty, 1);
    }
}

void setup() {
    Serial.begin(115200);
    delay(10);

    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(onI2CRequest);

    sensors.addSensor(SensorType::SoftPot, 32);
    sensors.addSensor(SensorType::SoftPot, 35);
    sensors.addSensor(SensorType::SoftPot, 34);

    sensors.addSensor(SensorType::FSR, 33);
    sensors.addSensor(SensorType::FSR, 25);
    sensors.addSensor(SensorType::FSR, 26);
    
    sensors.addSensor(SensorType::DotFSR, 12);
    sensors.addSensor(SensorType::DotFSR, 14);
    sensors.addSensor(SensorType::DotFSR, 27);

    sensors.begin();

    Serial.println("[SLAVE] Sensores inicializados.");
    Serial.print("[SLAVE] Dirección I2C: 0x");
    Serial.println(SLAVE_ADDR, HEX);
}

void loop() {
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();

    if (now - lastUpdate >= REQUEST_INTERVAL_MS) {
        lastUpdate = now;

        sensors.update();

        // Generamos paquete SIEMPRE para que el maestro siempre reciba algo válido
        sensorPacket.clear();
        sensorPacket.push_back(0xAA); // Cabecera para sincronización
        const auto& values = sensors.getValues();

        for (auto v : values) {
            sensorPacket.push_back(highByte(v));
            sensorPacket.push_back(lowByte(v));
        }

        packetReady = true;

#if DEBUG_MODE
        if (now - lastDebugPrint > 300) {
            lastDebugPrint = now;
            if (values.size() == 9) {
                bool hasChanged = false;
                for (size_t i = 0; i < 9; ++i) {
                    if (abs((int)values[i] - (int)lastValues[i]) > CHANGE_THRESHOLD) {
                        hasChanged = true;
                        break;
                    }
                }

                if (hasChanged) {
                    for (size_t i = 0; i < 9; ++i) lastValues[i] = values[i];

                    Serial.println(F("====== SENSOR DEBUG ======"));
                    Serial.println(F("| # | SoftPot |   FSR   | DotFSR |"));
                    Serial.println(F("|---|---------|---------|--------|"));
                    for (int i = 0; i < 3; ++i) {
                        uint16_t softpot = values[i];
                        uint16_t fsr     = values[i + 3];
                        uint16_t dotfsr  = values[i + 6];
                        Serial.printf("| %d |  %5d  |  %5d  |  %5d |\n", i + 1, softpot, fsr, dotfsr);
                    }
                    Serial.println(F("==========================\n"));
                }
            }
        }
#endif
    }
}
