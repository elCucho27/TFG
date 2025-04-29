#include <Wire.h>

#define I2C_SLAVE_ADDR 0x12

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Maestro I2C iniciado");
}

void loop() {
  Wire.requestFrom(I2C_SLAVE_ADDR, 7);
  if (Wire.available() == 7) {
    uint8_t header = Wire.read();
    if (header == 0xAA) {
      uint16_t sensor1 = Wire.read() | (Wire.read() << 8);
      uint16_t sensor2 = Wire.read() | (Wire.read() << 8);
      uint16_t sensor3 = Wire.read() | (Wire.read() << 8);

      // Verificar si las lecturas no son cero
      if (sensor1 != 0 || sensor2 != 0 || sensor3 != 0) {
        Serial.print("Sensor 1: ");
        Serial.println(sensor1);
        Serial.print("Sensor 2: ");
        Serial.println(sensor2);
        Serial.print("Sensor 3: ");
        Serial.println(sensor3);
      } else {
        //Serial.println("Sin cambios significativos en las lecturas.");
      }
    } else {
      Serial.println("Encabezado inválido recibido.");
    }
  } else {
    Serial.println("Datos incompletos recibidos.");
  }
  delay(30);  // Esperar antes de la siguiente solicitud
}
