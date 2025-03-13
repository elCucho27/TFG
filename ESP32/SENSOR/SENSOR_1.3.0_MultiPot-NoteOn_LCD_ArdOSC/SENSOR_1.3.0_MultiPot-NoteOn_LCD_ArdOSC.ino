#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>  // Librería OSC
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

const char* ssid = "ESP32_AP_OSC";
const char* password = "12345678";

WiFiUDP Udp;
const int remotePort = 8000;
IPAddress serverIP(192, 168, 4, 1);

const int AnalogPot1 = 33;
const int AnalogPot2 = 32;
const int AnalogPot3 = 34;

int pot1value = 0, pot2value = 0, pot3value = 0;
int lastMidiValue1 = -1, lastMidiValue2 = -1, lastMidiValue3 = -1;

void setup() {
  Serial.begin(115200);
  pinMode(AnalogPot1, INPUT);
  pinMode(AnalogPot2, INPUT);
  pinMode(AnalogPot3, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red del controlador (AP)");
  Udp.begin(8000); 
}

void loop() {
  potRead();
  sendOSC();

  delay(20);
}

void potRead() {
  pot1value = analogRead(AnalogPot1);
  pot2value = analogRead(AnalogPot2);
  pot3value = analogRead(AnalogPot3);

  pot1value = map(pot1value, 0, 4095, 0, 127);
  pot2value = map(pot2value, 0, 4095, 0, 127);
  pot3value = map(pot3value, 0, 4095, 0, 127);
}

void sendOSC() {
  // Enviar valor potenciómetro 1
  if (pot1value != lastMidiValue1) {
    OSCMessage msg;
    msg.setAddress("/pot1");
    msg.add(pot1value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();

    lastMidiValue1 = pot1value;
  }

  // Enviar valor potenciómetro 2
  if (pot2value != lastMidiValue2) {
    OSCMessage msg;
    msg.setAddress("/pot2");
    msg.add(pot2value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();

    lastMidiValue2 = pot2value;
  }

  // Enviar valor potenciómetro 3
  if (pot3value != lastMidiValue3) {
    OSCMessage msg;
    msg.setAddress("/pot3");
    msg.add(pot3value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();

    lastMidiValue3 = pot3value;
  }
}
