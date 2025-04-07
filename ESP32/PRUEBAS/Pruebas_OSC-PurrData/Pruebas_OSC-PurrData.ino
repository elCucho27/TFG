#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const char* ssid = "TP-Link_3506";
const char* password = "30517493";
const char* host = "192.168.1.48"; // IP de tu PC
const int port = 8000; // Puerto en el que Purr Data está escuchando

WiFiUDP Udp;

const int potPinPitch = 33;
const int potPinVolume = 32;
const int potPinModulation = 34;
const int buttonPin = 26;  // Pin del pulsador

const int fsrPin = 35;
const int fsrTrigger = 1000; //trigger para noteOn

int fsrSamples[10]; //buffer para promediado fsr
int sampleIndex = 0;
bool fullBuff = false;

bool buttonState = false;
bool lastButtonState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // Tiempo de debounce en milisegundos

bool noteOnSent = false;

int lastPitch = -1;
float lastVolume = 0.001;
int lastModulation = -1;

float filteredPitch = 0;  // Almacena el valor filtrado
const float alpha = 0.1;  // Factor de suavizado (ajústalo entre 0.05 y 0.2)

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Conectando a Wi-Fi...\n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi\n");

  pinMode(potPinPitch, INPUT);
  pinMode(potPinVolume, INPUT);
  pinMode(potPinModulation, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Activar resistencia interna
}

void sendOSCMessage(const char* address, float value) {
  OSCMessage msg(address);
  msg.add(value);
  Udp.beginPacket(host, port);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  Serial.print("Enviando OSC: ");
  Serial.print(address);
  Serial.print(" -> ");
  Serial.println(value);
}

void loop() {
  int pitchValue = analogRead(potPinPitch);
  int volumeValue = analogRead(potPinVolume);
  int modulationValue = analogRead(potPinModulation);

// Aplicar filtro exponencial
  filteredPitch = (filteredPitch * (1 - alpha)) + (pitchValue * alpha);
  
  int pitch = map(filteredPitch, 0, 4095, 220, 880);
  float volume = map(volumeValue, 0, 4095, 0, 1000);
  int modulation = map(modulationValue, 0, 4095, 0, 27);

// Solo enviar mensajes si hay cambios
  if (pitch != lastPitch) {
    sendOSCMessage("/LAVANA/pitch", pitch);
    lastPitch = pitch;
  }

  if (volume != lastVolume) {

    sendOSCMessage("/LAVANA/volume", volume/1000);
    lastVolume = volume;
  }

  if (modulation != lastModulation) {
    sendOSCMessage("/LAVANA/modulation", modulation);
    lastModulation = modulation;
  }

// Lectura del botón con debounce
  bool reading = digitalRead(buttonPin) == LOW;  // Invertido por PULLUP

  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // Reiniciar el temporizador de debounce
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState) {
        Serial.println("BOTÓN PRESIONADO -> Enviando Note ON");
        sendOSCMessage("/LAVANA/noteON", 1);
      } else {
        Serial.println("BOTÓN LIBERADO -> Enviando Note OFF");
        sendOSCMessage("/LAVANA/noteOFF", 1);
      }
    }
  }
  lastButtonState = reading;

// Lectura del FSR 
  int fsrValue = analogRead(fsrPin);
  float fsrMapped = map(fsrValue, 0, 4095, 0, 1000) / 1000.0;  // Mapeo de 0 a 1

// Hacer el promedio de las últimas 10 muestras
  fsrSamples[sampleIndex] = fsrMapped * 1000;  // Guardar la muestra en el buffer
  sampleIndex = (sampleIndex + 1) % 10;  // Actualizar el índice (circular)
  if (sampleIndex == 0) {
    fullBuff = true;  // Indicar que el buffer de muestras se llenó
  }

// Promediar las 10 muestras
  float fsrAverage = 0;
  if (fullBuff) {
    for (int i = 0; i < 10; i++) {
      fsrAverage += fsrSamples[i];
    }
    fsrAverage /= 10;
  }

// Imprimir el valor promedio en el monitor serial
  Serial.print("Valor FSR promedio: ");
  Serial.println(fsrAverage / 1000.0, 3);  // Imprimir el promedio mapeado entre 0 y 1, con 3 decimales

// Comprobar si el valor del FSR supera el umbral para enviar Note ON
  if (fsrAverage > fsrTrigger) {  // Solo enviar Note ON si no se ha enviado antes
    sendOSCMessage("/LAVANA/noteON", 1);  // Enviar Note ON
    noteOnSent = true;  // Marcar que se ha enviado Note ON
    Serial.println("Note ON enviado");
  } 

// Enviar Note OFF solo cuando el FSR esté por debajo del umbral y se haya enviado un Note ON previamente
  if (fsrAverage = 0 && noteOnSent) {  // Solo enviar Note OFF si se ha enviado un Note ON antes
    sendOSCMessage("/LAVANA/noteOFF", 0);  // Enviar Note OFF
    noteOnSent = false;  // Marcar que se ha enviado Note OFF
    Serial.println("Note OFF enviado");
  }


  delay(30);
}
