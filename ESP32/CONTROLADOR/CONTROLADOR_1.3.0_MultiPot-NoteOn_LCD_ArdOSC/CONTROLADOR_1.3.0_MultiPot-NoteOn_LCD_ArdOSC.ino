#include <WiFi.h>
#include <WiFiClient.h>
#include <OSCMessage.h>  // Librería OSC
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

const char* ssid = "ESP32_AP_OSC";
const char* password = "12345678";

WiFiServer server(8000);  // Servidor TCP en el puerto 8000
WiFiClient client;

LiquidCrystal_PCF8574 lcd(0x27);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("Red Wi-Fi creada. Esperando clientes...");
  server.begin();  // Iniciar el servidor TCP

  lcd.begin(16, 2);
  lcd.setBacklight(255);
}

void loop() {
  if (server.hasClient()) {
    client = server.available();
    while (client.connected()) {
      if (client.available()) {
        OSCMessage msg;
        while (client.available()) {
          msg.fill((uint8_t*)client.read(), client.available());  // Llenar el mensaje con los bytes disponibles
        }
        // Usamos getAddress() para obtener la dirección del mensaje
        if (String(msg.getAddress()) == "/lcd") {  // Verificar si el mensaje tiene la dirección "/lcd"
          char buffer[256];
          msg.getString(0, buffer);  // Obtener el primer argumento como cadena
          String msgStr = String(buffer);
          mostrarLCD(msgStr.c_str());
        }
      }
    }
  }
}

void mostrarLCD(const char* mensaje) {
  String msg = String(mensaje);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(msg.substring(16, 32));
}
