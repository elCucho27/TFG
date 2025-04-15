#include "SensorManager.h"   // Incluye la clase que gestiona todos los sensores
#include "I2CHandler.h"      // Incluye la clase que gestiona la comunicación I2C

#define SDA_PIN 21
#define SCL_PIN 22

// Creamos una instancia del gestor de sensores
SensorManager sensorManager;

// Creamos una instancia del manejador I2C con dirección de esclavo 0x10 (puede cambiarse si hay múltiples esclavos)
I2CHandler i2cHandler(SDA_PIN, SCL_PIN, 0x10);

void setup() {
  Serial.begin(115200); // Inicia la comunicación por puerto serie para debug

  // --- Inicialización de sensores ---
  // Formato: pin físico, dirección OSC, ID I2C, valor mínimo, valor máximo, coeficiente alpha (filtro exponencial)
  sensorManager.addSensor(34, "/sensor/fsr1", 1, 0, 4095, 0.1);
  sensorManager.addSensor(35, "/sensor/dot1", 2, 0, 4095, 0.1);
  sensorManager.addSensor(32, "/sensor/pot1", 3, 0, 4095, 0.1);

  // Inicia la comunicación I2C como esclavo
  i2cHandler.begin();

  Serial.println("Módulo sensor I2C iniciado como esclavo en dirección 0x10");
}

void loop() {
  sensorManager.updateSensors();
  i2cHandler.update(&sensorManager);

  // Mostrar valores por consola para depuración
  for (int i = 0; i < sensorManager.getSensorCount(); i++) {
    Sensor* s = sensorManager.getSensor(i);
    Serial.print("ID ");
    Serial.print(s->getId());
    Serial.print(" -> ");
    Serial.println(s->getValue(), 3);
  }

  delay(100);
}

