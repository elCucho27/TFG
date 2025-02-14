/***************************************************
 * Envío de mensajes MIDI en ESP32
 * 
 * Enciende y apaga distintas notas repetidamente
 * 
 * 
 ***************************************************/

#include <MIDI.h>

// Define qué puerto serie usarás para MIDI.
// En muchas placas ESP32, Serial2 corresponde a GPIO17 (TX2) y GPIO16 (RX2).
#define MIDI_SERIAL Serial2

// Crea la instancia MIDI sobre el puerto serie elegido
MIDI_CREATE_INSTANCE(HardwareSerial, MIDI_SERIAL, MIDI);

void setup() {
  // Inicia el puerto serie para MIDI a la velocidad estándar (31.250 baudios).
  MIDI_SERIAL.begin(31250);

  // Inicia la librería MIDI en todos los canales (MIDI_CHANNEL_OMNI).
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // Serial normal (USB) para depuración (opcional)
  Serial.begin(115200);
  Serial.println("Iniciando ESP32 con MIDI por Serial2...");
}

void loop() {
  int note = 60;       // Nota MIDI 60 = C4 (Do central)
  int velocity = 100;  // Velocidad (0-127)

  // Envía Note On en el canal 1
  MIDI.sendNoteOn(note, velocity, 1);
  Serial.println("Note On (C4)");
  delay(500);

  // Envía Note Off en el canal 1
  MIDI.sendNoteOff(note, 0, 1);
  Serial.println("Note Off (C4)");
  delay(500);
}

