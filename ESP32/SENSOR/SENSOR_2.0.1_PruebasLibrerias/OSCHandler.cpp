#include "OSCHandler.h"

OSCHandler::OSCHandler(const char* ssid, const char* password, const char* host, int port)
  : _ssid(ssid), _password(password), _host(host), _port(port) {}

void OSCHandler::begin() {
  WiFi.begin(_ssid, _password); // Inicia la conexión WiFi
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  _udp.begin(WiFi.localIP(), _port); // Inicia UDP desde IP local
}

void OSCHandler::sendSensorValue(Sensor* s) {
  float value = s->getValue(); // Obtiene el valor del sensor
  OSCMessage msg(s->getAddress()); // Crea mensaje OSC con su dirección
  msg.add(value); // Añade valor
  _udp.beginPacket(_host, _port); // Prepara envío
  msg.send(_udp); // Envia mensaje
  _udp.endPacket(); // Finaliza envío
  msg.empty(); // Limpia mensaje

  Serial.print("Enviado OSC -> ");
  Serial.print(s->getAddress());
  Serial.print(": ");
  Serial.println(value, 3); // Imprime en consola
}
