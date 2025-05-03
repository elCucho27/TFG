git add .
// MaestroI2C_SensorReader.ino
#include <Wire.h>

#define SLAVE_ADDR 0x08    // Dirección del esclavo I²C (debe coincidir con el que configuraste)
#define REQUEST_INTERVAL 10 // Tiempo entre peticiones en milisegundos
#define NUM_SENSORS 3       // Número de sensores que esperamos
#define BYTES_PER_SENSOR 2  // Cada sensor envía un uint16_t (2 bytes)

uint32_t lastRequestTime = 0; // Para controlar el tiempo entre peticiones

void setup() {
    Serial.begin(115200);
    delay(100);

    Wire.begin(); // Iniciar como Maestro
    Serial.println("[MASTER] Iniciado maestro I2C.");
}

void loop() {
    uint32_t now = millis();
    if (now - lastRequestTime >= REQUEST_INTERVAL) {
        lastRequestTime = now;

        // --- 1) Solicitar datos al esclavo ---
        uint8_t numBytesToRequest = NUM_SENSORS * BYTES_PER_SENSOR;
        Wire.requestFrom(SLAVE_ADDR, numBytesToRequest);

        // --- 2) Leer datos recibidos ---
        uint16_t sensorValues[NUM_SENSORS] = {0}; // Array para guardar los valores de sensores
        uint8_t i = 0; // Índice para recorrer el array

        while (Wire.available() >= 2 && i < NUM_SENSORS) {
            uint8_t highByte = Wire.read(); // Leer byte alto
            uint8_t lowByte = Wire.read();  // Leer byte bajo

            sensorValues[i] = (highByte << 8) | lowByte; // Reconstruir el uint16_t
            i++;
        }

        // --- 3) Depuración: mostrar valores por Serial ---
        Serial.print("[MASTER] Lecturas recibidas: ");
        for (uint8_t j = 0; j < NUM_SENSORS; j++) {
            Serial.print("Sensor ");
            Serial.print(j);
            Serial.print(": ");
            Serial.print(sensorValues[j]);
            Serial.print("  ");
        }
        Serial.println();
    }

    // Aquí podrías añadir otras tareas si quisieras (leer botones, LEDs, etc.)
}
