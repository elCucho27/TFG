#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define IDENTIFY_COMMAND 0x01
#define PACKET_TYPE_IDENTIFY 0x01
#define PACKET_TYPE_SENSOR   0x02

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin();
  Serial.println("[MASTER] Iniciando escaneo...");

  // ---- ENVÍA COMANDO DE IDENTIFY ----
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(IDENTIFY_COMMAND);  // 0x01
  Wire.endTransmission();
  delay(50); // Espera para que el esclavo se prepare

  // ---- SOLICITA PAQUETE IDENTIFY ----
  Wire.requestFrom(SLAVE_ADDRESS, 1 + 6);
  if (Wire.available() == 7) {
    uint8_t type = Wire.read();
    if (type == PACKET_TYPE_IDENTIFY) {
      char id[7];
      for (int i = 0; i < 6; i++) {
        id[i] = Wire.read();
      }
      id[6] = '\0';
      Serial.print("[MASTER] Identificado como: ");
      Serial.println(id);
    } else {
      Serial.printf("[MASTER] Tipo inesperado (esperado IDENTIFY): 0x%02X\n", type);
    }
  } else {
    Serial.println("[MASTER] Paquete IDENTIFY malformado.");
  }
}


void loop() {
  delay(500);

  Wire.requestFrom(SLAVE_ADDRESS, 1 + 6); // Tipo + 3*uint16_t = 7 bytes
  if (Wire.available() == 7) {
    uint8_t type = Wire.read();
    if (type == PACKET_TYPE_SENSOR) {
      uint16_t values[3];
      for (int i = 0; i < 3; i++) {
        uint8_t lsb = Wire.read();
        uint8_t msb = Wire.read();
        values[i] = (msb << 8) | lsb;
      }

      Serial.println("====== SENSOR DATA ======");
      Serial.println("| # |   Valor   |");
      Serial.println("|---|-----------|");
      for (int i = 0; i < 3; i++) {
        Serial.printf("| %d | %9u |\n", i + 1, values[i]);
      }
      Serial.println("=========================");
    } else {
      Serial.printf("[MASTER] Tipo inesperado: 0x%02X\n", type);
    }
  } else {
    Serial.println("[MASTER] Paquete SENSOR malformado.");
  }
}
