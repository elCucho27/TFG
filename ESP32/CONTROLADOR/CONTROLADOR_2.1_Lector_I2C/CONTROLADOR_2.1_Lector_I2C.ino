#include <Wire.h> // Incluimos la librería estándar para comunicación I2C

// Dirección I2C del esclavo (sensor)
#define SLAVE_ADDR 0x10

// Tamaño máximo de datos que esperamos recibir (3 bytes por sensor)
#define MAX_BYTES 30

// Pines I2C personalizados para la ESP32-S3
#define SDA_PIN 8
#define SCL_PIN 9

// Función para escanear el bus I2C y detectar dispositivos conectados
void escanearI2C() {
  Serial.println("🔍 Escaneando dispositivos I2C...");

  int encontrados = 0;

  // Iteramos sobre todas las direcciones posibles (7 bits)
  for (uint8_t direccion = 1; direccion < 127; direccion++) {
    Wire.beginTransmission(direccion);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("✅ Dispositivo encontrado en 0x");
      Serial.println(direccion, HEX);
      encontrados++;
    } else if (error == 4) {
      Serial.print("⚠ Error desconocido al consultar 0x");
      Serial.println(direccion, HEX);
    }
  }

  if (encontrados == 0) {
    Serial.println("❌ No se encontraron dispositivos I2C.");
  } else {
    Serial.println("✔ Escaneo completo.");
  }

  Serial.println();
}


void setup() {
  Serial.begin(115200); // Iniciamos la comunicación serie para depuración
  delay(500);

  // Inicializamos el bus I2C como maestro, con pines personalizados
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("🔌 Maestro I2C iniciado en SDA=8, SCL=9");

  delay(3000); // Damos tiempo a que el esclavo se inicie completamente

  // Escaneamos el bus I2C para ver si el esclavo está presente
  escanearI2C();
}

void loop() {
  // Solicitamos al esclavo hasta MAX_BYTES (ej. 3 sensores = 9 bytes)
  Wire.requestFrom(SLAVE_ADDR, MAX_BYTES);

  int disponibles = Wire.available(); // Cuántos bytes llegaron

  if (disponibles % 3 != 0) {
    Serial.println("❌ Error: Paquete no alineado (esperado múltiplo de 3)");
    while (Wire.available()) Wire.read(); // Limpiamos el buffer
    delay(500);
    return;
  }

  // Procesamos los datos recibidos: ID (1 byte) + Valor (2 bytes)
  while (Wire.available() >= 3) {
    uint8_t id = Wire.read();       // Primer byte: ID del sensor
    uint8_t msb = Wire.read();      // Segundo byte: parte alta del valor
    uint8_t lsb = Wire.read();      // Tercer byte: parte baja del valor

    uint16_t value = (msb << 8) | lsb;      // Reconstruimos el valor (12 bits)
    float normalized = value / 4095.0;      // Lo convertimos a [0.0 - 1.0]

    // Mostramos en consola el valor recibido
    Serial.print("📡 Sensor ID ");
    Serial.print(id);
    Serial.print(" → Valor: ");
    Serial.print(value);
    Serial.print(" (");
    Serial.print(normalized, 3);
    Serial.println(")");
  }

  delay(100); // Esperamos 100 ms antes de la próxima lectura (~10 Hz)
}
