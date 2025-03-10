/*#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // put your setup code here, to run once:
  MIDI.begin(1);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.write(144);
  Serial.write(60);
  Serial.write(100);
  delay(1000);
}




--------------------------------------------------------

#include <MIDI.h>  // Incluye la librería MIDI

#define MIDI_BAUDRATE 115200  // Importante: Hairless debe estar configurado a esta velocidad
#define MIDI_CHANNEL 1        // Canal MIDI (1-16)

// Inicializa la instancia MIDI en el puerto serie por defecto
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
    Serial.begin(MIDI_BAUDRATE); // Iniciar la comunicación serie
    MIDI.begin(MIDI_CHANNEL);    // Iniciar MIDI en el canal seleccionado
    delay(1000); // Esperar un poco para evitar problemas de conexión
}

void loop() {
    MIDI.sendNoteOn(60, 100, MIDI_CHANNEL); // Enviar Note On (C4)
    delay(500); 
    MIDI.sendNoteOff(60, 0, MIDI_CHANNEL); // Enviar Note Off
    delay(500);
}
*/

#include <Arduino.h>

#define MIDI_BAUDRATE 115200  // Hairless MIDI debe coincidir con este baudrate
#define MIDI_CHANNEL 1

void setup() {
    Serial.begin(MIDI_BAUDRATE); // Iniciar Serial a 115200 baudios
    delay(1000);
    Serial.println("ESP32 MIDI listo..."); // Mensaje de prueba
}

void noteOn(byte channel, byte pitch, byte velocity) {
    Serial.write(0x90 | (channel & 0x0F)); // Mensaje MIDI Note On
    Serial.write(pitch);
    Serial.write(velocity);
    Serial.println("Note On enviada"); // Mensaje de depuración
}

void noteOff(byte channel, byte pitch, byte velocity) {
    Serial.write(0x80 | (channel & 0x0F)); // Mensaje MIDI Note Off
    Serial.write(pitch);
    Serial.write(velocity);
    Serial.println("Note Off enviada"); // Mensaje de depuración
}

void loop() {
    noteOn(MIDI_CHANNEL, 60, 127); // Enviar nota C4 (60)
    delay(500);
    noteOff(MIDI_CHANNEL, 60, 0);  // Apagar nota C4
    delay(500);
}
