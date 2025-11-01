/*
  CONTROLADOR_3.1.0_MIDIDIN5 — PRUEBA I2C → MÓDULO MIDI
  -----------------------------------------------------
  Envío por I2C en el formato que espera el esclavo:
    [N][status,d1,d2] x N
  Aquí usamos N=1 por envío.

 
*/

#include <Arduino.h>
#include <Wire.h>

// =================== CONFIGURACIÓN ===================
static const int SDA_OUT = 11;            // AJUSTA a tu placa
static const int SCL_OUT = 12;            // AJUSTA a tu placa
static const uint32_t I2C_FREQ  = 100000; // 100 kHz para robustez (sube a 400 kHz si todo ok)
static const uint8_t  I2C_ADDR  = 0x30;   // Dirección del módulo MIDI (tu esclavo)
static const uint8_t  MIDI_CH   = 1;      // Canal 1..16
static const uint8_t  VEL_ON    = 100;
static const uint8_t  VEL_OFF   = 0;

TwoWire I2C_OUT(1);

// ---- Utilidad: error como texto
const char* i2cErrStr(uint8_t e) {
  switch (e) {
    case 0:  return "OK";
    case 1:  return "data too long";
    case 2:  return "NACK on address";
    case 3:  return "NACK on data";
    case 4:  return "other error";
    case 5:  return "timeout";
    default: return "unknown";
  }
}

// ---- Enviar 1 mensaje MIDI en frame: [1][status][d1][d2]
bool sendMidiPacket(uint8_t status, uint8_t d1, uint8_t d2) {
  I2C_OUT.beginTransmission(I2C_ADDR);
  I2C_OUT.write((uint8_t)1);          // N = 1 mensaje
  I2C_OUT.write(status);
  I2C_OUT.write(d1 & 0x7F);
  I2C_OUT.write(d2 & 0x7F);
  uint8_t err = I2C_OUT.endTransmission(true);
  if (err != 0) {
    Serial.printf("[I2C-OUT] TX err=%u (%s)\n", err, i2cErrStr(err));
    return false;
  }
  return true;
}

// ---- Atajos NoteOn/NoteOff
bool noteOn(uint8_t ch, uint8_t note, uint8_t vel) {
  ch = max<uint8_t>(1, min<uint8_t>(16, ch));
  return sendMidiPacket(0x90 | ((ch - 1) & 0x0F), note, vel);
}
bool noteOff(uint8_t ch, uint8_t note, uint8_t vel) {
  ch = max<uint8_t>(1, min<uint8_t>(16, ch));
  return sendMidiPacket(0x80 | ((ch - 1) & 0x0F), note, vel);
}

// ---- Escaneo rápido del bus OUT
void scanI2COut() {
  Serial.println(F("=== ESCANEO I2C (bus de salida) ==="));
  uint8_t found = 0;
  for (uint8_t addr = 1; addr <= 0x7E; addr++) {
    I2C_OUT.beginTransmission(addr);
    uint8_t err = I2C_OUT.endTransmission(true);
    if (err == 0) {
      Serial.printf("→ Dispositivo encontrado en 0x%02X\n", addr);
      found++;
    }
    delay(2);
  }
  if (!found) Serial.println(F("(no se encontró ningún dispositivo)"));
  Serial.println(F("=== ESCANEO COMPLETADO ==="));
}

// ---- Melodía de prueba
struct Step { uint8_t note; uint16_t ms; };
Step melody[] = {
  {60, 300}, {62, 300}, {64, 300}, {65, 300},
  {67, 300}, {69, 300}, {71, 300}, {72, 600},
};
const size_t melodyLen = sizeof(melody) / sizeof(melody[0]);

void playMelodyOnce() {
  Serial.println(F("Reproduciendo melodía..."));
  for (size_t i = 0; i < melodyLen; ++i) {
    uint8_t n = melody[i].note;
    uint16_t d = melody[i].ms;

    if (!noteOn(MIDI_CH, n, VEL_ON)) {
      delay(3); // reintento suave
      noteOn(MIDI_CH, n, VEL_ON);
    }
    delay(d);
    noteOff(MIDI_CH, n, VEL_OFF);
    delay(50);
  }
  Serial.println(F("Melodía finalizada."));
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println(F("=== CONTROLADOR: I2C → MIDI (formato N + 3N) ==="));

  I2C_OUT.begin(SDA_OUT, SCL_OUT, I2C_FREQ);
  Serial.printf("I2C OUT : SDA=%d, SCL=%d @%lu Hz (destino 0x%02X)\n",
                SDA_OUT, SCL_OUT, (unsigned long)I2C_FREQ, I2C_ADDR);

  scanI2COut();          // debería listar 0x30
  playMelodyOnce();      // una pasada en setup
}

unsigned long lastPlay = 0;
void loop() {
  if (millis() - lastPlay > 5000) {
    lastPlay = millis();
    playMelodyOnce();    // repite cada 5 s
  }
}
