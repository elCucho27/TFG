#include <Wire.h>

#define I2C_ADDRESS 0x0D  // Rango reservado para Apollo3

struct Apollo3I2CData {
  uint16_t softpots[3];
  uint16_t fsrs[3];
  uint16_t dotfsrs[3];
};

Apollo3I2CData data;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(onRequestHandler);

  Serial.begin(115200);
  Serial.println("🧪 Esclavo Apollo3 iniciado (0x0D)");
}

void loop() {
  for (int i = 0; i < 3; i++) {
    data.softpots[i] = random(0, 4095);
    data.fsrs[i]     = random(0, 4095);
    data.dotfsrs[i]  = random(0, 4095);
  }

  delay(10);  // Simula tasa de muestreo
}

void onRequestHandler() {
  Wire.write((uint8_t*)&data, sizeof(data));
}
