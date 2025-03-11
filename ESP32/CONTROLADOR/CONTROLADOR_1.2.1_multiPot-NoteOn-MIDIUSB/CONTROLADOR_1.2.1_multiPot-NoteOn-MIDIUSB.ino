#include <WiFi.h>
#include <WiFiUdp.h>
#include <MIDI.h>  // Incluir la librería MIDI para comunicación UART

const char* ssid = "ESP32_AP_OSC";  // Nombre de la red Wi-Fi
const char* password = "12345678";  // Contraseña

WiFiUDP Udp;
const int localPort = 8000; // Puerto UDP

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);  // Crear instancia para UART (puerto serial 2)

void setup() {
  Serial.begin(115200);
  // Configurar ESP32 como Access Point (AP)
  WiFi.softAP(ssid, password);
  Serial.println("Red Wi-Fi creada. Esperando clientes...");

  // Iniciar UDP
  Udp.begin(localPort);

  // Iniciar puerto MIDI UART
  Serial2.begin(31250);  // Velocidad de comunicación MIDI por UART
  MIDI.begin(MIDI_CHANNEL_OMNI);  // Iniciar la comunicación MIDI
}

void loop() {
  char mensajeOSC[255];
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    int len = Udp.read(mensajeOSC, 255);
    if (len > 0) {
      mensajeOSC[len] = '\0'; // Asegurar fin de cadena
    }

    Serial.print("Mensaje OSC recibido: \n");
    Serial.println(mensajeOSC);

    // Procesar mensaje OSC y convertirlo a MIDI
    procesarOSC(mensajeOSC);
  }
}

// Función para procesar los mensajes OSC recibidos
void procesarOSC(char* mensaje) {
  String msg = String(mensaje); // Convertir a String para manejarlo mejor

  // Procesar valores normalizados (0.0 - 1.0)
  if (msg.startsWith("/pot1 ")) {
    float valor1 = msg.substring(6).toFloat();
    Serial.print("Potenciómetro 1 (normalizado): ");
    Serial.println(valor1);
  } 
  else if (msg.startsWith("/pot2 ")) {
    float valor2 = msg.substring(6).toFloat();
    Serial.print("Potenciómetro 2 (normalizado): ");
    Serial.println(valor2);
  } 
  else if (msg.startsWith("/pot3 ")) {
    float valor3 = msg.substring(6).toFloat();
    Serial.print("Potenciómetro 3 (normalizado): ");
    Serial.println(valor3);
  }

  // Procesar valores MIDI (0-127)
  else if (msg.startsWith("/pot1midi ")) {
    int valor1midi = msg.substring(10).toInt();
    Serial.print("Potenciómetro 1 (MIDI 0-127): ");
    Serial.println(valor1midi);
    
    // Enviar mensaje MIDI (Control Change)
    enviarMIDI(1, valor1midi);  // Potenciómetro 1 (canal 1, valor MIDI)
  } 
  else if (msg.startsWith("/pot2midi ")) {
    int valor2midi = msg.substring(10).toInt();
    Serial.print("Potenciómetro 2 (MIDI 0-127): ");
    Serial.println(valor2midi);
    
    // Enviar mensaje MIDI (Control Change)
    enviarMIDI(2, valor2midi);  // Potenciómetro 2 (canal 1, valor MIDI)
  } 
  else if (msg.startsWith("/pot3midi ")) {
    int valor3midi = msg.substring(10).toInt();
    Serial.print("Potenciómetro 3 (MIDI 0-127): ");
    Serial.println(valor3midi);
    
    // Enviar mensaje MIDI (Control Change)
    enviarMIDI(3, valor3midi);  // Potenciómetro 3 (canal 1, valor MIDI)
  }
  else {
    Serial.println("Mensaje OSC no reconocido");
  }
}

// Función para enviar el mensaje MIDI
void enviarMIDI(int control, int valor) {
  MIDI.sendControlChange(control, valor, 1);  // Enviar Control Change en el canal 1

  Serial.print("Enviado mensaje MIDI (CC) canal 1, Control: ");
  Serial.print(control);
  Serial.print(", Valor: ");
  Serial.println(valor);
}
