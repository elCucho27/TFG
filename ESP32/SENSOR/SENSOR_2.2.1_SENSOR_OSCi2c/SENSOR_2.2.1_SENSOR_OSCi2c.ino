// SENSOR_2.3_SENSOR_I2C.ino

#include <Wire.h>
#include <SensorManager.h>
#include <I2CHandler.h>

#define I2C_ADDRESS 0x08 // Dirección I2C del esclavo

SensorManager sensorManager;
I2CHandler i2cHandler(I2C_ADDRESS, sensorManager);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando módulo SENSOR...");

  sensorManager.begin();
  i2cHandler.begin();

  Serial.println("Inicialización completa.");
}

void loop() {
  sensorManager.update();  // Actualizar todas las lecturas
  i2cHandler.update();     // Preparar los datos para enviar
  delay(10);               // Pequeño delay para estabilidad
}
