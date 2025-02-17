#include <MIDI.h>

// Define qué puerto serie usarás para MIDI.
// En muchas placas ESP32, Serial2 corresponde a GPIO17 (TX2) y GPIO16 (RX2).
#define MIDI_SERIAL Serial2

// Crea la instancia MIDI sobre el puerto serie elegido
MIDI_CREATE_INSTANCE(HardwareSerial, MIDI_SERIAL, MIDI);

void setup() {
  // Inicia el puerto serie para MIDI a la velocidad estándar (31.250 baudios).
  MIDI_SERIAL.begin(115200);

  // Inicia la librería MIDI en todos los canales (MIDI_CHANNEL_OMNI).
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // Serial normal (USB) para depuración (opcional)
  Serial.begin(115200);
  Serial.println("Iniciando ESP32 con MIDI por Serial2...");
}

void loop() {
  // Definir las notas de "Para Elisa" (simplificadas)
  int notes[] = { 
    64,  63,  64,  63,  64,  59,  62,  60,  57,  // E5, D#5, E5, D#5, E5, B4, D5, C5, A4
    64,  63,  64,  63,  64,  59,  62,  60,  57,  // E5, D#5, E5, D#5, E5, B4, D5, C5, A4
    64,  63,  64,  63,  64,  59,  62,  60,  57,  // E5, D#5, E5, D#5, E5, B4, D5, C5, A4
    64,  63,  64,  63,  64,  59,  62,  60,  57   // E5, D#5, E5, D#5, E5, B4, D5, C5, A4
  };

  // Definir la duración de las notas en milisegundos (500 ms por nota)
  int noteDuration = 500;

  // Reproducir las notas de "Para Elisa"
  for (int i = 0; i < 36; i++) {
    int note = notes[i];
    int velocity = 100;  // Velocidad (0-127)

    // Enviar Note On en el canal 1
    MIDI.sendNoteOn(note, velocity, 1);
    Serial.write(144);
    Serial.write(note);
    Serial.write(velocity);
    Serial.print("Note On: ");
    Serial.println(note);
    delay(noteDuration);

    // Enviar Note Off en el canal 1
    MIDI.sendNoteOff(note, 0, 1);
    Serial.write(144);
    Serial.write(note);
    Serial.write(0);
    Serial.print("Note Off: ");
    Serial.println(note);
    delay(noteDuration);
  }

  // Repetir el ciclo una vez terminado
  delay(1000);  // Pausa antes de repetir
}
