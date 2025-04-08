#include "WiFiManager_LAVANA.h" // Incluye el header correspondiente

// Constructor vacío
WiFiManager_LAVANA::WiFiManager_LAVANA() {
  // No se requiere inicialización especial en el constructor
}

// connectAsClient(): Conecta el ESP32 a una red WiFi existente en modo estación.
void WiFiManager_LAVANA::connectAsClient(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);           // Establece el modo WiFi como Cliente (Station)
  WiFi.begin(ssid, password);    // Inicia la conexión a la red con las credenciales dadas
  
  Serial.print("Conectando a WiFi (cliente) ");
  Serial.print(ssid);
  Serial.println("...");
  
  // Espera hasta conectarse. Este bucle se puede limitar en un futuro.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConectado a WiFi como cliente!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());  // Imprime la IP asignada al ESP32
}

// startAP(): Configura el ESP32 como un punto de acceso.
void WiFiManager_LAVANA::startAP(const char* ssid, const char* password) {
  WiFi.mode(WIFI_AP);            // Establece el modo AP (Access Point)
  WiFi.softAP(ssid, password);   // Crea el punto de acceso con el SSID y contraseña proporcionados
  
  Serial.print("Access Point creado: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());  // Imprime la IP asignada al AP
}

// isConnected(): Retorna verdadero si el ESP32 está conectado a una red (para modo cliente).
bool WiFiManager_LAVANA::isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}
