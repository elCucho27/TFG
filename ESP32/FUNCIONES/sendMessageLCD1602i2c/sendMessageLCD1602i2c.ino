#include <Wire.h>
#include <LiquidCrystal_PCF8574.h> // Librería LCD I2C

// Configuración del LCD I2C
LiquidCrystal_PCF8574 lcd(0x27); // Cambia la dirección si es necesario

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial a 115200 baudios

  // Inicializar LCD
  lcd.begin(16, 2); // 16 columnas, 2 filas
  lcd.setBacklight(255); // Encender retroiluminación

  Serial.println("Introduzca un mensaje y presione Enter para mostrarlo en el LCD:");
}

void loop() {
  entradaTecladoPC();
}

// Función para mostrar el mensaje en el LCD
void mostrarLCD(String mensaje) {
  lcd.clear(); // Limpiar la pantalla del LCD

  // Mostrar el mensaje en la primera línea del LCD
  lcd.setCursor(0, 0);
  lcd.print(mensaje.substring(0, 16)); // Muestra solo los primeros 16 caracteres

  // Si el mensaje es mayor de 16 caracteres, mostrar el resto en la segunda línea
  if (mensaje.length() > 16) {
    lcd.setCursor(0, 1);
    lcd.print(mensaje.substring(16, 32)); // Muestra los siguientes 16 caracteres
  }
}

//Funcion de lectura por teclado
void entradaTecladoPC(){

    if (Serial.available() > 0) {
    String mensaje = Serial.readStringUntil('\n'); // Leer el mensaje del teclado hasta la tecla Enter
    mensaje.trim(); // Eliminar posibles saltos de línea u otros espacios extras

    if (mensaje.length() > 0) {
      mostrarLCD(mensaje); // Mostrar el mensaje en el LCD
    }
  }
}