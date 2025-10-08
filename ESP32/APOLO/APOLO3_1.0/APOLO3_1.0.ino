/**
 * @file APOLO3_1.0.ino
 * @brief Firmware del módulo sensor Apolo3.
 *
 * Este sketch implementa un módulo esclavo que lee datos de 9 sensores analógicos
 * (3 SoftPot, 3 FSR y 3 DotFSR), y transmite esta información estructurada al controlador
 * maestro mediante el protocolo I2C. Forma parte del sistema modular de control musical.
 *
 * @author Roberto García Cuchillo
 * @date Junio 2025
 * @version 1.0
 */

// ======================== LIBRERÍAS ========================

#include <Arduino.h>     ///< Librería principal de Arduino
#include <Wire.h>        ///< Librería para comunicación I2C

#include "SensorManager.h"   ///< Gestiona múltiples sensores
#include "ModuleTypes.h"     ///< Define la estructura Apolo3I2CData y el tipo de módulo

// ==================== CONFIGURACIÓN I2C ====================

#define I2C_ADDRESS 0x0D  ///< Dirección I2C del módulo Apolo3 (0x0D = 13 decimal)
#define SDA_PIN 21        ///< Pin SDA (no usado en esta implementación)
#define SCL_PIN 22        ///< Pin SCL (no usado en esta implementación)



// =================== OBJETOS PRINCIPALES ===================

SensorManager manager;      ///< Instancia que gestiona todos los sensores del módulo
Apolo3I2CData i2cData;      ///< Estructura que guarda los datos a enviar por I2C


// ================== FLAGS DE DEPURACIÓN ==================

#define DEBUG_I2C           ///< Habilita mensajes de depuración relacionados con I2C
#define DEBUG_SENSORS       ///< Habilita impresión de datos crudos de sensores por Serial


/**
 * @brief updateI2CBuffer(): Actualiza la estructura de datos que se enviará por I2C.
 *
 * Esta función obtiene un vector con las lecturas de todos los sensores gestionados por
 * el SensorManager. Se espera un vector de tamaño 9, ordenado del siguiente modo:
 * - Índices 0 a 2 → SoftPot (posición)
 * - Índices 3 a 5 → FSR (presión continua)
 * - Índices 6 a 8 → DotFSR (presión puntual)
 *
 * Estas lecturas se copian en la estructura `i2cData`, que será enviada al maestro cuando
 * este lo solicite a través del bus I2C.
 *
 * Si está definida la bandera DEBUG_SENSORS, se imprimen por consola los valores leídos
 * cuando hay un cambio significativo respecto a la última lectura.
 */
void updateI2CBuffer() {
  // Obtener todos los valores de sensores en el orden en que fueron añadidos
  const std::vector<uint16_t>& values = manager.getValues();

  // Asignar los valores a la estructura i2cData según el orden fijo
  for (int i = 0; i < 3; i++) {
    i2cData.softpots[i] = values[i];       // SoftPot en posiciones 0,1,2
    i2cData.fsrs[i]     = values[i + 3];   // FSR en posiciones 3,4,5
    i2cData.dotfsrs[i]  = values[i + 6];   // DotFSR en posiciones 6,7,8
  }

  // Depuración opcional por consola
#ifdef DEBUG_SENSORS
    static uint32_t lastDebugPrint = 0;
    static uint16_t lastValues[9] = {0};  // Últimos valores conocidos
    const uint16_t CHANGE_THRESHOLD = 8;  // Umbral de cambio mínimo

    uint32_t now = millis();
    if (now - lastDebugPrint > 300) {  // Muestra cada 300ms como máximo
        lastDebugPrint = now;

        if (values.size() == 9) {
            bool hasChanged = false;
            for (size_t i = 0; i < 9; ++i) {
                if (abs((int)values[i] - (int)lastValues[i]) > CHANGE_THRESHOLD) {
                    hasChanged = true;
                    break;
                }
            }

            if (hasChanged) {
                for (size_t i = 0; i < 9; ++i) lastValues[i] = values[i];

                Serial.println(F("====== SENSOR DEBUG ======"));
                Serial.println(F("| # | SoftPot |   FSR   | DotFSR |"));
                Serial.println(F("|---|---------|---------|--------|"));
                for (int i = 0; i < 3; ++i) {
                    uint16_t softpot = values[i];
                    uint16_t fsr     = values[i + 3];
                    uint16_t dotfsr  = values[i + 6];
                    Serial.printf("| %d |  %5d  |  %5d  |  %5d |\n", i + 1, softpot, fsr, dotfsr);
                }
                Serial.println(F("==========================\n"));
            }
        }
    }
#endif

}


/**
 * @brief onI2CRequest(): Envía los datos del buffer i2cData al maestro I2C cuando se realiza una petición.
 *
 * Esta función es registrada como callback de interrupción mediante `Wire.onRequest()` y se
 * ejecuta automáticamente cuando el maestro solicita datos al esclavo.
 */
void onI2CRequest() {
  #ifdef DEBUG_I2C
  Serial.println(F("[I2C] Petición recibida: enviando datos Apolo3..."));
  #endif

  // Enviar los datos binarios como paquete al maestro
  Wire.write((uint8_t*)&i2cData, sizeof(i2cData));
}

/**
 * @brief Inicialización del módulo Apolo3.
 *
 * Configura la comunicación I2C como esclavo con dirección fija. Registra la función de callback
 * para responder al maestro y realiza la configuración inicial de sensores mediante SensorManager.
 */
void setup() {
  Serial.begin(115200);       // Inicia puerto serie para depuración
  delay(100);                 // Pequeña pausa para estabilidad

  Wire.begin(I2C_ADDRESS);    // Inicia como esclavo I2C con dirección fija
  Wire.onRequest(onI2CRequest);  // Define función a ejecutar cuando el maestro solicita datos

  // === Inicialización de sensores ===
  manager.addSensor(SensorType::SoftPot, 32);
  manager.addSensor(SensorType::SoftPot, 35);
  manager.addSensor(SensorType::SoftPot, 34);

  manager.addSensor(SensorType::FSR, 33);
  manager.addSensor(SensorType::FSR, 25);
  manager.addSensor(SensorType::FSR, 26);

  manager.addSensor(SensorType::DotFSR, 12);
  manager.addSensor(SensorType::DotFSR, 14);
  manager.addSensor(SensorType::DotFSR, 27);

  manager.begin();

    Serial.println("[SLAVE] Sensores inicializados.");
    Serial.print("[SLAVE] Dirección I2C: 0x");
    Serial.println(I2C_ADDRESS, HEX);

}

/**
 * @brief Bucle principal del firmware del módulo Apolo3.
 *
 * En cada iteración se actualizan las lecturas de los sensores y se empaquetan los datos
 * en la estructura i2cData, lista para ser enviada al maestro si este la solicita.
 */
void loop() {
  manager.update();        // Actualiza todas las lecturas de sensores
  updateI2CBuffer();       // Empaqueta los datos actualizados para el maestro
  delay(10);               // Pequeño retardo (evita lecturas excesivas)
}
