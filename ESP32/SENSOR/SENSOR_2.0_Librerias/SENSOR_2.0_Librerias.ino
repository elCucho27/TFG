#include "SensorManager.h"    // Incluye la gestión de sensores
#include "OSCHandler.h"       // Incluye el envío de mensajes OSC

//  CONFIGURACIÓN DE RED Y OSC
#define WIFI_SSID     "TP-Link_3506"        // Nombre de la red WiFi
#define WIFI_PASS     "30517493"            // Contraseña
#define DEST_IP       "192.168.1.48"        // IP destino donde va el mensaje OSC (otra ESP32 o PC)
#define DEST_PORT     8000                  // Puerto en el que escucha el receptor OSC (ej. Purr Data)

//  CONFIGURACIÓN DE PINES ANALÓGICOS
// FSR 500mm
#define FSR1_PIN      33
#define FSR2_PIN      32
#define FSR3_PIN      35

// Dot FSR
#define DOT1_PIN      34
#define DOT2_PIN      39
#define DOT3_PIN      36

// SoftPot Membrane
#define SFT1_PIN      25
#define SFT2_PIN      26
#define SFT3_PIN      27

//  OBJETOS GLOBALES
SensorManager sensorManager;                // Crea instancia del gestor de sensores
OSCHandler osc(WIFI_SSID, WIFI_PASS, DEST_IP, DEST_PORT); // Crea el objeto de envío OSC

void setup() {
  Serial.begin(115200);                     // Inicializa el monitor serie

  // Inicializa WiFi y socket UDP
  osc.begin();

  //  Agrega todos los sensores al manager
  //         PIN , "DIRECCION_OSC", VAL_MIN , VAL_MAX , FILTRO_ALPHA
  sensorManager.addSensor(FSR1_PIN, "/LAVANA/fsr1", 0, 4095, 0.1);
  sensorManager.addSensor(FSR2_PIN, "/LAVANA/fsr2", 0, 4095, 0.1);
  sensorManager.addSensor(FSR3_PIN, "/LAVANA/fsr3", 0, 4095, 0.1);

  sensorManager.addSensor(DOT1_PIN, "/LAVANA/dot1", 0, 4095, 0.1);
  sensorManager.addSensor(DOT2_PIN, "/LAVANA/dot2", 0, 4095, 0.1);
  sensorManager.addSensor(DOT3_PIN, "/LAVANA/dot3", 0, 4095, 0.1);

  sensorManager.addSensor(SFT1_PIN, "/LAVANA/sft1", 0, 4095, 0.1);
  sensorManager.addSensor(SFT2_PIN, "/LAVANA/sft2", 0, 4095, 0.1);
  sensorManager.addSensor(SFT3_PIN, "/LAVANA/sft3", 0, 4095, 0.1);
}

void loop() {
  sensorManager.updateSensors();            // Lee todos los sensores

  // Recorre todos los sensores y envía solo los que han cambiado
  for (int i = 0; i < sensorManager.getSensorCount(); i++) {
    Sensor* sensor = sensorManager.getSensor(i);

    if (sensor->hasChanged()) {             // Solo si el valor cambió
      osc.sendSensorValue(sensor);          // Enviar por OSC
    }
  }

  delay(30);  // Controla la frecuencia de muestreo (~33Hz)
}
