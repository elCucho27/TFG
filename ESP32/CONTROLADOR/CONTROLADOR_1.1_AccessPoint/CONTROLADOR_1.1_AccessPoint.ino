#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "ESP32_AP_OSC";  // Nombre de la red Wi-Fi
const char* password = "12345678";  // Contraseña

WiFiUDP Udp;
const int localPort = 8000; // Puerto UDP

void setup() {
  Serial.begin(115200);

  // Configurar ESP32 como Access Point (Access Point)
  WiFi.softAP(ssid, password);
  Serial.println("Red Wi-Fi creada. Esperando clientes...");

  // Iniciar UDP
  Udp.begin(localPort);
}

void loop() {
  char mensajeOSC[255];
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(mensajeOSC, 255);
    if (len > 0) {
      mensajeOSC[len] = '\0';
    }
    Serial.print("Mensaje OSC recibido: ");
    Serial.println(mensajeOSC);
  }
}
