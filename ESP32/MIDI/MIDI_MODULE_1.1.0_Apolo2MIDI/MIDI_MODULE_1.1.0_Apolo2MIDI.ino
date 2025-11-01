/*******************************************************
 * MIDI_MODULE_1.2_DIN5.ino
 * -----------------------------------------------------
 * Módulo MIDI I2C → DIN-5 (UART)
 * 
 * Recibe paquetes por I2C desde el controlador maestro con
 * formato:
 *   [N][status1][data1_1][data2_1]...[statusN][data1_N][data2_N]
 *
 * Envía cada mensaje por UART a 31250 baudios (DIN-5 estándar).
 * 
 * No usa USB MIDI (TinyUSB ni Adafruit_USBD_MIDI).
 * Incluye salida Serial (USB) de depuración.
 *******************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <MIDI.h>

// =============== CONFIGURACIÓN =========================

// Dirección I2C del módulo MIDI (debe coincidir con el controlador)
#define MIDI_ADDR 0x30

// Pines I2C
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 100000  // 100 kHz

// UART para MIDI DIN-5
#define MIDI_TX_PIN 17   // Cambia según tu hardware
#define MIDI_BAUD 31250  // Velocidad estándar MIDI DIN

// Modo debug
#define DEBUG 1
#define DEBUG_BAUD 115200

// =============== VARIABLES GLOBALES =====================
static const uint8_t MAX_BUFFER = 64; // Tamaño máximo esperado de paquete
uint8_t i2cBuffer[MAX_BUFFER];

// Instancia de puerto serie MIDI
HardwareSerial midiSerial(1);  // UART1: GPIO17 (TX), sin RX

// =============== CALLBACK I2C ===========================

void onReceiveHandler(int bytesReceived) {
  if (bytesReceived <= 0) return;
  if (bytesReceived > MAX_BUFFER) bytesReceived = MAX_BUFFER;

  // Leer bytes del maestro
  int idx = 0;
  while (Wire.available() && idx < bytesReceived) {
    i2cBuffer[idx++] = Wire.read();
  }

  if (idx < 4) return; // mínimo: N + 1 mensaje (3 bytes)

  uint8_t numMsgs = i2cBuffer[0];
  if (numMsgs == 0) return;

#if DEBUG
  Serial.printf("\n[I2C] Recibido paquete de %u mensajes (%u bytes)\n", numMsgs, idx);
#endif

  // Procesar los mensajes
  for (uint8_t i = 0; i < numMsgs; ++i) {
    int base = 1 + i * 3;
    if (base + 2 >= idx) break; // seguridad

    uint8_t status = i2cBuffer[base + 0];
    uint8_t data1  = i2cBuffer[base + 1];
    uint8_t data2  = i2cBuffer[base + 2];

#if DEBUG
    Serial.printf("[MIDI OUT] %02X %02X %02X\n", status, data1, data2);
#endif

    // Enviar por UART (DIN-5)
    midiSerial.write(status);
    midiSerial.write(data1);
    midiSerial.write(data2);
  }

#if DEBUG
  Serial.println("[MIDI] Paquete transmitido por DIN5.\n");
#endif
}

// =============== SETUP =================================

void setup() {
#if DEBUG
  Serial.begin(DEBUG_BAUD);
  delay(100);
  Serial.println("\n[MIDI MODULE DIN5] Inicializando...");
#endif

  // --- Configura UART para salida MIDI física ---
  midiSerial.begin(MIDI_BAUD, SERIAL_8N1, -1, MIDI_TX_PIN);  // solo TX
#if DEBUG
  Serial.printf("UART MIDI inicializado (TX=%d, %lu baudios)\n",
                MIDI_TX_PIN, (unsigned long)MIDI_BAUD);
#endif

  // --- Configura I2C como esclavo ---
  Wire.begin(MIDI_ADDR, I2C_SDA, I2C_SCL, I2C_FREQ);
  Wire.onReceive(onReceiveHandler);

#if DEBUG
  Serial.printf("I2C esclavo @0x%02X (SDA=%d, SCL=%d)\n",
                MIDI_ADDR, I2C_SDA, I2C_SCL);
  Serial.println("Esperando mensajes del controlador...");
#endif
}

// =============== LOOP ==================================

void loop() {
  // No hay tareas en loop; todo ocurre en interrupción I2C
  delay(10);
}
