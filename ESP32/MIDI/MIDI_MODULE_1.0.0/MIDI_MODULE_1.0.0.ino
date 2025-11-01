/************************************************************
 * MIDI_MODULE_1.0.0.ino  (ESCLAVO I2C)  —  SOLO DIN-5 AHORA
 * Recibe [N][status,d1,d2]xN por I2C y lo reenvía a DIN-5
 ************************************************************/
#include <Arduino.h>
#include <Wire.h>

static const uint8_t I2C_ADDR_MIDI = 0x30; // Dirección del módulo
static const int   MIDI_TX_PIN   = 17;     // GPIO hacia D1/TX del shield
static const long  MIDI_BAUD     = 31250;

#define MAX_MIDI_MSGS   10
#define PKT_SIZE        (1 + MAX_MIDI_MSGS * 3)

volatile uint8_t rxBuf[PKT_SIZE];
volatile int     rxLen       = 0;
volatile bool    hasNewFrame = false;

uint8_t workBuf[PKT_SIZE];
int     workLen = 0;

static void onI2CReceive(int numBytes) {
  if (numBytes <= 0) return;
  if (numBytes > PKT_SIZE) numBytes = PKT_SIZE;

  for (int i = 0; i < numBytes; i++) {
    rxBuf[i] = Wire.available() ? Wire.read() : 0;
  }
  rxLen       = numBytes;
  hasNewFrame = true;
}

void setup() {
  Serial.begin(115200); delay(200);
  Serial.println("\n[MIDI_MODULE] I2C esclavo -> DIN5");

  Serial1.end();
  Serial1.begin(MIDI_BAUD, SERIAL_8N1, -1, MIDI_TX_PIN);
  Serial.printf("[MIDI_MODULE] DIN UART: TX=%d @ %ld\n", MIDI_TX_PIN, MIDI_BAUD);

  Wire.begin(I2C_ADDR_MIDI);     // esclavo I2C
  Wire.onReceive(onI2CReceive);
  Serial.printf("[MIDI_MODULE] I2C esclavo @ 0x%02X\n", I2C_ADDR_MIDI);
}

void loop() {
  if (hasNewFrame) {
    noInterrupts();
    int len = rxLen; if (len > PKT_SIZE) len = PKT_SIZE;
    memcpy(workBuf, (const void*)rxBuf, len);
    hasNewFrame = false;
    interrupts();

    workLen = len;

    if (workLen >= 1) {
      uint8_t count = workBuf[0];
      if (count > MAX_MIDI_MSGS) count = MAX_MIDI_MSGS;
      int needed = 1 + count*3;

      if (workLen >= needed) {
        for (uint8_t i = 0; i < count; i++) {
          int base   = 1 + i*3;
          uint8_t st = workBuf[base + 0];
          uint8_t d1 = workBuf[base + 1] & 0x7F;
          uint8_t d2 = workBuf[base + 2] & 0x7F;
          Serial1.write(st); Serial1.write(d1); Serial1.write(d2);
        }
      }
    }
  }
}
