// MercuryOSC: Ethernet OSC + I2C maestro
// ESP32-WROOM-32 + LAN8720

#include <ETH.h>
#include <WiFi.h>
#include <Wire.h>
#include <OSCMessage.h>
#include <WiFiUdp.h>

// === CONFIGURACIÓN OSC ===
IPAddress outIp(192, 168, 1, 102);  // IP del receptor OSC (PC)
const unsigned int outPort = 9000; // Puerto destino
WiFiUDP Udp;

// === ETHERNET (LAN8720) ===
#define ETH_PHY_TYPE      ETH_PHY_LAN8720
#define ETH_PHY_ADDR      1
#define ETH_PHY_POWER     17
#define ETH_PHY_MDC       23
#define ETH_PHY_MDIO      18
#define ETH_CLK_MODE      ETH_CLOCK_GPIO0_IN
bool eth_connected = false;

// === I2C MAESTRO ===
#define I2C_SDA 33
#define I2C_SCL 32
#define SLAVE_ADDR 0x08
uint8_t sensorBuffer[18]; // 9 sensores x 2 bytes

// === Comunicación con esclavo I2C ===
bool leerSensoresDesdeEsclavo() {
  // Solicitar directamente los datos al esclavo (sin comando previo)
  int bytesRead = Wire.requestFrom(SLAVE_ADDR, sizeof(sensorBuffer));
  if (bytesRead != sizeof(sensorBuffer)) return false;

  for (int i = 0; i < sizeof(sensorBuffer); i++) {
    sensorBuffer[i] = Wire.read();
  }
  return true;
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("mercuri-eth");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("[ETH] IP obtenida: "); Serial.println(ETH.localIP());
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[ETH] DESCONECTADO");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== MercuryOSC: ETH + I2C maestro =====");

  WiFi.onEvent(WiFiEvent);
  bool eth_ok = ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  if (!eth_ok) Serial.println("[ETH] ❌ Fallo al iniciar Ethernet");

  Wire.begin(I2C_SDA, I2C_SCL, 400000); // I2C maestro
  Serial.println("[I2C] Bus iniciado con 400kHz");
}

void loop() {
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 100) {
    lastRead = millis();
    if (leerSensoresDesdeEsclavo()) {
      enviarOSC();
    } else {
      Serial.println("[I2C] ❌ Fallo en lectura de esclavo");
    }
  }
}

void enviarOSC() {
  if (!eth_connected) return;
  for (int i = 0; i < 3; i++) {
    OSCMessage msg(String("/3stringController/String" + String(i + 1)).c_str());
    msg.add((intOSC_t)((sensorBuffer[i * 6 + 0] << 8) | sensorBuffer[i * 6 + 1])); // SoftPot
    msg.add((intOSC_t)((sensorBuffer[i * 6 + 2] << 8) | sensorBuffer[i * 6 + 3])); // FSR
    msg.add((intOSC_t)((sensorBuffer[i * 6 + 4] << 8) | sensorBuffer[i * 6 + 5])); // DotFSR
    msg.add((intOSC_t)0);  // Dummy pitchbend

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }
  Serial.println("[OSC] Datos enviados");
}
