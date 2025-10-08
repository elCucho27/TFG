#include <Wire.h>

#define I2C_ADDRESS 0x1E  // Rango reservado para Orpheus

struct OrpheusI2CData {
  uint16_t values[7];
};

OrpheusI2CData data;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(onRequestHandler);

  Serial.begin(115200);
  Serial.println("🧪 Esclavo Orpheus iniciado (0x1E)");
}

void loop() {
  for (int i = 0; i < 7; i++) {
    data.values[i] = random(0, 4095);  // Simulación
  }

  delay(10);  // Simula tasa de muestreo
}

void onRequestHandler() {
  Wire.write((uint8_t*)&data, sizeof(data));
}
