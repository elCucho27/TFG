#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const char* ssid = "TP-Link_3506";
const char* password = "30517493";
const char* host = "192.168.1.48"; // Reemplaza con la IP de tu PC
const int port = 8000; // Puerto en el que Purr Data está escuchando

WiFiUDP Udp;

// Definir los pines a los que están conectados los potenciómetros
const int potPinPitch = 33;
const int potPinVolume = 32;
const int potPinModulation = 34;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("Conectado a Wi-Fi");
  
  // Configurar los pines de los potenciómetros como entradas
  pinMode(potPinPitch, INPUT);
  pinMode(potPinVolume, INPUT);
  pinMode(potPinModulation, INPUT);
}

void loop() {
  int pitchValue = analogRead(potPinPitch);
  int volumeValue = analogRead(potPinVolume);
  int modulationValue = analogRead(potPinModulation);

  // Mapear los valores leídos al rango de 0 a 127
  int pitch = map(pitchValue, 0, 4095, 0, 127);
  int volume = map(volumeValue, 0, 4095, 0, 127);
  int modulation = map(modulationValue, 0, 4095, 0, 127);

  // Imprimir los valores en el Monitor Serie para depuración
  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print("\tVolume: ");
  Serial.print(volume);
  Serial.print("\tModulation: ");
  Serial.println(modulation);

  // Enviar los valores mediante OSC
  OSCMessage msgPitch("/pitch");
  msgPitch.add(pitch);
  Udp.beginPacket(host, port);
  msgPitch.send(Udp);
  Udp.endPacket();
  msgPitch.empty();

  OSCMessage msgVolume("/volume");
  msgVolume.add(volume);
  Udp.beginPacket(host, port);
  msgVolume.send(Udp);
  Udp.endPacket();
  msgVolume.empty();

  OSCMessage msgModulation("/modulation");
  msgModulation.add(modulation);
  Udp.beginPacket(host, port);
  msgModulation.send(Udp);
  Udp.endPacket();
  msgModulation.empty();

  delay(100); // Esperar 100 ms antes de la siguiente lectura
}

