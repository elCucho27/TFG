#include <Wire.h>
#include "Apolo3Processor.h"

// Dirección I2C fija del sensor Apollo3
#define APOLLO3_ADDRESS 0x0D
#define POLL_INTERVAL_MS 30

// Estructura enviada por el módulo Apollo3
struct Apolo3I2CData {
  uint16_t softpots[3];
  uint16_t fsrs[3];
  uint16_t dotfsrs[3];
};

Apolo3Processor apolo3;
Apolo3I2CData sensorData;
uint32_t lastPollTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // SDA = 8, SCL = 9 en ESP32-S3

  // Configurar cada cuerda en modo continuo (sin trastes)
  for (uint8_t i = 0; i < 3; ++i) {
    apolo3.setStringConfig(
      i,           // índice de la cuerda (0, 1, 2)
      i + 1,       // canal MIDI (1, 2, 3)
      60,          // nota base C4
      20,          // 20 zonas de nota a lo largo del softpot
      false,       // modo trasteado = false → modo continuo
      true,        // pitch bend activado
      2            // pitch bend ±2 semitonos
    );
  }

  Serial.println("🎼 CONTROLADOR_CONTINUO_TEST listo.");
}

void loop() {
  uint32_t now = millis();
  if (now - lastPollTime >= POLL_INTERVAL_MS) {
    lastPollTime = now;

    // Solicitar datos binarios del sensor esclavo
    Wire.requestFrom(APOLLO3_ADDRESS, sizeof(Apolo3I2CData));
    if (Wire.available() == sizeof(Apolo3I2CData)) {
      Wire.readBytes((char*)&sensorData, sizeof(Apolo3I2CData));

      // Convertir estructura a vector para update()
      std::vector<uint16_t> raw(9);
      for (int i = 0; i < 3; ++i) {
        raw[i * 3 + 0] = sensorData.softpots[i];
        raw[i * 3 + 1] = sensorData.fsrs[i];     // FSR500 = activación + velocidad
        raw[i * 3 + 2] = sensorData.dotfsrs[i];  // DotFSR = modulación
      }

      apolo3.update(raw);
      apolo3.printMusicalState();  // 🔍 Muestra nota, bend, mod, vel, estado
    } else {
      Serial.println("⚠️ Error: datos incompletos desde Apollo3.");
    }
  }
}
