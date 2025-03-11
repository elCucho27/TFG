#include <MIDI.h>

// Crear una instancia de MIDI utilizando el puerto serial (Serial1)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void setup() {
  Serial.begin(115200);  // Configura el puerto serial para depuración (Monitor)
  //Serial1.begin(31250);  // Configurar el puerto serial 1 a 31250 baudios (velocidad MIDI)
  
  // Iniciar la comunicación MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);  // Omni: Recibe en todos los canales MIDI
}

void loop() {
  // Enviar mensaje de Nota ON
  MIDI.sendNoteOn(60, 127, 1);  // Nota 60 (C4), Velocidad 127, Canal 1
  Serial.println("Nota ON enviada");

  delay(500);  // Esperar 500 ms

  // Enviar mensaje de Nota OFF
  MIDI.sendNoteOff(60, 0, 1);  // Nota 60 (C4), Velocidad 0 (para apagar la nota), Canal 1
  Serial.println("Nota OFF enviada");

  delay(500);  // Esperar 500 ms antes de repetir el ciclo
}
