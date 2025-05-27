#include <Wire.h>

#define I2C_ADDRESS 0x13

// Estructura que devuelve este módulo
struct SensorDataPacket {
  uint16_t softpot[3];
  uint16_t fsr[3];
  uint16_t dotfsr[3];
};

SensorDataPacket currentData;

// Buffer de comandos recibidos
char commandBuffer[16];
uint8_t commandLength = 0;
bool commandReady = false;

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(I2C_ADDRESS); // SDA/SCL por defecto = 21/22
  Wire.onReceive(onReceiveHandler);
  Wire.onRequest(onRequestHandler);

  Serial.printf("📟 Módulo SENSOR activo en dirección 0x%02X\n", I2C_ADDRESS);
}

void loop() {
  // Simulación de lectura de sensores reales
  for (int i = 0; i < 3; ++i) {
    currentData.softpot[i] = random(100, 3000);
    currentData.fsr[i] = random(0, 4095);
    currentData.dotfsr[i] = random(0, 4095);
  }
  delay(20); // Simulación de latencia
}

void onReceiveHandler(int bytesReceived) {
  commandLength = 0;
  while (Wire.available()) {
    char c = Wire.read();
    if (commandLength < sizeof(commandBuffer) - 1) {
      commandBuffer[commandLength++] = c;
    }
  }
  commandBuffer[commandLength] = '\0'; // Terminamos el string
  commandReady = true;

  Serial.print("📨 Comando recibido: ");
  Serial.println(commandBuffer);
}

void onRequestHandler() {
  if (!commandReady) return; // No responder si no hay comando válido

  // Comprobamos si el último comando recibido fue "IDENTIFY"
  if (strncmp(commandBuffer, "IDENTIFY", 8) == 0) {
    Serial.println("✅ Respondiendo a IDENTIFY");

    // Definimos las cadenas como arrays de tipo uint8_t para evitar errores de tipo
    const uint8_t header[] = "LAVANA";   // Cabecera de identificación
    const uint8_t type[]   = "SENSOR";   // Tipo de módulo
    Wire.write(header, sizeof(header) - 1);  // -1 para no enviar el carácter nulo
    Wire.write(type, sizeof(type) - 1);
    Wire.write(1); // Versión mayor
    Wire.write(0); // Versión menor

  }
  // Si el comando fue "READ", enviamos la estructura de datos
  else if (strncmp(commandBuffer, "READ", 4) == 0) {
    Serial.println("📤 Enviando datos de sensor");
    Wire.write((uint8_t*)&currentData, sizeof(currentData));
  }
  // Si el comando no se reconoce, enviamos un mensaje de error
  else {
    Serial.println("❌ Comando no reconocido");
    const uint8_t err[] = "ERR";
    Wire.write(err, sizeof(err) - 1);
  }

  // Borramos el comando actual después de responder
  commandReady = false;
}

