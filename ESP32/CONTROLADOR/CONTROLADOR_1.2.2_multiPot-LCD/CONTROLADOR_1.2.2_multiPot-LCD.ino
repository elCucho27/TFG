#include <WiFi.h>
#include <WiFiUdp.h>
#include <MIDI.h>  // Incluir la librería MIDI para comunicación UART
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h> // Librería LCD I2C

const char* ssid = "ESP32_AP_OSC";  // Nombre de la red Wi-Fi
const char* password = "12345678";  // Contraseña

WiFiUDP Udp;
const int localPort = 8000; // Puerto UDP

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);  // Crear instancia para UART (puerto serial 2)

// Configuración del LCD I2C
LiquidCrystal_PCF8574 lcd(0x27); // Cambia la dirección si es necesario

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

  // Inicializar LCD
  lcd.begin(16, 2); // 16 columnas, 2 filas
  lcd.setBacklight(255); // Encender retroiluminación
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

    // Solo procesar mensajes OSC que lleguen a la dirección "/lcd"
    if (strStartsWith(mensajeOSC, "/lcd")) {
      mostrarLCD(mensajeOSC); // Mostrar el mensaje en el LCD
    }

    // También podrías seguir procesando mensajes MIDI si lo necesitas
    // procesarOSC(mensajeOSC); // Si es necesario, puedes seguir con el procesamiento de MIDI
  }
}

// Función para mostrar el mensaje OSC en el LCD
void mostrarLCD(char* mensaje) {
  String msg = String(mensaje); // Convertir el mensaje OSC a String
  lcd.clear(); // Limpiar la pantalla del LCD

  // Mostrar solo los primeros 16 caracteres en la primera línea
  lcd.setCursor(0, 0);
  lcd.print(msg.substring(0, 16)); // Muestra solo los primeros 16 caracteres

  // Mostrar los siguientes 16 caracteres en la segunda línea
  lcd.setCursor(0, 1);
  lcd.print(msg.substring(16, 32)); // Muestra los siguientes 16 caracteres (si hay más texto)
}

// Función auxiliar para verificar si una cadena empieza con un prefijo dado
bool strStartsWith(char* str, const char* prefix) {
  while (*prefix) {
    if (*str != *prefix) {
      return false;
    }
    str++;
    prefix++;
  }
  return true;
}

