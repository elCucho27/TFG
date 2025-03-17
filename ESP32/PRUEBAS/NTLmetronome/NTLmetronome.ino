#include <Wire.h> // Librería para comunicación I2C
#include <LiquidCrystal_PCF8574.h> // Librería para controlar el LCD I2C
#include "AiEsp32RotaryEncoder.h"  // Librería para el encoder rotatorio
#include <Ticker.h>  // Librería para manejar interrupciones periódicas

// Dirección del LCD en el bus I2C
LiquidCrystal_PCF8574 lcd(0x27); // Si el LCD no responde, probar con 0x3F

// Definir pines para el encoder rotatorio
#define ROTARY_ENCODER_A_PIN 32 // Pin A del encoder (DT)
#define ROTARY_ENCODER_B_PIN 35 // Pin B del encoder (CLK)
#define ROTARY_ENCODER_BUTTON_PIN 27 // Pin del botón del encoder (SW)
#define ROTARY_ENCODER_VCC_PIN -1 // No se usa alimentación del encoder desde ESP32
#define ROTARY_ENCODER_STEPS 4 // Sensibilidad del encoder

// Crear instancia del encoder rotatorio
AiEsp32RotaryEncoder rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

// Instancia para manejar interrupciones del metrónomo
Ticker metronomeTicker;

// Declaración de variables globales
int bpm = 120; // Beats por minuto inicial
unsigned long tempoInterval = 60000 / bpm; // Intervalo de tiempo entre beats en ms

// Definir pines para los LEDs
const int ledPins[4] = {17, 5, 18, 19}; // Pines de los LEDs indicadores

// Definir pin para el buzzer
const int buzzerPin = 4; // Pin donde se conecta el buzzer

// Variables para el control del encoder
int lastEncoderValue = 0; // Último valor leído del encoder

// Variables para manejo de debounce del botón
unsigned long lastButtonPressTime = 0; // Último tiempo de pulsación del botón
unsigned long debounceDelay = 200; // Tiempo de espera para evitar rebotes del botón

// Definir pines y variables para el cambio de modo
#define MODE_SWITCH_PIN 14 // Pin del botón para cambiar entre modos
bool isFreeMode = true; // Indica el modo actual (true = Free Mode, false = Setlist)

// Definir estructura para almacenar las canciones del setlist
typedef struct {
  String title; // Nombre de la canción
  int bpm; // Beats por minuto de la canción
  String timeSignature; // Compás (ej. "4/4", "3/4")
} Song;

// Lista de canciones predefinidas en el setlist
Song setlist[] = {
  {"Sadness", 100, "4/4"},
  {"InsideMe", 120, "3/4"},
  {"Spacemen", 110, "4/4"},
  {"Old Codes", 160, "4/4"}
};
const int setlistSize = sizeof(setlist) / sizeof(setlist[0]); // Número de canciones
int currentSongIndex = 0; // Índice de la canción actual

// Función de interrupción para el encoder
void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR(); // Leer el estado del encoder en interrupción
}

void setup() {
  Serial.begin(115200); // Inicializar comunicación serie
  lcd.begin(16, 2); // Configurar el LCD con 16 columnas y 2 filas
  lcd.setBacklight(255); // Encender la retroiluminación del LCD
  
  // Configurar LEDs como salidas
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Apagar los LEDs al inicio
  }

  // Configurar buzzer como salida
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); // Apagar buzzer al inicio

  // Inicializar encoder rotatorio
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);

  // Configurar botón de cambio de modo
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

  // Mostrar información inicial en LCD
  mostrarLCD();

  // Configurar el metrónomo con el BPM inicial
  metronomeTicker.attach_ms(tempoInterval, metronomeTick);
}

void loop() {
  // Detectar cambios en el encoder rotatorio
  rotaryLoop();
  
  // Verificar si se ha presionado el botón del encoder
  checkButtonPress();
  
  // Verificar si se ha presionado el botón de cambio de modo
  checkModeSwitch();
}

void rotaryLoop() {
  if (rotaryEncoder.encoderChanged()) { // Si el encoder ha cambiado de posición
    int currentEncoderValue = rotaryEncoder.readEncoder(); // Leer nuevo valor
    
    if (isFreeMode) { // Si está en modo libre, modificar BPM
      bpm = constrain(bpm + (currentEncoderValue > lastEncoderValue ? 1 : -1), 40, 330);
    } else { // Si está en modo Setlist, cambiar canción
      currentSongIndex = (currentSongIndex + (currentEncoderValue > lastEncoderValue ? 1 : -1) + setlistSize) % setlistSize;
      bpm = setlist[currentSongIndex].bpm; // Ajustar BPM a la canción seleccionada
    }

    tempoInterval = 60000 / bpm; // Calcular nuevo intervalo
    metronomeTicker.detach();
    metronomeTicker.attach_ms(tempoInterval, metronomeTick);
    mostrarLCD(); // Actualizar pantalla LCD

    lastEncoderValue = currentEncoderValue; // Guardar último valor leído
  }
}

void checkButtonPress() {
  if (digitalRead(ROTARY_ENCODER_BUTTON_PIN) == LOW) {
    if (millis() - lastButtonPressTime > debounceDelay) {
      lastButtonPressTime = millis();
      if (isFreeMode) {
        bpm = constrain(bpm, 40, 330);
      }
      mostrarLCD();
    }
  }
}

void checkModeSwitch() {
  if (digitalRead(MODE_SWITCH_PIN) == LOW) {
    if (millis() - lastButtonPressTime > debounceDelay) {
      lastButtonPressTime = millis();
      isFreeMode = !isFreeMode; // Cambiar entre modos
      mostrarLCD(); // Actualizar pantalla LCD
    }
  }
}

void mostrarLCD() {
  lcd.clear(); // Limpiar pantalla LCD
  lcd.setCursor(0, 0); // Posicionar cursor en primera línea
  lcd.print(isFreeMode ? "Free Mode:" : "Setlist:"); // Mostrar modo actual
  lcd.setCursor(0, 1); // Segunda línea
  if (isFreeMode) {
    lcd.print("BPM: ");
    lcd.print(bpm);
  } else {
    lcd.print(setlist[currentSongIndex].title);
    lcd.print(" ");
    lcd.print(bpm);
    lcd.print(" ");
    lcd.print(setlist[currentSongIndex].timeSignature);
  }
}

void metronomeTick() {
  static int ledIndex = 0;
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  digitalWrite(ledPins[ledIndex], HIGH);
  tone(buzzerPin, 1000, 100);
  ledIndex = (ledIndex + 1) % 4;
}
