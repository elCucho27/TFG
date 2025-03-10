#include <LiquidCrystal.h>

// Pines del LCD (modo paralelo)
LiquidCrystal lcd(4, 5, 18, 19, 21, 22);

// Encoder Pinout
#define CLK 32  
#define DT 33   
#define SW 25   

// Switches
#define SW_SUB5 26  // Botón que resta 5
#define SW_ADD5 27  // Botón que suma 5
#define SW_ADD10 14 // Botón que suma 10

volatile int contador = 0;  
int lastStateCLK;

void setup() {
    Serial.begin(115200);

    // Inicializar LCD
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Contador: ");
    lcd.setCursor(0, 1);
    lcd.print(contador);

    // Configurar pines
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);
    
    pinMode(SW_SUB5, INPUT_PULLUP);
    pinMode(SW_ADD5, INPUT_PULLUP);
    pinMode(SW_ADD10, INPUT_PULLUP);

    lastStateCLK = digitalRead(CLK);
}

void loop() {
    int currentStateCLK = digitalRead(CLK);

    // Detecta el cambio de estado en CLK
    if (currentStateCLK != lastStateCLK) {
        if (digitalRead(DT) != currentStateCLK) {
            contador++;  // Girando en una dirección
        } else {
            contador--;  // Girando en la otra dirección
        }

        actualizarLCD();
        delay(5);  // debounce
    }

    lastStateCLK = currentStateCLK;  // Guarda el estado para la siguiente lectura

    // Switch encoder
    if (digitalRead(SW) == LOW) {
        Serial.println("¡Botón del encoder presionado! Reiniciando...");
        contador = 0;
        actualizarLCD();
        delay(300);  // debounce
    }

    // Switch -5 
    if (digitalRead(SW_SUB5) == LOW) {
        contador -= 5;
        Serial.println("Restado 5");
        actualizarLCD();
        delay(300);  // debounce
    }

    // Switch +5
    if (digitalRead(SW_ADD5) == LOW) {
        contador += 5;
        Serial.println("Sumado 5");
        actualizarLCD();
        delay(300);  // debounce
    }

    // Switch +10
    if (digitalRead(SW_ADD10) == LOW) {
        contador += 10;
        Serial.println("Sumado 10");
        actualizarLCD();
        delay(300);  // debounce
    }
}

// Función para actualizar el LCD con el valor del contador
void actualizarLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Contador: ");
    lcd.setCursor(0, 1);
    lcd.print(contador);

    Serial.print("Valor en LCD: ");
    Serial.println(contador);
}
