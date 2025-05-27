#include <Wire.h>
#include <SensorManager.h>

// Sensor Configuration
#define NUM_SENSORS 9
#define NUM_PAIRS 3

// I2C Configuration
#define SLAVE_ADDR 0x08
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define REQUEST_INTERVAL_MS 30

#define DEBUG_MODE 1

SensorManager sensors;

#if DEBUG_MODE
#include <array>
uint32_t lastDebugPrint = 0;
const uint16_t CHANGE_THRESHOLD = 10;
std::array<uint16_t, 9> lastValues = {0};
#endif

// Paquete binario que se actualizará periódicamente
std::vector<uint8_t> sensorPacket;
bool packetReady = false;

// Función llamada automáticamente por Wire cuando el maestro solicita datos
void onI2CRequest() {
    if (packetReady) {
        Wire.write(sensorPacket.data(), sensorPacket.size());
    } else {
        uint8_t empty = 0;
        Wire.write(&empty, 1);
    }
}

void setup() {
    setupHardware();
    setupSoftware();
}

void setupHardware() {
    Serial.begin(115200);
    delay(10);

    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(onI2CRequest);

    Serial.println("[SLAVE] I2C inicializado como esclavo.");
    Serial.print("[SLAVE] Dirección I2C: 0x");
    Serial.println(SLAVE_ADDR, HEX);
}

void setupSoftware() {
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
}

void loop() {
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();

    if (now - lastUpdate >= REQUEST_INTERVAL_MS) {
        lastUpdate = now;

        sensors.update();

        // Empaquetar los datos con cabecera
        sensorPacket.clear();
        sensorPacket.push_back(0xAA);  // Cabecera

        const auto& values = sensors.getValues();
        for (uint16_t val : values) {
            sensorPacket.push_back(val & 0xFF);
            sensorPacket.push_back((val >> 8) & 0xFF);
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
                    for (size_t i = 0; i < 9; ++i) {
                        lastValues[i] = values[i];
                    }

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
