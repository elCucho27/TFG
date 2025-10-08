#include <Wire.h>
#include "Apollo3I2CData.h"

// Dirección fija del esclavo
#define I2C_ADDRESS 0x0D

Apollo3I2CData sensorData;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(onRequestHandler);

  #ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Apollo3 I2C esclavo iniciado en dirección 0x0D");
  #endif
}

void loop() {
  for (int i = 0; i < 3; i++) {
    sensorData.softpots[i] = random(0, 4095);
    sensorData.fsrs[i]     = random(0, 4095);
    sensorData.dotfsrs[i]  = random(0, 4095);
  }

  delay(10);
}

void onRequestHandler() {
  Wire.write((uint8_t*)&sensorData, sizeof(Apollo3I2CData));
}
