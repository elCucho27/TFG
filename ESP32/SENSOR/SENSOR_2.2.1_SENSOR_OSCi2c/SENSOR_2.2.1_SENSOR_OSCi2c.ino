#include "SensorManager.h"
#include "I2CHandler.h"

// Pines I2C específicos
#define SDA_SLAVE 8
#define SCL_SLAVE 9

// Crear objetos globales
SensorManager sensorManager;
I2CHandler i2cHandler(&sensorManager); // Le pasamos el SensorManager

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando SensorManager e I2CHandler...");

    sensorManager.addSoftPot(32);
    sensorManager.addSoftPot(35);
    sensorManager.addSoftPot(34);

    sensorManager.begin();
    i2cHandler.begin(0x08, SDA_SLAVE, SCL_SLAVE); // dirección + pines
}

void loop() {
    // Solo actualizamos las lecturas de sensores
    sensorManager.update();

    delay(50);  // Nada más aquí
}
