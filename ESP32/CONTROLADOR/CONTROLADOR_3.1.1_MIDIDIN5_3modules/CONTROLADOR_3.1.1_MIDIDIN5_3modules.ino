/*******************************************************
 * CONTROLADOR_3.4.0_Apolo3_to_MIDI_DIN5.ino
 * -----------------------------------------------------
 * Maestro con DOBLE I2C:
 *   - I2C-IN  -> lee APOLO3  (0x0D)   [entradas]
 *   - I2C-OUT -> envía a MÓDULO MIDI (0x30) [salidas DIN-5]
 *
 * Interpretación musical EXACTA a tu controlador de referencia:
 *   - Se usa Apolo3Processor + StringHandler (tu código).
 *   - En Apolo3Processor::update() ya se hace el "swap" para que:
 *       FSR500 real = Gate  (processDotFSR en StringHandler)
 *       DotFSR real = CC1   (processFSR en StringHandler)
 *     (ver archivos que me pasaste).
 *
 * Este sketch solo:
 *   - Lee el frame (18 bytes) de Apolo3 en 0x0D (Soft, FSR, Dot por cuerda).
 *   - Llama a apolo.update(raw).
 *   - Extrae eventos por cuerda y los empaqueta a tripletas MIDI,
 *     incl. Pitch Bend continuo (PB centro=8192), y los manda al módulo MIDI.
 *
 * Comentado a fondo para tu TFG.
 *******************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <vector>

// === Tus librerías (NO las modificamos) ===
#include "Apolo3Processor.h"   // orquesta 3 cuerdas con StringHandler dentro
#include "StringHandler.h"     // lógica por cuerda (gate/mod/pb/nota)

// ===================== CONFIG: BUSES I2C =======================

// Instancias TwoWire separadas
TwoWire I2C_IN  = TwoWire(0);  // Bus ENTRADAS (Apolo)
TwoWire I2C_OUT = TwoWire(1);  // Bus SALIDAS  (MIDI)

static const int SDA_IN  = 8;   // Ajusta a tus pines reales
static const int SCL_IN  = 9;
static const int SDA_OUT = 11;  // Ajusta a tus pines reales
static const int SCL_OUT = 12;

static const uint32_t I2C_FREQ_IN  = 100000;  // robusto
static const uint32_t I2C_FREQ_OUT = 100000;

// Direcciones I2C
static const uint8_t APOLO_ADDR = 0x0D; // confirmado
static const uint8_t MIDI_ADDR  = 0x30; // AJUSTA si tu módulo usa otra

// Lectura Apolo
static const uint8_t  APOLLO_FRAME_BYTES = 18;   // 9 x uint16_t
static const bool     APOLLO_MSB_FIRST   = true; // pon false si ves valores raros
static const uint32_t READ_PERIOD_MS     = 5;    // ~200 Hz

// ===================== CONFIG: MUSICAL =========================

// Canales por cuerda
static const uint8_t MIDI_CH[3] = {1, 2, 3};

// Afinaciones tipo bajo E A D (20 notas por cuerda)
// (Ajustables con apolo.setStringConfig(...))
static const uint8_t NOTE_START[3] = {28, 33, 38}; // E1, A1, D2
static const uint8_t NOTE_RANGE    = 20;

// Pitch Bend
static const uint8_t PB_SEMITONES  = 2;   // ±2 semitonos
static const uint16_t PB_DEADBAND  = 8;   // no spamear cerca del centro

// ===================== DEBUG ==================================
#define DEBUG 1
static const uint32_t DEBUG_DUMP_EVERY_MS = 1000;

// ===================== OBJETOS Y ESTADO =======================
Apolo3Processor apolo;       // usa internamente 3 x StringHandler (tu código)
uint32_t lastReadMs  = 0;
uint32_t lastDebugMs = 0;

// Seguimiento local de PB para no inundar
uint16_t lastPB[3] = {8192, 8192, 8192};

// ===================== UTILIDADES =============================

void scanBus(TwoWire& bus, const char* name) {
  Serial.printf("\n[SCAN %s] ---------\n", name);
  for (uint8_t addr=1; addr<127; ++addr) {
    bus.beginTransmission(addr);
    uint8_t err = bus.endTransmission(true);
    if (err == 0) Serial.printf("  - 0x%02X\n", addr);
  }
  Serial.printf("[SCAN %s] ---------\n", name);
}

// Convierte 18 bytes en 9 uint16_t
static inline void decodeApoloFrame(const uint8_t* buf, uint16_t outVals[9]) {
  for (int i = 0; i < 9; ++i) {
    uint8_t b0 = buf[i*2 + 0];
    uint8_t b1 = buf[i*2 + 1];
    outVals[i] = APOLLO_MSB_FIRST ? ((uint16_t)b0 << 8) | b1
                                  : ((uint16_t)b1 << 8) | b0;
  }
}

// Helpers de tripletas MIDI
static inline void pushNoteOn (std::vector<uint8_t>& v, uint8_t ch, uint8_t note, uint8_t vel) {
  v.push_back(0x90 | ((ch-1)&0x0F)); v.push_back(note&0x7F); v.push_back(vel&0x7F);
}
static inline void pushNoteOff(std::vector<uint8_t>& v, uint8_t ch, uint8_t note, uint8_t vel) {
  v.push_back(0x80 | ((ch-1)&0x0F)); v.push_back(note&0x7F); v.push_back(vel&0x7F);
}
static inline void pushCC     (std::vector<uint8_t>& v, uint8_t ch, uint8_t num, uint8_t val) {
  v.push_back(0xB0 | ((ch-1)&0x0F)); v.push_back(num&0x7F);  v.push_back(val&0x7F);
}
static inline void pushPB     (std::vector<uint8_t>& v, uint8_t ch, uint16_t v14) {
  uint8_t lsb = v14 & 0x7F, msb = (v14 >> 7) & 0x7F;
  v.push_back(0xE0 | ((ch-1)&0x0F)); v.push_back(lsb); v.push_back(msb);
}

// Nombres de nota para impresión humana
String noteName(uint8_t note) {
  static const char* N[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  int n = note % 12, oct = (note/12)-1;
  char buf[8]; snprintf(buf,sizeof(buf),"%s%d",N[n],oct);
  return String(buf);
}
void printMidiHuman(uint8_t st, uint8_t d1, uint8_t d2) {
  uint8_t t = st & 0xF0, ch=(st&0x0F)+1;
  if (t==0x90) Serial.printf("MIDI → ch%u NoteOn  %s (%u) vel=%u\n",ch,noteName(d1).c_str(),d1,d2);
  else if (t==0x80) Serial.printf("MIDI → ch%u NoteOff %s (%u) vel=%u\n",ch,noteName(d1).c_str(),d1,d2);
  else if (t==0xB0) Serial.printf("MIDI → ch%u CC%u val=%u\n",ch,d1,d2);
  else if (t==0xE0) {
    uint16_t v14 = ((uint16_t)d2<<7) | (d1&0x7F);
    int bend = (int)v14 - 8192;
    Serial.printf("MIDI → ch%u PitchBend %+d (v14=%u)\n", ch, bend, v14);
  }
  else Serial.printf("MIDI → ch%u 0x%02X %u %u\n", ch, st, d1, d2);
}

// Empaqueta [N]+tripletas y envía por I2C-OUT al módulo MIDI
bool midiFlushTriplets(std::vector<uint8_t>& triplets) {
  if (triplets.empty()) return true;

#if DEBUG
  for (size_t i=0;i+2<triplets.size();i+=3)
    printMidiHuman(triplets[i],triplets[i+1],triplets[i+2]);
#endif

  std::vector<uint8_t> pkt; pkt.reserve(1+triplets.size());
  pkt.push_back((uint8_t)(triplets.size()/3));
  pkt.insert(pkt.end(), triplets.begin(), triplets.end());

  I2C_OUT.beginTransmission(MIDI_ADDR);
  for (uint8_t b: pkt) I2C_OUT.write(b);
  uint8_t err = I2C_OUT.endTransmission();

#if DEBUG
  if (err) Serial.printf("[MIDI-OUT] I2C error=%u\n", err);
#endif

  triplets.clear();
  return err==0;
}

// ===================== SETUP ===================================
void setup() {
  Serial.begin(115200);
  delay(100);

  // I2C
  I2C_IN.begin (SDA_IN,  SCL_IN,  I2C_FREQ_IN);
  I2C_OUT.begin(SDA_OUT, SCL_OUT, I2C_FREQ_OUT);
  I2C_IN.setTimeOut (1000);
  I2C_OUT.setTimeOut(1000);

#if DEBUG
  Serial.println(F("\n[CONTROLADOR 3.4.0] Doble I2C listo."));
  Serial.printf("IN : SDA=%d SCL=%d @%lu\n", SDA_IN,  SCL_IN,  (unsigned long)I2C_FREQ_IN);
  Serial.printf("OUT: SDA=%d SCL=%d @%lu\n", SDA_OUT, SCL_OUT, (unsigned long)I2C_FREQ_OUT);
  Serial.printf("Apolo@0x%02X  MIDI@0x%02X\n", APOLO_ADDR, MIDI_ADDR);
  scanBus(I2C_IN,  "IN");
  scanBus(I2C_OUT, "OUT");
#endif

  // Configura cuerdas (E1/A1/D2, 20 notas), PB continuo (trasteado= false)
  // -> Esto permite que el StringHandler haga: base discreta + PB continuo (si lo usas así).
  apolo.setStringConfig(0, MIDI_CH[0], NOTE_START[0], NOTE_RANGE, /*trasteado*/false, /*PB*/true, PB_SEMITONES);
  apolo.setStringConfig(1, MIDI_CH[1], NOTE_START[1], NOTE_RANGE, /*trasteado*/false, /*PB*/true, PB_SEMITONES);
  apolo.setStringConfig(2, MIDI_CH[2], NOTE_START[2], NOTE_RANGE, /*trasteado*/false, /*PB*/true, PB_SEMITONES);

  lastReadMs  = millis();
  lastDebugMs = lastReadMs;
}

