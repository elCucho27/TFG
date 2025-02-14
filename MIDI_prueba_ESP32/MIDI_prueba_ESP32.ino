#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  Serial.begin(115200);           // Inicializa el Serial a 115200 baudios
  delay(1000);                    // Espera un poco para dar tiempo a que arranque el puerto serie

  // En la ESP32, 'while (!Serial)' no es necesario (y a veces no funciona igual que en Teensy),
  // por lo que lo omitimos.
  
  Serial.println("Starting code on ESP32...");

  // Si quieres usar la librería MIDI para enviar datos por Serial (no por USB), puedes iniciar así:
  // MIDI.begin(MIDI_CHANNEL_OMNI);
  // Pero ten en cuenta que esto enviará datos por los pines Serial, no por USB.
}

void loop() {
  Serial.println("Looping on ESP32...");
  delay(1000);  // Espera 1 segundo antes de volver a imprimir
}
