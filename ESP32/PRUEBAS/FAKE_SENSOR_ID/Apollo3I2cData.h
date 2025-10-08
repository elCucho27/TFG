#ifndef APOLLO3_I2C_DATA_H
#define APOLLO3_I2C_DATA_H

#include <Arduino.h>

// Estructura que representa un módulo sensor Apollo3 (3 cuerdas)
struct Apollo3I2CData {
  uint16_t softpots[3];   // Posición de cada cuerda
  uint16_t fsrs[3];       // Presión continua
  uint16_t dotfsrs[3];    // Presión tipo on/off
};

#endif
