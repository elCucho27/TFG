#include "SensorManager.h"
#include "OSCHandler.h"

#define WIFI_SSID     "TP-Link_3506"        // Nombre de la red WiFi
#define WIFI_PASS     "30517493"            // Contraseña
#define DEST_IP       "192.168.1.255"        // IP destino donde va el mensaje OSC (otra ESP32 o PC) habitualmente se usará BROADCAST
#define DEST_PORT     7000                  // Puerto en el que escucha el receptor OSC (ej. Purr Data)

#define FSR_PIN 33
#define DOT_PIN 34
#define SPOTM_PIN 27

SensorManager sensorManager;
OSCHandler osc(WIFI_SSID, WIFI_PASS, DEST_IP, DEST_PORT);

void setup() {
  Serial.begin(115200);

  osc.begin();

  sensorManager.addSensor(FSR_PIN, "/LAVANA/fsr", 0, 4095, 0.1);
  sensorManager.addSensor(DOT_PIN, "/LAVANA/fsr", 0, 4095, 0.1);
  sensorManager.addSensor(SPOTM_PIN, "/LAVANA/fsr", 0, 4095, 0.1);

}

void loop() {
  
  sensorManager.updateSensors();

  for(int i = 0; i < sensorManager.getSensorCount(); i++){
    Sensor* sensor = sensorManager.getSensor(i);

    if(sensor->hasChanged()){
      osc.sendSensorValue(sensor);
    }
    delay(30);
  }

}
