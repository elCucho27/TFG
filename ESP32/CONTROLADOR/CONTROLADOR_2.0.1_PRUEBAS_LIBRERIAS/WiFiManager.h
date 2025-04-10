#ifndef WIFI_MANAGER   // Protección contra inclusiones múltiples
#define WIFI_MANAGER

#include <WiFi.h>              // Incluye la biblioteca WiFi para ESP32

// Enumeración para definir el modo de conexión
enum class WiFiMode {
  CLIENT,         // Modo Cliente: se conecta a una red WiFi existente
  ACCESS_POINT    // Modo AP: actúa como punto de acceso
};

// La clase WiFiManager gestiona la conexión WiFi en modos de cliente o AP.
class WiFiManager {
  public:
    // Constructor: no requiere parámetros, se configura con los métodos siguientes.
    WiFiManager();

    // Conecta al ESP32 a una red WiFi existente como cliente.
    // Recibe el SSID y la contraseña.
    void connectAsClient(const char* ssid, const char* password);

    // Configura el ESP32 como punto de acceso (AP) con el SSID y contraseña proporcionados.
    void startAP(const char* ssid, const char* password);

    // Retorna true si el dispositivo está conectado a una red (modo cliente) o si se ha iniciado el AP.
    bool isConnected();
};

#endif  // WIFI_MANAGER
