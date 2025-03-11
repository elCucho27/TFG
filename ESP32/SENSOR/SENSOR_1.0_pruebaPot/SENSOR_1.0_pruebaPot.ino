// CONTROLADOR_prueba_1.0
/*
Se mapea un potenciómetro de 10K(B) lineal al rango de volumen MIDI (0-127)
*/

// Variables
const int potAnalogPin = 33;

int lastMidiValue = -1; // Último valor MIDI enviado (-1 para asegurar el primer envío)

void setup() {
  Serial.begin(115200);
  pinMode(potAnalogPin, INPUT);
}

void loop() {
  int value = analogRead(potAnalogPin); // Leer el valor del potenciómetro (0-4095)
  
  // Mapear el valor a un rango de 0 a 127 (rango MIDI)
  int midiValue = map(value, 0, 4095, 0, 127);

  // Solo enviar si el valor cambia para evitar redundancias
  if (midiValue != lastMidiValue) {
    Serial.print("Potenciómetro: ");
    Serial.print(value);
    Serial.print(" -> MIDI Volume: ");
    Serial.println(midiValue);

    lastMidiValue = midiValue; // Actualizar el último valor MIDI enviado
  }

  delay(20); // Pequeña pausa para estabilizar la lectura
}
