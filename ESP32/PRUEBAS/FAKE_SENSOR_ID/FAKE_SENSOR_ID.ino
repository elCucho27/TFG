#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define SDA_PIN 21
#define SCL_PIN 22
#define IDENTIFY_COMMAND 0x01
#define PACKET_TYPE_IDENTIFY 0x01
#define PACKET_TYPE_SENSOR   0x02

const char IDENTIFIER[7] = "SENS_X"; // Máx. 6 caracteres + cabecera + sin '\0'

volatile uint8_t lastCommand = 0;
volatile bool identifyRequested = false;

void setup() {
  Serial.begin(115200);
  delay(500); // Espera para monitor

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Serial.println("[SLAVE] I2C esclavo iniciado.");
}

void loop() {
  delay(10);
}

void receiveEvent(int howMany) {
  if (howMany > 0) {
    lastCommand = Wire.read();
    if (lastCommand == IDENTIFY_COMMAND) {
      identifyRequested = true;
    }
    while (Wire.available()) Wire.read(); // Limpia buffer
  }
}

void requestEvent() {
  if (identifyRequested) {
    identifyRequested = false; // ✅ Limpia el flag ANTES de responder
    Wire.write(PACKET_TYPE_IDENTIFY);
    Wire.write((const uint8_t*)IDENTIFIER, sizeof(IDENTIFIER));
    Serial.println("[SLAVE] IDENTIFIER enviado.");
  } else {
    Wire.write(PACKET_TYPE_SENSOR);
    for (int i = 0; i < 3; i++) {
      uint16_t val = random(0, 4096);
      Wire.write(lowByte(val));
      Wire.write(highByte(val));
    }
    Serial.println("[SLAVE] Datos aleatorios enviados.");
  }
}

