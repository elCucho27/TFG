// MercuryOSC: Ethernet OSC + I2C esclavo (ETH en núcleo principal, I2C en tarea separada)
// ESP32-WROOM-32 + LAN8720 + 9 sensores simulados (3 cuerdas)

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

// === I2C (como esclavo) ===
#define I2C_SLAVE_ADDR 0x08
uint8_t sensorBuffer[18]; // 9 sensores x 2 bytes
volatile bool sendRequested = false;

// === Simulación de sensores ===
uint16_t fakeValues[9] = {0};
unsigned long lastSim = 0;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("mercuri-eth");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("[ETH] IP: "); Serial.println(ETH.localIP());
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

void onRequest() {
  Wire.write(sensorBuffer, sizeof(sensorBuffer));
  sendRequested = false;
}

void onReceive(int len) {
  while (Wire.available()) Wire.read(); // Ignorar comandos
  sendRequested = true;
}

// === Tarea dedicada a inicializar I2C esclavo en otro núcleo ===
void i2cTask(void* parameter) {
  delay(2000); // Esperar a que ETH se estabilice

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest);
  Wire.onReceive(onReceive);
  Serial.println("[I2C] Modo esclavo iniciado desde tarea");

  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== MercuryOSC: ETH primero, I2C en tarea =====");

  WiFi.onEvent(WiFiEvent);
  bool eth_ok = ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  if (!eth_ok) Serial.println("[ETH] ❌ Fallo al iniciar Ethernet");

  // Crear tarea dedicada para activar I2C esclavo
  xTaskCreatePinnedToCore(
    i2cTask,          // función
    "I2CSlaveTask",   // nombre
    4096,             // tamaño de pila
    NULL,             // parámetro
    1,                // prioridad
    NULL,             // manejador
    1                 // núcleo (0 o 1)
  );
}

void loop() {
  unsigned long now = millis();

  // Simulación de sensores cada 500 ms
  if (now - lastSim > 500) {
    lastSim = now;
    for (int i = 0; i < 9; i++) {
      fakeValues[i] = random(0, 4096);
      sensorBuffer[2 * i] = highByte(fakeValues[i]);
      sensorBuffer[2 * i + 1] = lowByte(fakeValues[i]);
    }
    if (eth_connected) enviarOSC();
  }
}

void enviarOSC() {
  for (int i = 0; i < 3; i++) {
    OSCMessage msg(String("/3stringController/String" + String(i + 1)).c_str());

    msg.add((intOSC_t)fakeValues[i * 3 + 0]); // Note
    msg.add((intOSC_t)fakeValues[i * 3 + 1]); // Vel
    msg.add((intOSC_t)fakeValues[i * 3 + 2]); // Mod
    msg.add((intOSC_t)0);                    // Pitch bend dummy

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }
  Serial.println("[OSC] Simulación enviada");
}
