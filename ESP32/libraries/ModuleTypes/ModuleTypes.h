#ifndef MODULE_TYPES_H
#define MODULE_TYPES_H

#include <Wire.h>
#include <vector>

enum ModuleType {
  UNKNOWN,
  SENSOR_APOLO3,
  SENSOR_ORPHEUS
};

// Estructura de datos para sensor Apolo3
struct Apolo3I2CData {
  uint16_t softpots[3];
  uint16_t fsrs[3];
  uint16_t dotfsrs[3];
};

// Estructura de datos para sensor Orpheus
struct OrpheusI2CData {
  uint16_t values[7];
};

// Función para obtener nombre de tipo
inline const char* getModuleTypeName(ModuleType type) {
  switch (type) {
    case SENSOR_APOLO3: return "Apolo3";
    case SENSOR_ORPHEUS: return "Orpheus";
    default: return "Unknown";
  }
}

// Función para pedir datos crudos según tipo
inline bool requestSensorRaw(uint8_t address, ModuleType type, std::vector<uint16_t>& raw) {
  switch (type) {
    case SENSOR_APOLO3: {
      Apolo3I2CData data;
      Wire.requestFrom(address, sizeof(data));
      if (Wire.available() != sizeof(data)) return false;
      Wire.readBytes((uint8_t*)&data, sizeof(data));

      std::vector<uint16_t> tmp = {
        data.softpots[0], data.fsrs[0], data.dotfsrs[0],
        data.softpots[1], data.fsrs[1], data.dotfsrs[1],
        data.softpots[2], data.fsrs[2], data.dotfsrs[2]
      };
      raw = tmp;
      return true;
    }

    case SENSOR_ORPHEUS: {
      OrpheusI2CData data;
      Wire.requestFrom(address, sizeof(data));
      if (Wire.available() != sizeof(data)) return false;
      Wire.readBytes((uint8_t*)&data, sizeof(data));

      std::vector<uint16_t> tmp(data.values, data.values + 7);
      raw = tmp;
      return true;
    }

    default:
      return false;
  }
}

#endif
