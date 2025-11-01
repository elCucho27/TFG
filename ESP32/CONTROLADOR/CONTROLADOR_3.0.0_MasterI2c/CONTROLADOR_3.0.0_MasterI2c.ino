// ====================== CONTROLADOR 2.4.1 (sin MIDI/OSC, con debug modular) ======================

// ========================== LIBRERÍAS ==========================
#include <Wire.h>
#include "StringHandler.h"

// ======================= CONFIGURACIÓN DE DEPURACIÓN =======================
#define DEBUG
#define DEBUG_TABLE       // Activa la tabla tipo sensor
//#define DEBUG_RAW       // Activa la salida hexadecimal del paquete I2C
#define DEBUG_STATE       // Activa los mensajes de estado de conexión

#define DEBUG_PRINT_INTERVAL_MS 300
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

// ======================= CONFIGURACIÓN I2C =====================
#define SDA_PIN 8
#define SCL_PIN 9
#define I2C_ADDRESS_SENSOR 0x0D
#define I2C_INTERVAL_MS 10

// ===================== CONFIGURACIÓN GENERAL ===================
#define NUM_STRINGS 3

// =================== OBJETOS Y VARIABLES =======================
TwoWire I2CBus = Wire;
SensorI2CData sensorData;

StringHandler strings[NUM_STRINGS] = {
  StringHandler(0),
  StringHandler(1),
  StringHandler(2)
};

uint32_t lastI2CRequest = 0;

// ============================ SETUP ============================
void setup() {
  Serial.begin(115200);
  I2CBus.begin(SDA_PIN, SCL_PIN);

  for (uint8_t i = 0; i < NUM_STRINGS; i++) {
    strings[i].setConfig(i + 1, 48, 20, true, true, 2);
  }

  Serial.println("[CONTROLADOR] Iniciado sin módulos MIDI/OSC.");
}

// ============================ LOOP =============================
void loop() {
  uint32_t now = millis();

  if (now - lastI2CRequest >= I2C_INTERVAL_MS) {
    lastI2CRequest = now;

    I2CBus.beginTransmission(I2C_ADDRESS_SENSOR);
    uint8_t error = I2CBus.endTransmission();

    if (error != 0) {
      currentState = SensorCommState::NoResponse;
    } else {
      const int packetSize = sizeof(SensorI2CData) + 1;
      uint8_t rawBuffer[packetSize];
      int bytesRead = I2CBus.requestFrom(I2C_ADDRESS_SENSOR, packetSize);

      if (bytesRead != packetSize) {
        while (I2CBus.available()) I2CBus.read();
        currentState = SensorCommState::PacketSizeError;
      } else {
        I2CBus.readBytes(rawBuffer, packetSize);

        if (rawBuffer[0] != 0xAA) {
          currentState = SensorCommState::InvalidHeader;
        } else {
          memcpy(&sensorData, &rawBuffer[1], sizeof(SensorI2CData));
          currentState = SensorCommState::Connected;

          // Actualización de lógica de cuerdas
          for (uint8_t i = 0; i < NUM_STRINGS; i++) {
            uint16_t softpot = sensorData.softpots[i];
            uint16_t fsr     = sensorData.fsrs[i];
            uint16_t dotfsr  = sensorData.dotfsrs[i];
            strings[i].update(softpot, fsr, dotfsr, now);
          }

#ifdef DEBUG
          uint16_t sensorValues[9] = {
            sensorData.softpots[0], sensorData.softpots[1], sensorData.softpots[2],
            sensorData.fsrs[0],     sensorData.fsrs[1],     sensorData.fsrs[2],
            sensorData.dotfsrs[0],  sensorData.dotfsrs[1],  sensorData.dotfsrs[2]
          };

          bool significantChange = false;
          for (uint8_t j = 0; j < 9; j++) {
            if (abs((int)sensorValues[j] - (int)lastSensorValues[j]) > 3) {
              significantChange = true;
              break;
            }
          }

          bool printNow = significantChange && (now - lastDebugPrintTime >= DEBUG_PRINT_INTERVAL_MS);

          if (printNow) {
            memcpy(lastSensorValues, sensorValues, sizeof(lastSensorValues));
            lastDebugPrintTime = now;

  #ifdef DEBUG_TABLE
            Serial.println(F("====== SENSOR DEBUG ======"));
            Serial.println(F("| # | SoftPot |   FSR   | DotFSR |"));
            Serial.println(F("|---|---------|---------|--------|"));
            for (int i = 0; i < 3; ++i) {
              Serial.printf("| %d |  %5d  |  %5d  |  %5d |\n",
                            i + 1,
                            sensorValues[i],
                            sensorValues[i + 3],
                            sensorValues[i + 6]);
            }
            Serial.println(F("=========================="));
  #endif

  #ifdef DEBUG_RAW
            Serial.print("[DEBUG] I2C Packet Raw: ");
            for (int i = 0; i < packetSize; i++) {
              Serial.printf("%02X ", rawBuffer[i]);
            }
            Serial.println();
  #endif
          }
#endif
        }
      }
    }

#ifdef DEBUG_STATE
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
