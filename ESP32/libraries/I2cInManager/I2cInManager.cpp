#include "I2cInManager.h"

bool I2cInManager::isAvailable(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

ModuleType I2cInManager::detectType(uint8_t address) {
  if (address >= 0x0D && address <= 0x1B) return SENSOR_APOLO3;
  if (address >= 0x1C && address <= 0x2B) return SENSOR_ORPHEUS;
  return UNKNOWN;
}

void I2cInManager::scan() {
  count = 0;
  for (uint8_t addr = 1; addr < 127 && count < MAX_MODULES; addr++) {
    if (isAvailable(addr)) {
      ModuleType type = detectType(addr);

      modules[count].address = addr;
      modules[count].type = type;
      modules[count].isConnected = true;
      modules[count].justReconnected = false;
      modules[count].failCount = 0;
      modules[count].lastReadTime = 0;

      switch (type) {
        case SENSOR_APOLO3:
          modules[count].processor = new Apolo3Processor();
          break;
        case SENSOR_ORPHEUS:
          modules[count].processor = new OrpheusProcessor();
          break;
        default:
          modules[count].processor = nullptr;
      }

      Serial.printf("✅ 0x%02X → %s\n", addr, getModuleTypeName(type));
      count++;
    }
  }

  Serial.println("✔ Escaneo finalizado\n");
}

void I2cInManager::update(uint32_t now, uint32_t intervalMs, uint16_t threshold, bool debugGeneral) {
  for (uint8_t i = 0; i < count; i++) {
    SensorModule& m = modules[i];
    if (now - m.lastReadTime < intervalMs) continue;
    m.lastReadTime = now;

    std::vector<uint16_t> raw;
    if (!requestSensorRaw(m.address, m.type, raw)) {
      m.failCount++;
      m.isConnected = false;
      if (debugGeneral) {
        Serial.printf("[%s] Dirección 0x%02X → ❌ Desconectado (fallo %d)\n", getModuleTypeName(m.type), m.address, m.failCount);
      }
      continue;
    }

    m.failCount = 0;

    // Primera lectura tras reconexión → ignorar datos y marcar como esperando confirmación
    if (!m.isConnected) {
      m.isConnected = true;
      m.justReconnected = true;
      if (debugGeneral) {
        Serial.printf("[%s] Dirección 0x%02X → 🟡 Reconectado (esperando confirmación)\n", getModuleTypeName(m.type), m.address);
      }
      continue;
    }

    // Segunda lectura válida → confirmar reconexión estable
    if (m.justReconnected) {
      m.justReconnected = false;
      if (debugGeneral) {
        Serial.printf("[%s] Dirección 0x%02X → ✅ Reconexión confirmada\n", getModuleTypeName(m.type), m.address);
      }
    }

    if (!m.processor) continue;

    m.processor->update(raw);

    if (m.processor->hasChanged(threshold)) {
      m.processor->printDebug(m.address);

        if (onChanged) { //CallBack
           onChanged(m.address, m.type, *m.processor);
         }
      m.processor->markAsSent();
    }
  }
}
