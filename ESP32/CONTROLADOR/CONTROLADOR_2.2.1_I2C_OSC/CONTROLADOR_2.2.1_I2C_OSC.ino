#include <Wire.h>

// Dirección del esclavo I2C (sensor)
#define SLAVE_ADDRESS 0x08

#define SDA_MASTER 8
#define SCL_MASTER 9

#define I2C_HEADER_BYTE1 0xAA
#define I2C_HEADER_BYTE2 0x55

uint8_t buffer[64];  // Buffer para recibir los datos

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_MASTER, SCL_MASTER); // Inicializamos como Master
  delay(1000);
  Serial.println("Iniciado Controlador I2C...");
}

void loop() {
  // Solicitar 64 bytes (máximo que esperamos recibir)
  Wire.requestFrom(SLAVE_ADDRESS, (uint8_t)64);

  int index = 0;
  while (Wire.available()) {
    buffer[index++] = Wire.read();
  }

  if (index < 5) {
    Serial.println("Paquete demasiado pequeño, ignorado.");
    delay(50);
    return;
  }

  // Verificar cabeceras
  if (buffer[0] != I2C_HEADER_BYTE1 || buffer[1] != I2C_HEADER_BYTE2) {
    Serial.println("Cabecera inválida, ignorando paquete.");
    delay(50);
    return;
  }

  uint8_t payloadLength = buffer[2];
  
  // Verificar que tenemos todos los datos
  if (payloadLength + 4 > index) {
    Serial.println("Datos incompletos, esperando más.");
    delay(50);
    return;
  }

  uint8_t checksumReceived = buffer[3 + payloadLength];
  uint8_t checksumCalculated = 0;
  for (uint8_t i = 3; i < 3 + payloadLength; i++) {
    checksumCalculated += buffer[i];
  }

  if (checksumReceived != checksumCalculated) {
    Serial.println("Checksum inválido, descartando paquete.");
    delay(50);
    return;
  }

  // -------------------
  // Interpretar el payload
  // -------------------
  uint8_t pos = 3;  // Payload empieza después de cabeceras y longitud

  uint8_t numSoftPots = buffer[pos++];
  Serial.print("SoftPots detectados: ");
  Serial.println(numSoftPots);

  for (uint8_t i = 0; i < numSoftPots; i++) {
    uint16_t val = (buffer[pos++] << 8) | buffer[pos++];
    Serial.print("SoftPot ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(val);
  }

  uint8_t numFSRs = buffer[pos++];
  Serial.print("FSRs detectados: ");
  Serial.println(numFSRs);

  for (uint8_t i = 0; i < numFSRs; i++) {
    uint16_t val = (buffer[pos++] << 8) | buffer[pos++];
    Serial.print("FSR ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(val);
  }

  delay(50);  // Evitar saturar el bus
}
