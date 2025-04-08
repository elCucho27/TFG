#include "OSCReceiver.h"   // Incluye su propio header

// Constructor: inicializa el puerto local a 7000 y sin callback
OSCReceiver::OSCReceiver() : localPort(7000), messageCallback(NULL) {
  // Constructor vacío: no se requiere más inicialización aquí
}

// Método begin(): Inicializa el UDP en el puerto deseado para recibir OSC.
void OSCReceiver::begin(int port) {
  localPort = port;              // Asigna el puerto local
  udp.begin(localPort);          // Inicia la escucha UDP en ese puerto
  // Imprime en Serial información del inicio del receptor
  Serial.print("OSC Receiver iniciado en el puerto ");
  Serial.println(localPort);
}

// Permite asignar un callback que se llamará cada vez que se reciba un mensaje OSC.
void OSCReceiver::setCallback(OSCMessageCallback callback) {
  messageCallback = callback;
}

// Método process(): Se debe llamar periódicamente en el loop para revisar mensajes entrantes.
void OSCReceiver::process() {
  // Comprueba si hay un paquete UDP entrante
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {                  // Si se ha recibido un paquete
    const int bufferSize = 512;          // Tamaño máximo del buffer para recibir datos (ajustable)
    uint8_t buffer[bufferSize];          // Buffer temporal para almacenar datos recibidos
    
    int len = udp.read(buffer, bufferSize); // Lee los datos entrantes
    if (len > 0) {
      // Añade un terminador nulo por seguridad (no es obligatorio para OSC, pero es buena práctica)
      buffer[len] = 0;
    }
    
    // Crea un objeto OSCMessage para interpretar el contenido del buffer
    OSCMessage msg;
    msg.fill(buffer, len);           // Llena el objeto con los datos recibidos
    
    // Obtiene la IP y el puerto del emisor, para información en la depuración
    IPAddress remoteIp = udp.remoteIP();
    int remotePort = udp.remotePort();
    
    // Si se ha asignado un callback, se llama a la función proporcionada
    if (messageCallback) {
      messageCallback(msg, remoteIp, remotePort);
    } else {
      // Si no se ha definido un callback, se imprime información de depuración por Serial
      Serial.println("[OSC Receiver] Mensaje recibido");
      Serial.print("Ruta: ");
      Serial.println(msg.getAddress());
      
      // Recorre los argumentos del mensaje e imprime cada valor.
      // Se comprueba el tipo de cada argumento.
      for (int i = 0; i < msg.size(); i++) {
        if (msg.isFloat(i)) {  // Si es un argumento float
          Serial.print("Valor[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(msg.getFloat(i));
        } else if (msg.isInt(i)) {  // Si es un entero
          Serial.print("Valor[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(msg.getInt(i));
        } else {
          // Puedes extender la verificación a otros tipos según se requiera.
          Serial.print("Valor[");
          Serial.print(i);
          Serial.println("]: (tipo no reconocido)");
        }
      }
      
      // Imprime la IP y el puerto del dispositivo que envió el mensaje
      Serial.print("Desde IP: ");
      Serial.println(remoteIp);
      Serial.print("Puerto: ");
      Serial.println(remotePort);
      Serial.println("-------------------------");
    }
  }
}
