#include <Arduino.h>

// Pin TX conectado al shield MIDI OUT
const int MIDI_TX_PIN = 17;  // modifica si usas otro

// Constantes MIDI
const long MIDI_BAUD = 31250;
const uint8_t MIDI_CHANNEL = 1;

// Estructura nota + duración
struct NoteEvent {
  uint8_t note;
  uint16_t duration;
};

// Fragmento del tema inicial de Für Elise
NoteEvent furEliseMelody[] = {
  {76, 1},  {75, 1},  {76, 1},  {75, 1},  {76, 1},  {71, 1},  {74, 1},  {72, 1},  {69, 2},
  {0,  1},  // pausa
  {64, 1},  {67, 1},  {69, 1},  {71, 1},  {0,  1},
  {64, 1},  {68, 1},  {71, 1},  {72, 1},  {0,  1},
  {64, 1},  {76, 1},  {75, 1},  {76, 1},  {75, 1},  {76, 1},  {71, 1},  {74, 1},  {72, 1},  {69, 2}
};

const int furLength = sizeof(furEliseMelody)/sizeof(furEliseMelody[0]);

// Define cuánto dura un “pulso” en milisegundos
unsigned long BEAT_MS = 300;  // ajustar tempo: por ejemplo 300 ms por pulso

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Iniciando Für Elise (fragmento) por MIDI OUT");

  Serial1.begin(MIDI_BAUD, SERIAL_8N1, -1, MIDI_TX_PIN);
  Serial.printf("Serial1 iniciado: TX pin %d, baud %ld\n", MIDI_TX_PIN, MIDI_BAUD);
}

void loop() {
  for (int i = 0; i < furLength; i++) {
    NoteEvent ev = furEliseMelody[i];
    uint8_t note = ev.note;
    uint16_t dur = ev.duration;

    if (note != 0) {
      // enviar Note On
      Serial1.write(0x90 | ((MIDI_CHANNEL - 1) & 0x0F));
      Serial1.write(note & 0x7F);
      Serial1.write(0x7F);  // velocidad máxima

      // espera duración de la nota
      delay(dur * BEAT_MS);

      // enviar Note Off
      Serial1.write(0x80 | ((MIDI_CHANNEL - 1) & 0x0F));
      Serial1.write(note & 0x7F);
      Serial1.write(0x00);
    } else {
      // es una pausa — simplemente esperar
      delay(dur * BEAT_MS);
    }

    // pequeña separación entre notas para evitar ligar demasiado
    delay(30);
  }

  // pausa entre repeticiones
  delay(2000);
}
