#include "WiFiManager_LAVANA.h"   // Gestión de la conexión WiFi (cliente o AP)
#include "OSCReceiver.h"          // Módulo receptor OSC

// =====================
// CONFIGURACIÓN DE WIFI
// =====================

// Cambia a false para usar el modo AP (Access Point) en lugar de cliente.
#define CLIENT_MODE true     

// Configuración para ambos modos: si se usa modo cliente, estos serán los datos de la red a la que conectar.
// En modo AP, estos serán el nombre y contraseña del punto de acceso que creará el ESP32.
#define WIFI_SSID "TP-Link_3506"        
#define WIFI_PASS "30517493"

// =====================
// DECLARACIÓN DE OBJETOS GLOBALES
// =====================

// Objeto para gestionar la conexión WiFi.
WiFiManager_LAVANA wifiManager;
// Objeto para recibir mensajes OSC.
OSCReceiver oscReceiver;

// =====================
// setup(): se ejecuta una sola vez al inicio.
void setup() {
  // Inicializa el Monitor Serial para depuración.
  Serial.begin(115200);
  delay(1000);  // Pequeña demora para permitir la inicialización del Serial
  
  // Configuración de WiFi según el modo elegido.
  if (CLIENT_MODE) {
    // Si CLIENT_MODE es verdadero, se conecta a una red existente.
    wifiManager.connectAsClient(WIFI_SSID, WIFI_PASS);
  } else {
    // Si CLIENT_MODE es falso, el ESP32 se configura como un punto de acceso.
    wifiManager.startAP(WIFI_SSID, WIFI_PASS);
  }
  
  // Inicia el receptor OSC en el puerto 7000.
  oscReceiver.begin(7000);
  
  // Mensaje de inicio de la aplicación.
  Serial.println("Controlador OSC listo para recibir mensajes");
}

// =====================
// loop(): se ejecuta repetidamente.
void loop() {
  // Procesa cualquier mensaje OSC entrante.
  // Se llama a oscReceiver.process() en cada iteración para revisar si hay datos.
  oscReceiver.process();
  
  // Breve retraso para no saturar el procesador.
  delay(10);
}
