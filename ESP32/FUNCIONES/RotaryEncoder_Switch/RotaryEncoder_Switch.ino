#include "AiEsp32RotaryEncoder.h"

// Pines del encoder (ajústalos según tu conexión)
#define ROTARY_ENCODER_A_PIN 32  // DT
#define ROTARY_ENCODER_B_PIN 35  // CLK
#define ROTARY_ENCODER_BUTTON_PIN 27  // SW
#define ROTARY_ENCODER_VCC_PIN -1  // No usado
#define ROTARY_ENCODER_STEPS 4  // Sensibilidad

// Crear objeto del encoder
AiEsp32RotaryEncoder rotaryEncoder(
    ROTARY_ENCODER_A_PIN, 
    ROTARY_ENCODER_B_PIN, 
    ROTARY_ENCODER_BUTTON_PIN, 
    ROTARY_ENCODER_VCC_PIN, 
    ROTARY_ENCODER_STEPS
);

// Función de interrupción
void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // Configurar el encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(-9999, 9999, false);  // Sin límites de giro

  // Configurar el botón con pull-up interno
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // Comprobar si el valor del encoder ha cambiado
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Encoder Value: ");
    Serial.println(rotaryEncoder.readEncoder());  // Imprimir valor del encoder
  }

  // Comprobar si se presionó el botón
  if (rotaryEncoder.isEncoderButtonClicked()) {
    Serial.println("Botón presionado!");
  }

  delay(50);  // Pequeño retardo para estabilidad
}
