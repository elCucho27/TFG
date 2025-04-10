#ifndef OSCHANDLER_H
#define OSCHANDLER_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include "Sensor.h"

class OSCHandler {
  public:
    OSCHandler(const char* ssid, const char* password, const char* host, int port);
    void begin();                       // Conecta al WiFi e inicia UDP
    void sendSensorValue(Sensor* s);   // Envía valor de un sensor por OSC

  private:
    const char* _ssid;                 // SSID WiFi
    const char* _password;             // Contraseña WiFi
    const char* _host;                 // IP de destino
    int _port;                         // Puerto de destino
    WiFiUDP _udp;                      // Objeto UDP
};

#endif
