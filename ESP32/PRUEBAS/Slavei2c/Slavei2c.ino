#include <Wire.h>

#define I2C_SLAVE_ADDR 0x12
#define SENSOR1_PIN 34  // ADC1_CH6
#define SENSOR2_PIN 35  // ADC1_CH7
#define SENSOR3_PIN 32  // ADC1_CH4
#define CHANGE_THRESHOLD 5  // Umbral de cambio significativo

uint16_t lastSensor1 = 0;
uint16_t lastSensor2 = 0;
uint16_t lastSensor3 = 0;

void setup() {
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);
  Serial.println("Esclavo I2C iniciado");
}

void loop() {
  // No es necesario hacer nada en el loop principal
  delay(100);
}

void requestEvent() {
  uint16_t sensor1 = analogRead(SENSOR1_PIN);
  uint16_t sensor2 = analogRead(SENSOR2_PIN);
  uint16_t sensor3 = analogRead(SENSOR3_PIN);

  // Verificar si hay cambios significativos
  bool changed = false;
  if (abs((int)sensor1 - (int)lastSensor1) >= CHANGE_THRESHOLD) {
    lastSensor1 = sensor1;
    changed = true;
  }
  if (abs((int)sensor2 - (int)lastSensor2) >= CHANGE_THRESHOLD) {
    lastSensor2 = sensor2;
    changed = true;
  }
  if (abs((int)sensor3 - (int)lastSensor3) >= CHANGE_THRESHOLD) {
    lastSensor3 = sensor3;
    changed = true;
  }

  uint8_t buffer[7];
  buffer[0] = 0xAA;  // Encabezado

  if (changed) {
    // Empaquetar las lecturas en formato little-endian
    buffer[1] = lastSensor1 & 0xFF;
    buffer[2] = (lastSensor1 >> 8) & 0xFF;
    buffer[3] = lastSensor2 & 0xFF;
    buffer[4] = (lastSensor2 >> 8) & 0xFF;
    buffer[5] = lastSensor3 & 0xFF;
    buffer[6] = (lastSensor3 >> 8) & 0xFF;
  } else {
    // Si no hay cambios, enviar ceros
    for (int i = 1; i < 7; i++) {
      buffer[i] = 0x00;
    }
  }

  Wire.write(buffer, 7);
}
