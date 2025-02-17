#define CLK 32  // Pin del encoder (CLK)
#define DT 33   // Pin del encoder (DT)
#define SW 25   // Botón del encoder (SW)

// Definimos los pines de los botones adicionales
#define BTN_SUB5 26  // Botón que resta 5
#define BTN_ADD5 27  // Botón que suma 5
#define BTN_ADD10 14 // Botón que suma 10

volatile int contador = 0;  
int lastStateCLK;

void setup() {
    Serial.begin(115200);

    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);
    
    pinMode(BTN_SUB5, INPUT_PULLUP);
    pinMode(BTN_ADD5, INPUT_PULLUP);
    pinMode(BTN_ADD10, INPUT_PULLUP);

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

        Serial.print("Valor: ");
        Serial.println(contador);
        delay(5);  // Pequeña pausa para evitar rebotes
    }

    lastStateCLK = currentStateCLK;  // Guarda el estado para la siguiente lectura

    // Verifica si se presiona el botón del encoder
    if (digitalRead(SW) == LOW) {
        Serial.println("¡Botón del encoder presionado! Reiniciando...");
        contador = 0;
        delay(300);  // Anti-rebote
    }

    // Botón que resta 5
    if (digitalRead(BTN_SUB5) == LOW) {
        contador -= 5;
        Serial.println("Restado 5");
        Serial.print("Valor: ");
        Serial.println(contador);
        delay(300);  // Anti-rebote
    }

    // Botón que suma 5
    if (digitalRead(BTN_ADD5) == LOW) {
        contador += 5;
        Serial.println("Sumado 5");
        Serial.print("Valor: ");
        Serial.println(contador);
        delay(300);  // Anti-rebote
    }

    // Botón que suma 10
    if (digitalRead(BTN_ADD10) == LOW) {
        contador += 10;
        Serial.println("Sumado 10");
        Serial.print("Valor: ");
        Serial.println(contador);
        delay(300);  // Anti-rebote
    }
}
