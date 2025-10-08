#ifndef I2C_IN_MANAGER_H
#define I2C_IN_MANAGER_H

#include <Wire.h>
#include <vector>
#include <functional>     
#include "ModuleTypes.h"
#include "SensorProcessor.h"
#include "Apolo3Processor.h"
#include "OrpheusProcessor.h"

// ============================================
// Estructura que representa un módulo sensor conectado por I2C
// ============================================
struct SensorModule {
  uint8_t address;                       // Dirección I2C del esclavo
  ModuleType type;                       // Tipo del sensor (Apolo3, Orpheus…)
  bool isConnected = false;              // Estado de conexión actual
  bool justReconnected = false;          // Flag: reconectado recientemente (paquete de validación pendiente)
  uint8_t failCount = 0;                 // Cuántos intentos de lectura han fallado consecutivamente
  uint32_t lastReadTime = 0;             // Último timestamp de lectura (millis)
  uint16_t threshold = 50;               // Umbral de cambio significativo
  SensorProcessor* processor = nullptr;  // Procesador que interpreta los datos
};

// ============================================
// Clase que gestiona todos los sensores conectados al bus I2C
// ============================================
class I2cInManager {
public:
  void scan();  // Escanea el bus I2C y detecta módulos
  void update(uint32_t now, uint32_t readInterval, uint16_t threshold, bool debugGeneral);

  void setOnProcessorChanged(std::function<void(uint8_t, ModuleType, SensorProcessor&)> cb) {   // Registrar callback cuando un módulo “cambia” Firma: (address I2C, tipo de módulo, referencia al processor)
  onChanged = std::move(cb);
}

private:
  static const uint8_t MAX_MODULES = 20;
  SensorModule modules[MAX_MODULES];
  uint8_t count = 0;

  // Métodos internos
  std::function<void(uint8_t, ModuleType, SensorProcessor&)> onChanged;   //CallBack
  bool isAvailable(uint8_t address);           // Verifica si hay respuesta en esa dirección
  ModuleType detectType(uint8_t address);      // Asigna tipo en función de dirección
};

#endif
