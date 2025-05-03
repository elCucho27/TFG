#ifndef SENSOR_DATA_TYPES_H
#define SENSOR_DATA_TYPES_H

#include <stdint.h>

// Número máximo de sensores por tipo (ajustable)
#define MAX_SOFTPOTS 3
#define MAX_FSRS     3

// Estructura que empaqueta todos los datos a enviar por I2C
struct SensorI2CData {
  uint16_t softPotValues[MAX_SOFTPOTS]; // Valores filtrados de SoftPots (posición)
  uint16_t fsrValues[MAX_FSRS];         // Valores filtrados de FSRs (presión)
};

#endif // SENSOR_DATA_TYPES_H
