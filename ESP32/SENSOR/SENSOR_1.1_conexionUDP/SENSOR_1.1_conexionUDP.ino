#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración de la red Wi-Fi del controlador (ESP32 AP)
const char* ssid = "ESP32_AP_OSC";  // Nombre de la red Wi-Fi del controlador
const char* password = "12345678";  // Contraseña de la red Wi-Fi

WiFiUDP Udp;
const int remotePort = 8000; // Puerto UDP donde la controladora escucha
IPAddress serverIP(192, 168, 4, 1); // IP del AP (Controladora)

const int potAnalogPin = 33;  // Pin al que está conectado el potenciómetro
int lastMidiValue = -1; // Último valor MIDI enviado (-1 para asegurar el primer envío)

void setup() {
  Serial.begin(115200);
  pinMode(potAnalogPin, INPUT);

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
  int value = analogRead(potAnalogPin); // Leer el valor del potenciómetro (0-4095)
  
  // Mapear el valor a un rango de 0 a 127 (rango MIDI)
  int midiValue = map(value, 0, 4095, 0, 127);

  // Solo enviar si el valor cambia para evitar redundancias
  if (midiValue != lastMidiValue) {
    Serial.print("Potenciómetro: ");
    Serial.print(value);
    Serial.print(" -> MIDI Volume: ");
    Serial.println(midiValue);

    // Crear mensaje OSC en formato: "/pot 0.75" (valor normalizado entre 0.0 y 1.0)
    String mensajeOSC = "/pot " + String(midiValue / 127.0); // Normalizado entre 0.0 y 1.0

    // Enviar el mensaje OSC al controlador
    Udp.beginPacket(serverIP, remotePort);
    Udp.print(mensajeOSC); // Usamos print() para enviar la cadena de texto
    Udp.endPacket();

    lastMidiValue = midiValue; // Actualizar el último valor MIDI enviado
  }

  delay(20); // Pequeña pausa para estabilizar la lectura
}
