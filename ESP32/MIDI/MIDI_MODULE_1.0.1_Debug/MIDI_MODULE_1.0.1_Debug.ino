/************************************************************
 * MIDI_MODULE_1.0.1.ino  (ESCLAVO I2C + DEPURACIÓN SERIAL)
 * ---------------------------------------------------------
 * Recibe [N][status,d1,d2]xN por I2C y lo reenvía a DIN-5.
 * Muestra cada mensaje recibido y enviado por Serial (USB).
 ************************************************************/
#include <Arduino.h>
#include <Wire.h>

static const uint8_t I2C_ADDR_MIDI = 0x30; // Dirección del módulo
static const int   MIDI_TX_PIN   = 17;     // GPIO hacia TX del shield DIN-5
static const long  MIDI_BAUD     = 31250;  // Velocidad estándar MIDI

#define MAX_MIDI_MSGS   10
#define PKT_SIZE        (1 + MAX_MIDI_MSGS * 3)

// --- Buffers compartidos entre ISR y loop
volatile uint8_t rxBuf[PKT_SIZE];
volatile int     rxLen       = 0;
volatile bool    hasNewFrame = false;

uint8_t workBuf[PKT_SIZE];
int     workLen = 0;

// ===========================================================
//            CALLBACK I2C (se ejecuta en interrupción)
// ===========================================================
static void onI2CReceive(int numBytes) {
  if (numBytes <= 0) return;
  if (numBytes > PKT_SIZE) numBytes = PKT_SIZE;

  for (int i = 0; i < numBytes; i++) {
    rxBuf[i] = Wire.available() ? Wire.read() : 0;
  }
  rxLen       = numBytes;
  hasNewFrame = true;
}

// ===========================================================
//                           SETUP
// ===========================================================
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[MIDI_MODULE] I2C esclavo -> DIN-5 con depuración");

  // Configurar UART MIDI
  Serial1.end();
  Serial1.begin(MIDI_BAUD, SERIAL_8N1, -1, MIDI_TX_PIN);
  Serial.printf("[MIDI_MODULE] DIN UART: TX=%d @ %ld\n", MIDI_TX_PIN, MIDI_BAUD);

  // Inicializar I²C como esclavo
  Wire.begin(I2C_ADDR_MIDI);
  Wire.onReceive(onI2CReceive);
  Serial.printf("[MIDI_MODULE] I2C esclavo @ 0x%02X listo\n", I2C_ADDR_MIDI);
}

// ===========================================================
//                            LOOP
// ===========================================================
void loop() {
  if (hasNewFrame) {
    // Copiamos el buffer recibido de forma segura
    noInterrupts();
    int len = rxLen;
    if (len > PKT_SIZE) len = PKT_SIZE;
    memcpy(workBuf, (const void*)rxBuf, len);
    hasNewFrame = false;
    interrupts();

    workLen = len;

    // --- Mostrar información general ---
    Serial.printf("\n[I2C] Frame recibido (%d bytes): ", workLen);
    for (int i = 0; i < workLen; i++) {
      Serial.printf("%02X ", workBuf[i]);
    }
    Serial.println();

    if (workLen >= 1) {
      uint8_t count = workBuf[0];
      if (count > MAX_MIDI_MSGS) count = MAX_MIDI_MSGS;
      int needed = 1 + count * 3;

      if (workLen >= needed) {
        Serial.printf("[I2C] N=%d mensajes válidos\n", count);

        for (uint8_t i = 0; i < count; i++) {
          int base = 1 + i * 3;
          uint8_t st = workBuf[base + 0];
          uint8_t d1 = workBuf[base + 1] & 0x7F;
          uint8_t d2 = workBuf[base + 2] & 0x7F;

          // --- Mostrar mensaje MIDI legible ---
          uint8_t type = st & 0xF0;
          uint8_t ch   = (st & 0x0F) + 1;
          const char* typeStr = "?";
          if (type == 0x90 && d2 > 0) typeStr = "NoteOn";
          else if (type == 0x90 && d2 == 0) typeStr = "NoteOff";
          else if (type == 0x80) typeStr = "NoteOff";
          else if (type == 0xB0) typeStr = "CC";
          else if (type == 0xE0) typeStr = "PitchBend";

          Serial.printf("  → [%02X %02X %02X]  %-8s  Ch=%d  D1=%d  D2=%d\n",
                        st, d1, d2, typeStr, ch, d1, d2);

          // --- Enviar al puerto DIN-5 ---
          Serial1.write(st);
          Serial1.write(d1);
          Serial1.write(d2);
        }
      } else {
        Serial.printf("[I2C] Frame incompleto (%d/%d bytes)\n", workLen, needed);
      }
    }
  }
}
