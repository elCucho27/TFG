// Sketch de prueba LAN8720 con ESP32-WROOM-32 (Core ESP32 3.x)
// Basado en la guía de Mischianti
// Ethernet usando ETH.begin() con reloj RMII externo en GPIO0

#include <ETH.h>
#include <WiFi.h> // Necesario para WiFiEvent_t

// === CONFIGURACIÓN ===
#define ETH_PHY_TYPE      ETH_PHY_LAN8720
#define ETH_PHY_ADDR      1              // Común en muchos módulos LAN8720
#define ETH_PHY_POWER     17             // GPIO que controla el oscilador (patch a EN)
#define ETH_PHY_MDC       23
#define ETH_PHY_MDIO      18
#define ETH_CLK_MODE      ETH_CLOCK_GPIO0_IN // Reloj externo de 50 MHz entra por GPIO0

bool eth_connected = false;

// === FUNCIONES ===

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("[EVENT] ETH_START: Interfaz Ethernet inicializada");
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[EVENT] ETH_CONNECTED: Cable conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("[EVENT] ETH_GOT_IP: IP obtenida → ");
      Serial.println(ETH.localIP());
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[EVENT] ETH_DISCONNECTED");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[EVENT] ETH_STOP");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n===== TEST ETHERNET LAN8720 - ESP32 (Core 3.x) =====");

  // Registrar eventos Ethernet
  WiFi.onEvent(WiFiEvent);

  // Iniciar Ethernet
  Serial.println("[INIT] Llamando a ETH.begin()...");
  bool success = ETH.begin(
    ETH_PHY_TYPE,
    ETH_PHY_ADDR,
    ETH_PHY_MDC,
    ETH_PHY_MDIO,
    ETH_PHY_POWER,
    ETH_CLK_MODE
  );

  if (success) {
    Serial.println("[INIT] ✅ ETH.begin() exitoso");
  } else {
    Serial.println("[INIT] ❌ ETH.begin() falló");
  }
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 3000) {
    last = millis();
    Serial.print("[STATE] Conectado: ");
    Serial.print(eth_connected ? "Sí" : "No");
    Serial.print(" | IP: ");
    Serial.println(ETH.localIP());
  }
}
