#include "WiFiManager.h"   // Librería para gestionar la conexión WiFi en modo AP o cliente.
#include "OSCReceiver.h"          // Librería para recibir mensajes OSC a través de UDP.

// =====================
// CONFIGURACIÓN DE WIFI PARA EL MODO AP
// =====================

#define CLIENT_MODE false        // Usamos false para que se active el modo AP.
#define WIFI_SSID "ESP32_S3_AP"    // Nombre del punto de acceso que creará el ESP32.
#define WIFI_PASS "LAVANA"     // Contraseña para el punto de acceso.

// =====================
// DECLARACIÓN DE OBJETOS GLOBALES
// =====================

WiFiManager wifiManager;  // Objeto para gestionar la conexión WiFi.
OSCReceiver oscReceiver;         // Objeto para recibir mensajes OSC.

// =====================
// setup(): se ejecuta una sola vez al inicio
// =====================
void setup() {
  // Inicializa el Monitor Serie para la depuración.
  Serial.begin(115200);
  delay(1000);  // Breve espera para que el Monitor Serie se inicie correctamente.

  // Configura el ESP32-S3 en modo AP (Access Point) usando WiFiManager_LAVANA.
  wifiManager.startAP(WIFI_SSID, WIFI_PASS);

  // Inicia el receptor OSC en el puerto 7000.
  oscReceiver.begin(7000);

  // Mensaje de inicio en el Monitor Serie.
  Serial.println("Controlador OSC en modo AP listo para recibir mensajes.");
}

// =====================
// loop(): se ejecuta repetidamente
// =====================
void loop() {
  // Procesa mensajes OSC entrantes; oscReceiver.process() se encarga de leer el socket UDP,
  // interpretar los mensajes OSC y, si no se ha definido un callback personalizado, imprimirlos por Monitor Serie.
  oscReceiver.process();

  // Retraso breve para no saturar el procesador.
  delay(10);
}
