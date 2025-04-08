#ifndef OSC_RECEIVER_H        // Evita inclusiones múltiples del header
#define OSC_RECEIVER_H

#include <WiFi.h>              // Biblioteca para las funciones WiFi en ESP32
#include <WiFiUdp.h>           // Para la comunicación UDP
#include <OSCMessage.h>        // Biblioteca para el manejo de mensajes OSC

// Definición de un tipo de función para callbacks que procesen mensajes OSC.
// La función recibirá una referencia a OSCMessage, la IP del remitente y el puerto.
typedef void (*OSCMessageCallback)(OSCMessage &msg, IPAddress ip, int port);

// La clase OSCReceiver encapsula la funcionalidad de recibir mensajes OSC.
class OSCReceiver {
  public:
    // Constructor sin argumentos. Inicializa el puerto a 7000 y sin callback asignado.
    OSCReceiver();

    // Método para iniciar el receptor. Se especifica el puerto local para escuchar.
    // Por defecto se usará el puerto 7000.
    void begin(int port = 7000);

    // Método que se invoca en el loop principal para procesar mensajes entrantes.
    void process();

    // Permite asignar una función callback personalizada para el procesamiento de mensajes OSC.
    void setCallback(OSCMessageCallback callback);

  private:
    WiFiUDP udp;                     // Objeto para la comunicación UDP
    int localPort;                   // Puerto UDP en el que se escucha (por defecto 7000)
    OSCMessageCallback messageCallback; // Función callback para procesar mensajes (opcional)
};

#endif  // OSC_RECEIVER_H
