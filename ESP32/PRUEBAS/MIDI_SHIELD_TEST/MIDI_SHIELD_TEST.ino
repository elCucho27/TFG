#include <Arduino.h>

// ======================================================
// CONFIGURACIÓN BÁSICA MIDI
// ======================================================

const int MIDI_TX_PIN = 17;      // pin TX hacia circuito MIDI OUT
const long MIDI_BAUD = 31250;    // velocidad estándar MIDI DIN
const uint8_t MIDI_CHANNEL = 1;  // canal MIDI (1–16)

// ======================================================
// ESTRUCTURA DE NOTA
// ======================================================

struct NoteEvent {
  uint8_t note;     // número de nota MIDI (0 = pausa)
  uint16_t duration; // duración en múltiplos del "beat"
};

// ======================================================
// TABLA DE NOTAS "AXEL F" (CRAZY FROG)
// ======================================================
//
// Melodía principal, versión correcta (en Fa menor)
//
//   F4  G#4  F4  F4  A#4  F4  D#4  F4  C4  F4
//   F4  C#4  C4  G#3  F4  C4  F4  F4  D#4  D#4
//   C4  G3   F4  F4  G#4  F4  F4  A#4  F4  D#4
//   F4  C4   F4  F4  C#4  C4  G#3  F4  C4  F4
//   F4  D#4  D#4  C4  G3  F4  F4  G#4  F4  F4
//   A#4 F4   D#4  F4  C4  F4  F4  C#4  C4  G#3
//   F4  C4   F4  F4  D#4  D#4  C4  G3  F4
//
// (Cada nota = una corchea; puedes ajustar el tempo con BEAT_MS)

NoteEvent melody[] = {
  {65,1},{68,1},{65,1},{65,1},{70,1},{65,1},{63,1},{65,1},{60,1},{65,1},
  {65,1},{61,1},{60,1},{56,1},{65,1},{60,1},{65,1},{65,1},{63,1},{63,1},
  {60,1},{55,1},{65,1},{65,1},{68,1},{65,1},{65,1},{70,1},{65,1},{63,1},
  {65,1},{60,1},{65,1},{65,1},{61,1},{60,1},{56,1},{65,1},{60,1},{65,1},
  {65,1},{63,1},{63,1},{60,1},{55,1},{65,1},{65,1},{68,1},{65,1},{65,1},
  {70,1},{65,1},{63,1},{65,1},{60,1},{65,1},{65,1},{61,1},{60,1},{56,1},
  {65,1},{60,1},{65,1},{65,1},{63,1},{63,1},{60,1},{55,1},{65,1}
};

const int melodyLength = sizeof(melody)/sizeof(melody[0]);

// ======================================================
// PARÁMETROS DE TIEMPO
// ======================================================
unsigned long BEAT_MS = 180;  // Duración de cada nota (~167 BPM aprox.)

// ======================================================
// FUNCIONES MIDI
// ======================================================

// Enviar nota ON
void midiNoteOn(uint8_t note, uint8_t velocity) {
  Serial1.write(0x90 | ((MIDI_CHANNEL - 1) & 0x0F));
  Serial1.write(note & 0x7F);
  Serial1.write(velocity & 0x7F);
}

// Enviar nota OFF
void midiNoteOff(uint8_t note) {
  Serial1.write(0x80 | ((MIDI_CHANNEL - 1) & 0x0F));
  Serial1.write(note & 0x7F);
  Serial1.write(0x00);
}

// ======================================================
// SETUP
// ======================================================

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Reproduciendo Axel F (Crazy Frog) por MIDI OUT");

  Serial1.begin(MIDI_BAUD, SERIAL_8N1, -1, MIDI_TX_PIN);
  Serial.printf("TX MIDI en pin %d\n", MIDI_TX_PIN);
}

// ======================================================
// LOOP PRINCIPAL
// ======================================================

void loop() {
  for (int i = 0; i < melodyLength; i++) {
    uint8_t note = melody[i].note;
    uint16_t dur = melody[i].duration;

    if (note != 0) {
      midiNoteOn(note, 100);          // Nota ON (velocidad media)
      delay(dur * BEAT_MS);           // Mantener duración
      midiNoteOff(note);              // Nota OFF
    } else {
      delay(dur * BEAT_MS);           // Pausa
    }

    delay(20); // pequeña separación
  }

  delay(1500);  // pausa antes de repetir la melodía
}
