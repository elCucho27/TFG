#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "StringHandler.h"

// === CONFIGURACIÓN GENERAL ===
#define SLAVE_ADDR 0x08
#define SDA_PIN 8
#define SCL_PIN 9
#define NUM_SENSORS 9
#define NUM_STRINGS 3
#define PACKET_SIZE (NUM_SENSORS * 2 + 1)
#define REQUEST_INTERVAL_MS 10

// === MIDI USB vía Adafruit TinyUSB + MIDI.h ===
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// === GESTIÓN DE CUERDAS ===
StringHandler strings[NUM_STRINGS] = {
  StringHandler(0),
  StringHandler(1),
  StringHandler(2)
};

// === I2C y temporización ===
uint8_t buffer[PACKET_SIZE];
uint16_t sensorValues[NUM_SENSORS];
uint32_t lastRequestTime = 0;
uint16_t lastPrintedValues[NUM_SENSORS] = {0}; 
const uint16_t DEBUG_CHANGE_THRESHOLD = 300;


// === ESTADO DE COMUNICACIÓN CON EL SENSOR ===
enum class SensorCommState {
  Waiting,
  Connected,
  NoResponse,
  PacketSizeError,
  InvalidHeader
};

SensorCommState lastState = SensorCommState::Waiting;

void setup() {
  Serial.begin(115200);   // Debug vía CDC (USB)
  usb_midi.begin();       // Inicializa interfaz MIDI USB
  MIDI.begin();           // Inicializa capa MIDI.h sobre usb_midi
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("[MASTER] MIDI USB + I2C inicializado correctamente");

  // Configuración por cuerda
  strings[0].setConfig(1, 48, 20, true, true, 2);  // Cuerda 1 → canal 1
  strings[1].setConfig(2, 60, 20, true, true, 2);  // Cuerda 2 → canal 2
  strings[2].setConfig(3, 72, 20, true, true, 2);  // Cuerda 3 → canal 3
}

void loop() {
  uint32_t now = millis();
  if (now - lastRequestTime < REQUEST_INTERVAL_MS) return;
  lastRequestTime = now;

  SensorCommState currentState;

  // === Paso 1: Verificar conexión I2C ===
  Wire.beginTransmission(SLAVE_ADDR);
  uint8_t error = Wire.endTransmission();

  if (error != 0) {
    currentState = SensorCommState::NoResponse;
  } else {
    Wire.requestFrom(SLAVE_ADDR, PACKET_SIZE);
    size_t bytesRead = Wire.available();

    if (bytesRead != PACKET_SIZE) {
      while (Wire.available()) Wire.read();
      currentState = SensorCommState::PacketSizeError;
    } else {
      for (size_t i = 0; i < PACKET_SIZE; ++i)
        buffer[i] = Wire.read();

      if (buffer[0] != 0xAA) {
        currentState = SensorCommState::InvalidHeader;
      } else {
        currentState = SensorCommState::Connected;

        // === Paso 2: Desempaquetar valores ===
        for (size_t i = 0; i < NUM_SENSORS; ++i) {
          size_t index = 1 + i * 2;
          sensorValues[i] = (buffer[index] << 8) | buffer[index + 1];
        }

      // === Paso 3: Procesar por cuerda ===
        for (int i = 0; i < NUM_STRINGS; ++i) {
          uint16_t softpot = sensorValues[i];
          uint16_t fsr     = sensorValues[i + 3];
          uint16_t dotfsr  = sensorValues[i + 6];

          strings[i].update(softpot, fsr, dotfsr, now);

          uint8_t ch = i + 1;

          // === MIDI: envío de eventos ===
         if (strings[i].hasNoteOnEvent()) {
            MIDI.sendNoteOn(strings[i].getNote(), strings[i].getVelocity(), ch);
            MIDI.sendPitchBend(strings[i].getPitchBend() - 8192, ch);  // Ajuste: convierte 0–16383 a -8192–+8191
          }

          if (strings[i].hasNoteOffEvent()) {
            MIDI.sendNoteOff(strings[i].getNote(), 0, ch);
            MIDI.sendPitchBend(0, ch);  // Centro de pitch bend en formato -8192–+8191
          }

          // Envio continuo de pitch bend si la nota sigue activa
          if (!strings[i].hasNoteOffEvent() && !strings[i].hasNoteOnEvent()) {
            int16_t bend = strings[i].getPitchBend();
            if (bend != 8192) {
              bend -= 8192;  // Convertimos a rango firmado
              MIDI.sendPitchBend(bend, ch);
            }
          }

          if (strings[i].hasModulationEvent()) {
            MIDI.sendControlChange(1, strings[i].getModulation(), ch);  // CC1: Modulación
          }
        }
      }
    }
  }

  // === Estado de conexión ===
  if (currentState != lastState) {
    switch (currentState) {
      case SensorCommState::Waiting:
       break;
      case SensorCommState::NoResponse: 
      Serial.println("[ALERTA] Sensor no responde.");
       break;
      case SensorCommState::PacketSizeError: 
      Serial.println("[ERROR] Paquete I2C incompleto.");
       break;
      case SensorCommState::InvalidHeader: 
      Serial.println("[ERROR] Cabecera inválida.");
       break;
      case SensorCommState::Connected:
        Serial.println(lastState == SensorCommState::Waiting
                    ? "[INFO] Sensor conectado por primera vez."
                       : "[INFO] Sensor reconectado.");
        break;
    }
    lastState = currentState;
  }

  // === Debug visual por consola ===
bool hasChanged = false;
for (int i = 0; i < NUM_SENSORS; ++i) {
  if (abs((int)sensorValues[i] - (int)lastPrintedValues[i]) > DEBUG_CHANGE_THRESHOLD) {
    hasChanged = true;
    break;
  }
}


if (currentState == SensorCommState::Connected && hasChanged) {
  printSensorData();
  memcpy(lastPrintedValues, sensorValues, sizeof(sensorValues));  // Actualiza los valores impresos
}

}

// === FUNCION DEBUG: muestra tabla con datos y eventos ===
void printSensorData() {
  Serial.println(F("====== SENSOR DATA ======"));
  Serial.println(F("| # | SoftPot |   FSR   | DotFSR | Note | Vel | Bend  | Mod |"));
  Serial.println(F("|---|---------|---------|--------|------|-----|--------|-----|"));

  for (int i = 0; i < NUM_STRINGS; ++i) {
    uint16_t softpot = sensorValues[i];
    uint16_t fsr     = sensorValues[i + 3];
    uint16_t dotfsr  = sensorValues[i + 6];

    uint8_t note     = strings[i].getNote();
    uint8_t velocity = strings[i].getVelocity();
    int16_t bend     = strings[i].getPitchBend();
    uint8_t mod      = strings[i].getModulation();

    const char* onEvent  = strings[i].hasNoteOnEvent()       ? "ON " : "   ";
    const char* offEvent = strings[i].hasNoteOffEvent()      ? "OFF" : "   ";
    const char* modEvent = strings[i].hasModulationEvent()   ? "*"   : " ";

    Serial.printf("| %d |  %5d  |  %5d  |  %5d | %s%s | %3d | %6d | %3d%s |\n",
                  i + 1, softpot, fsr, dotfsr,
                  onEvent, offEvent,
                  velocity, bend, mod, modEvent);
  }

  Serial.println(F("================================================================"));
}

