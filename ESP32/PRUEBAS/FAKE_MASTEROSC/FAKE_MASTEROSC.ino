#include <Wire.h>

// Dirección I2C del módulo OSC
#define SLAVE_ADDR 0x08

// Intervalo de envío (en milisegundos)
#define SEND_INTERVAL 3000  // 10 mensajes por segundo

// Buffer de valores de sensores: 3 cuerdas × (SoftPot, FSR, DotFSR)
uint16_t sensorValues[9] = {0};

// Control del tiempo de envío
uint32_t lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Configura el ESP32 como maestro I2C
  Serial.println("✅ Maestro I2C iniciado.");
}

void loop() {
  uint32_t now = millis();

  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    // Generar valores falsos pero útiles para pruebas
    simulateSensorData(now);

    // Enviar datos por I2C al módulo OSC
    sendSensorData();

    // Mostrar por Serial para debug
    printSensorData();
  }
}

// === Función que simula datos ===
void simulateSensorData(uint32_t t) {
  for (int i = 0; i < 3; i++) {
    // SoftPot: oscilación suave tipo seno
    sensorValues[i * 3 + 0] = 2048 + 2047 * sin(0.002 * t + i);

    // FSR: onda cuadrada con diferente fase
    sensorValues[i * 3 + 1] = ((t / (400 + i * 100)) % 2 == 0) ? 3000 : 300;

    // DotFSR: simula pulsación cada ~2 segundos
    sensorValues[i * 3 + 2] = (t % 2000 < 120) ? 600 : 0;
  }
}

// === Enviar los 9 valores como 18 bytes (2 bytes por sensor) ===
void sendSensorData() {
  Wire.beginTransmission(SLAVE_ADDR);

  for (int i = 0; i < 9; i++) {
    Wire.write((sensorValues[i] >> 8) & 0xFF); // byte alto
    Wire.write(sensorValues[i] & 0xFF);        // byte bajo
  }

  Wire.endTransmission();
}

// === Mostrar los valores por consola ===
void printSensorData() {
  Serial.println("📤 Enviando valores simulados:");
  for (int i = 0; i < 3; i++) {
    Serial.printf("Cuerda %d → SoftPot: %4d  FSR: %4d  DotFSR: %4d\n",
      i, sensorValues[i * 3 + 0], sensorValues[i * 3 + 1], sensorValues[i * 3 + 2]);
  }
  Serial.println();
}
