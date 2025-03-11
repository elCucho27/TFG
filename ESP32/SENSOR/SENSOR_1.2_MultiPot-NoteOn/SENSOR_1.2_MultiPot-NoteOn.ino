#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración de la red Wi-Fi del controlador (Access Point)
const char* ssid = "ESP32_AP_OSC";  // Nombre de la red Wi-Fi del controlador
const char* password = "12345678";  // Contraseña de la red Wi-Fi

WiFiUDP Udp;
const int remotePort = 8000; // Puerto UDP donde la controladora escucha
IPAddress serverIP(192, 168, 4, 1); // IP del AP (Controladora)

// Pines sensores (Potenciómetros)
const int AnalogPot1 = 33;  
const int AnalogPot2 = 32;
const int AnalogPot3 = 34;

// Últimos valores MIDI enviados (-1 para asegurar el primer envío)
int lastMidiValue1 = -1;
int lastMidiValue2 = -1;
int lastMidiValue3 = -1;                        

// Variables para los valores de los potenciómetros
int pot1value = 0;
int pot2value = 0;
int pot3value = 0;

void setup() {
  Serial.begin(115200);

  // Configuración de los pines de los potenciómetros
  pinMode(AnalogPot1, INPUT);
  pinMode(AnalogPot2, INPUT);
  pinMode(AnalogPot3, INPUT);

  // Conectar a la red Wi-Fi creada por la controladora (AP)
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red del controlador (AP)");

  // Iniciar UDP
  Udp.begin(8000); // Usamos el mismo puerto para enviar y recibir
}

void loop() {
  // Leer los valores de los potenciómetros
  potRead();

  // Enviar los valores si han cambiado
  sendOSC();
  
  delay(20); // Pequeña pausa para estabilizar la lectura
}

// Función para leer los valores de los potenciómetros y mapearlos a MIDI
void potRead() {
  pot1value = analogRead(AnalogPot1);  
  pot2value = analogRead(AnalogPot2);
  pot3value = analogRead(AnalogPot3);

  // Mapear los valores leídos a un rango de 0-127 (rango MIDI)
  pot1value = map(pot1value, 0, 4095, 0, 127);
  pot2value = map(pot2value, 0, 4095, 0, 127);
  pot3value = map(pot3value, 0, 4095, 0, 127);
}

// Función para enviar los mensajes OSC si los valores han cambiado
void sendOSC() {
  if (pot1value != lastMidiValue1) {
    // Enviar valor normalizado (0.0 - 1.0)
    String mensajeOSC1 = "/pot1 " + String(pot1value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC1);
    Udp.endPacket();

    // Enviar valor en rango MIDI (0-127)
    String mensajeOSC1midi = "/pot1midi " + String(pot1value);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC1midi);
    Udp.endPacket();

    lastMidiValue1 = pot1value;
  }

  if (pot2value != lastMidiValue2) {
    String mensajeOSC2 = "/pot2 " + String(pot2value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC2);
    Udp.endPacket();

    String mensajeOSC2midi = "/pot2midi " + String(pot2value);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC2midi);
    Udp.endPacket();

    lastMidiValue2 = pot2value;
  }

  if (pot3value != lastMidiValue3) {
    String mensajeOSC3 = "/pot3 " + String(pot3value / 127.0);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC3);
    Udp.endPacket();

    String mensajeOSC3midi = "/pot3midi " + String(pot3value);
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC3midi);
    Udp.endPacket();

    lastMidiValue3 = pot3value;
  }
}

