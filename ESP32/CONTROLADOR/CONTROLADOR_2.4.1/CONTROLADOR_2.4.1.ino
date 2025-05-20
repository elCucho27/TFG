// ======================================================================= CONTROLADOR 2.4.1 ====================================


// ========================== LIBRERÍAS ==========================
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include "StringHandler.h"
#include "OSCHandler.h"

// ======================= DEPURACIÓN OPCIONAL ===================
#define DEBUG
#define DEBUG_PRINT_INTERVAL_MS 250
uint32_t lastDebugPrintTime = 0;
uint16_t lastSensorValues[9] = {0};

// =================== DEFINICIÓN DE DATOS I2C ===================
#define MAX_SOFTPOTS 3
#define MAX_FSRS     3
#define MAX_DOTFSRS  3

struct SensorI2CData {
  uint16_t softpots[MAX_SOFTPOTS];
  uint16_t fsrs[MAX_FSRS];
  uint16_t dotfsrs[MAX_DOTFSRS];
};

// ==================== ESTADO DE CONEXIÓN SENSOR =================
enum class SensorCommState { Waiting, NoResponse, PacketSizeError, InvalidHeader, Connected };
SensorCommState currentState = SensorCommState::Waiting;
SensorCommState lastState    = SensorCommState::Waiting;

// ======================= CONFIGURACIÓN RED =====================
const char* ssid = "TP-Link_3506";
const char* password = "30517493";
IPAddress remoteIP(192, 168, 1, 255);
const uint16_t remotePort = 8000;

// ======================= CONFIGURACIÓN I2C =====================
#define SDA_PIN 8
#define SCL_PIN 9
#define I2C_ADDRESS_SENSOR 0x08
#define I2C_INTERVAL_MS 10

// ===================== CONFIGURACIÓN GENERAL ===================
#define NUM_STRINGS 3

// ======================= MIDI USB ==============================
Adafruit_USBD_MIDI usbMIDI;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMIDI, MIDI);

// =================== OBJETOS Y VARIABLES =======================
TwoWire I2CBus = Wire;
OSCHandler osc(remoteIP, remotePort);
SensorI2CData sensorData;

StringHandler strings[NUM_STRINGS] = {
  StringHandler(0),
  StringHandler(1),
  StringHandler(2)
};

uint32_t lastI2CRequest = 0;

// ============================ SETUP ========================================================================================
void setup() {
  Serial.begin(115200);

// ============================ WIFI ================================
WiFi.begin(ssid, password);
Serial.print("Conectando a WiFi:");
Serial.print(ssid);

unsigned long startAttemptTime = millis();
const unsigned long timeout = 5000;  // Esperamos hasta 5 segundos

while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
  delay(500);
  Serial.print(".");
}

if (WiFi.status() == WL_CONNECTED) {
  Serial.println("\nWiFi conectado");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
} else {
  Serial.println("\n[WARNING - SETUP] No se pudo conectar a WiFi. Continuando sin OSC.");
}


  I2CBus.begin(SDA_PIN, SCL_PIN);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  osc.begin();

  for (uint8_t i = 0; i < NUM_STRINGS; i++) {
    strings[i].setConfig(i + 1, 48, 20, true, true, 2);
  }
}

// ============================ LOOP =========================================================================================
void loop() {
  uint32_t now = millis();

  if (now - lastI2CRequest >= I2C_INTERVAL_MS) {
    lastI2CRequest = now;

    I2CBus.beginTransmission(I2C_ADDRESS_SENSOR);
    uint8_t error = I2CBus.endTransmission();


    if (error != 0) {
      currentState = SensorCommState::NoResponse;
    } else {
      int packetSize = sizeof(SensorI2CData);
      int bytesRead = I2CBus.requestFrom(I2C_ADDRESS_SENSOR, packetSize);

      if (bytesRead != packetSize) {
        while (I2CBus.available()) I2CBus.read();
        currentState = SensorCommState::PacketSizeError;
      } else {
        I2CBus.readBytes((uint8_t*)&sensorData, packetSize);

        // Validar cabecera: primer byte debe ser 0xAA
        uint8_t* dataPtr = (uint8_t*)&sensorData;
        if (dataPtr[0] != 0xAA) {
          currentState = SensorCommState::InvalidHeader;
        } else {
          currentState = SensorCommState::Connected;

// ======================================= DEBUG - detección de cambio significativo ===========
          #ifdef DEBUG
          bool significantChange = false;
          uint16_t sensorValues[9] = {
            sensorData.softpots[0], sensorData.softpots[1], sensorData.softpots[2],
            sensorData.fsrs[0],     sensorData.fsrs[1],     sensorData.fsrs[2],
            sensorData.dotfsrs[0],  sensorData.dotfsrs[1],  sensorData.dotfsrs[2]
          };
          for (uint8_t j = 0; j < 9; j++) {
            if (abs((int)sensorValues[j] - (int)lastSensorValues[j]) > 3) {
              significantChange = true;
              break;
            }
          }
          if (significantChange && (now - lastDebugPrintTime >= DEBUG_PRINT_INTERVAL_MS)) {
            printSensorData(sensorValues);
            memcpy(lastSensorValues, sensorValues, sizeof(lastSensorValues));
            lastDebugPrintTime = now;
          }
          #endif

// ===================================== procesamiento musical por cuerda ===
          for (uint8_t i = 0; i < NUM_STRINGS; i++) {
            uint16_t softpot = sensorData.softpots[i];
            uint16_t fsr     = sensorData.fsrs[i];
            uint16_t dotfsr  = sensorData.dotfsrs[i];

            strings[i].update(softpot, fsr, dotfsr, now);

            osc.sendRawPosition(i, softpot);
            osc.sendRawModulation(i, fsr);
            osc.sendRawVelocity(i, dotfsr);

            if (strings[i].hasNoteOnEvent()) {
              uint8_t note = strings[i].getNote();
              uint8_t velocity = strings[i].getVelocity();
              MIDI.sendNoteOn(note, velocity, i + 1);
              osc.sendNoteOn(i, note, velocity);
            }

            if (strings[i].hasNoteOffEvent()) {
              uint8_t note = strings[i].getNote();
              MIDI.sendNoteOff(note, 0, i + 1);
              osc.sendNoteOff(i, note);
            }

            if (strings[i].hasModulationEvent()) {
              uint8_t mod = strings[i].getModulation();
              MIDI.sendControlChange(1, mod, i + 1);
              osc.sendModulation(i, mod);
            }

            int16_t bend = strings[i].getPitchBend();
            if (bend != 8192) {
              MIDI.sendPitchBend(bend - 8192, i + 1);
              osc.sendPitchBend(i, bend);
            }
          }
        }
      }
    }

    // === Paso 3: mostrar estado de conexión ===
    #ifdef DEBUG
    if (currentState != lastState) {
      switch (currentState) {
        case SensorCommState::Waiting: break;
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
    #endif
  }
}

// ====================== FUNCIONES DEBUG ========================
#ifdef DEBUG
void printSensorData(uint16_t* sensorValues) {
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
#endif
