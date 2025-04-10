#include "SensorManager.h"
#include "OSCHandler.h"
#include <WiFi.h>  // Incluye directamente la librería WiFi

// =====================
// CONFIGURACIÓN DE WIFI PARA CONECTARSE AL AP DEL CONTROLADOR
// =====================
#define WIFI_SSID     "ESP32_S3_AP"   // Nombre del AP creado por el controlador
#define WIFI_PASS     "LAVANA"        // Contraseña del AP

// Dirección de destino: usamos la dirección de broadcast de la red típica de un ESP32 en modo AP.
#define DEST_IP       "192.168.4.255"  // Broadcast: envía a todos los dispositivos conectados al AP
#define DEST_PORT     7000             // Puerto en el que el controlador está escuchando

// =====================
// CONFIGURACIÓN DE SENSORES
// =====================
#define FSR_PIN       33
#define DOT_PIN       34
#define SPOTM_PIN     27

// =====================
// DECLARACIÓN DE OBJETOS GLOBALES
// =====================
SensorManager sensorManager;                        // Objeto para gestionar múltiples sensores
OSCHandler osc(WIFI_SSID, WIFI_PASS, DEST_IP, DEST_PORT);  // Objeto para enviar mensajes OSC

void setup() {
  Serial.begin(115200);        // Inicia la comunicación Serial para depuración.
  delay(1000);                 // Breve espera para que el Monitor Serie se inicie correctamente.
  
  // Conexión a la red WiFi sin usar WiFiManager:
  WiFi.mode(WIFI_STA);         // Configura el ESP32 en modo cliente
  WiFi.begin(WIFI_SSID, WIFI_PASS); // Inicia la conexión a la red con los datos proporcionados.
  
  Serial.print("Conectando a WiFi ");
  Serial.print(WIFI_SSID);
  Serial.println(" ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi como cliente!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Inicia el manejador OSC para el envío de mensajes.
  osc.begin();

  // Agrega sensores: define cada sensor con su pin, ruta OSC, rango esperado y factor de filtrado.
  sensorManager.addSensor(FSR_PIN, "/LAVANA/fsr", 0, 4095, 0.1);
  sensorManager.addSensor(DOT_PIN, "/LAVANA/fsr", 0, 4095, 0.1);
  sensorManager.addSensor(SPOTM_PIN, "/LAVANA/fsr", 0, 4095, 0.1);
}

void loop() {
  // Actualiza todos los sensores: cada sensor lee su valor, se normaliza y se filtra.
  sensorManager.updateSensors();

  // Recorre cada sensor y si ha cambiado, se envía su valor por OSC.
  for (int i = 0; i < sensorManager.getSensorCount(); i++) {
    Sensor* sensor = sensorManager.getSensor(i);
    if (sensor->hasChanged()) {
      osc.sendSensorValue(sensor);
    }
    delay(30);  // Pequeño retraso para evitar lecturas demasiado rápidas.
  }
}
