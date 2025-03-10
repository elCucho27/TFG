// Definimos los pines para el potenciómetro y el pulsador
const int potPin = 33;   // Pin para el potenciómetro (entrada analógica)
const int buttonPin = 23; // Pin para el pulsador (entrada digital)

// Variables para el estado del botón y la nota MIDI
int potValue = 0;    // Valor leído del potenciómetro
int midiNote = 0;        // Nota MIDI correspondiente
bool notePlaying = false; // Indica si la nota está activa
bool lastButtonState = HIGH; // Estado anterior del botón

unsigned long lastDebounceTime = 0; // Última vez que se detectó un cambio
const unsigned long debounceDelay = 50; // Tiempo de espera para filtrar rebotes (en ms)

void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial para depuración
  pinMode(buttonPin, INPUT_PULLUP);  // Configurar pulsador con resistencia pull-up interna
}

void loop() {
  // Leer el estado actual del botón
  bool buttonState = digitalRead(buttonPin);

  // Leer el valor del potenciómetro y mapearlo a notas MIDI
  potValue = analogRead(potPin);
  midiNote = map(potValue, 0, 4095, 36, 72);  // Nota MIDI entre 60 (C4) y 72 (C5)

  // Mostrar en el monitor serie el valor del potenciómetro y la nota correspondiente
 /*
  Serial.print("Pot Value: ");
  Serial.print(potValue);
  Serial.print(" -> MIDI Note: ");
  Serial.println(midiNote);
*/
  // Verificar si el estado del botón ha cambiado después del tiempo de debounce
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis(); // Resetear el tiempo de debounce
  }

  if ((millis() - lastDebounceTime) > debounceDelay) { 
    // Solo actuamos si el estado ha sido estable por más de debounceDelay
    if (buttonState == LOW && !notePlaying) {
      Serial.print("Botón presionado - Enviando NOTE ON: ");
      Serial.println(midiNote);
      sendMIDI(midiNote, true);
      notePlaying = true;
    } else if (buttonState == HIGH && notePlaying) {
      Serial.print("Botón liberado - Enviando NOTE OFF: ");
      Serial.println(midiNote);
      sendMIDI(midiNote, false);
      notePlaying = false;
    }
  }

  lastButtonState = buttonState;  // Guardar el estado del botón

  delay(10);  // Pequeña pausa para evitar sobrecarga de la CPU
}

// Función para enviar un mensaje MIDI (Note On / Note Off)
void sendMIDI(int note, bool noteOn) {
  if (noteOn) {
    Serial.write(0x90);  // Status byte para Note On (Canal 1)
    Serial.write(note);  // Nota MIDI
    Serial.write(100);   // Velocidad
    Serial.print("Pot Value: ");
    Serial.print(potValue);
    Serial.print(" -> MIDI Note: ");
    Serial.println(midiNote);
  } else {
    Serial.write(0x80);  // Status byte para Note Off (Canal 1)
    Serial.write(note);  // Nota MIDI
    Serial.write(0);     // Velocidad 0
  }
}
