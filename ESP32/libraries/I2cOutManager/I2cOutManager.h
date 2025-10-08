/************************************************************
 * I2cOutManager.h
 * Router/fan-out para el BUS DE SALIDAS.
 * Mantiene una lista de “sinks” (implementaciones de OutSink)
 * y expone helpers para emitir eventos a TODOS los sinks activos.
 *
 * NOTA: aunque se llame I2cOutManager, no depende de Wire/Wire1.
 * El nombre hace referencia a que forma parte del “lado OUT”.
 * Cada sink decide cómo transporta (I2C, USB, UDP, GPIO...).
 ************************************************************/
#pragma once
#include <Arduino.h>
#include <vector>
#include "OutSink.h"

class I2cOutManager {
public:
  I2cOutManager() = default;

  // Registrar/retirar salidas
  void addSink(OutSink* s) {
    if (!s) return;
    sinks.push_back(s);
  }

  void clearSinks() {
    sinks.clear();
  }

  // --------- API de alto nivel (fan-out) ----------
  inline void noteOn (uint8_t ch, uint8_t note, uint8_t vel) {
    for (auto* s : sinks) s->noteOn(ch, note, vel);
  }
  inline void noteOff(uint8_t ch, uint8_t note, uint8_t vel) {
    for (auto* s : sinks) s->noteOff(ch, note, vel);
  }
  inline void cc     (uint8_t ch, uint8_t num,  uint8_t val) {
    for (auto* s : sinks) s->cc(ch, num, val);
  }
  inline void pb     (uint8_t ch, uint16_t bend14) {
    for (auto* s : sinks) s->pb(ch, bend14);
  }

private:
  std::vector<OutSink*> sinks;
};
