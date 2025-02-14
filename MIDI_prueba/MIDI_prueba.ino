#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  Serial.begin(115200);         // Inicializa el Serial Monitor a 115200 baudios
 /* while (!Serial) {
    // En algunas placas Teensy, esto espera a que el puerto serie esté listo
  }
*/
  MIDI.begin(MIDI_CHANNEL_OMNI); // Inicia la comunicación MIDI en todos los canales
  Serial.println("Starting code..."); 
}

void loop() {
  int note = 60;      // Nota MIDI 60 (C4)
  int velocity = 100; // Velocidad (intensidad)

  Serial.println("Sending Note On...");
  MIDI.sendNoteOn(note, velocity, 1); // Envía la nota en el canal 1
  delay(200);

  Serial.println("Sending Note Off...");
  MIDI.sendNoteOff(note, 0, 1);       // Apaga la nota en el canal 1
  delay(200);
}
