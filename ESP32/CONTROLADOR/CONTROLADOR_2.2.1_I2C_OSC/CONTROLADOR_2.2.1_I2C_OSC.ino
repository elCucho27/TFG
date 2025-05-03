// CONTROLADOR_2.3_I2C_TEST.ino

#include <Wire.h>

#define I2C_ADDRESS 0x08     // Dirección del sensor
#define SDA_PIN 21           // Configura tu pin correcto de SDA
#define SCL_PIN 22           // Configura tu pin correcto de SCL

#define SOFTPOT_COUNT 3      // Número de SoftPots
#define FSR_COUNT 0          // Número de FSRs (de momento)
#define SENSOR_DATA_SIZE (SOFTPOT_COUNT + FSR_COUNT) * 2 // cada lectura es uint16_t (2 bytes)

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando módulo CONTROLADOR...");

  Wire.begin(SDA_PIN, SCL_PIN);  // Iniciar como maestro
  Serial.println("I2C iniciado correctamente.");
}

void loop() {
  // Solicitar los datos al sensor
  Wire.requestFrom(I2C_ADDRESS, SENSOR_DATA_SIZE);

  if (Wire.available() == SENSOR_DATA_SIZE) {
    for (int i = 0; i < SENSOR_DATA_SIZE; i += 2) {
      uint8_t lowByte = Wire.read();
      uint8_t highByte = Wire.read();
      uint16_t sensorValue = (highByte << 8) | lowByte;

      Serial.print("Sensor ");
      Serial.print(i / 2);
      Serial.print(": ");
      Serial.println(sensorValue);
    }
  } else {
    Serial.println("Datos incompletos recibidos.");
  }

  delay(100); // Leer cada 100 ms
}