// ===================== LOOP ====================================
void loop() {
  uint32_t now = millis();

  // 1) Poll a Apolo
  if (now - lastReadMs >= READ_PERIOD_MS) {
    lastReadMs = now;

    // Ping a la dirección (ACK)
    I2C_IN.beginTransmission(APOLO_ADDR);
    uint8_t pingErr = I2C_IN.endTransmission(true);
    if (pingErr) {
#if DEBUG
      Serial.printf("[Apolo] NACK ping err=%u\n", pingErr);
#endif
      delay(2);
      return;
    }

    // Solicita 18 bytes
    int req = I2C_IN.requestFrom((uint16_t)APOLO_ADDR, (size_t)APOLLO_FRAME_BYTES, (bool)true);
    if (req != APOLLO_FRAME_BYTES) {
#if DEBUG
      Serial.printf("[Apolo] requestFrom=%d (esperado %d)\n", req, APOLLO_FRAME_BYTES);
#endif
      delay(2);
      return;
    }

    // Lee frame
    uint8_t buf[APOLLO_FRAME_BYTES];
    int rcv=0; while (I2C_IN.available() && rcv<APOLLO_FRAME_BYTES) buf[rcv++]=I2C_IN.read();
    if (rcv != APOLLO_FRAME_BYTES) {
#if DEBUG
      Serial.printf("[Apolo] recibidos %d/%d bytes\n", rcv, APOLLO_FRAME_BYTES);
#endif
      delay(2);
      return;
    }

    // 2) Decodifica -> 9 uint16_t (Soft, FSR, Dot) * 3
    uint16_t vals[9]={0}; decodeApoloFrame(buf, vals);

    // 3) Prepara vector para Apolo3Processor (orden Soft, FSR, Dot)
    std::vector<uint16_t> raw; raw.reserve(9);
    for (int i=0;i<3;++i) {
      raw.push_back(vals[i*3 + 0]); // Soft
      raw.push_back(vals[i*3 + 1]); // FSR (gate real)
      raw.push_back(vals[i*3 + 2]); // DotFSR (modulación real)
    }

    // 4) Actualiza tu pipeline (internamente hace swap para StringHandler)
    apolo.update(raw); // strings[i]->update(soft, dot, fsr) dentro del .cpp que me pasaste.  :contentReference[oaicite:3]{index=3}

    // 5) Construye lote MIDI desde eventos / estados por cuerda
    std::vector<uint8_t> triplets; triplets.reserve(3*16);

    for (int i=0;i<3;++i) {
      StringHandler* s = apolo.getString(i);
      if (!s) continue;
      uint8_t ch   = MIDI_CH[i];
      uint8_t note = s->getNote();

      // Gate (NoteOn/Off) según eventos del handler
      if (s->hasNoteOnEvent()) {
        uint8_t vel = s->getVelocity();
        pushNoteOn(triplets, ch, note, vel);
      }
      if (s->hasNoteOffEvent()) {
        pushNoteOff(triplets, ch, note, 64);
      }

      // CC1 (DotFSR real) cuando cambie (el handler ya aplica umbral interno)
      if (s->hasModulationEvent()) {
        pushCC(triplets, ch, 1 /*CC1*/, s->getModulation());
      }

      // Pitch Bend continuo (cuando trasteado=false y PB enabled)
      int16_t pb = s->getPitchBend(); // 0..16383, centro=8192  :contentReference[oaicite:4]{index=4}
      if ((uint16_t)abs(pb - (int)lastPB[i]) >= PB_DEADBAND) {
        lastPB[i] = (uint16_t)pb;
        pushPB(triplets, ch, (uint16_t)pb);
      }
    }

    // 6) Envía el paquete al módulo MIDI (I2C-OUT)
    midiFlushTriplets(triplets);
  }

#if DEBUG
  if (now - lastDebugMs >= DEBUG_DUMP_EVERY_MS) {
    lastDebugMs = now;
    apolo.printMusicalState(); // ver notas, vel, mod y PB por cuerda  :contentReference[oaicite:5]{index=5}
  }
#endif
}
