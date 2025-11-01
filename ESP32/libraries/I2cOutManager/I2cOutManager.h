#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "OutSink.h"

// Si tu OutSink/OutEvent están en otro header, ajusta el include anterior.
// Interfaz mínima esperada:
//   struct OutEvent { ... };
//   struct OutSink { virtual void handle(const OutEvent& e) = 0; };

class I2cOutManager {
public:
 
  explicit I2cOutManager(TwoWire& busRef) : bus(busRef) {}

  I2cOutManager() : bus(Wire) {}

  void addSink(OutSink* s) {
    if (!s) return;
    sinks.push_back(s);
  }

  // Encaminador genérico de eventos de salida
  void handle(const OutEvent& e) {
    for (auto* s : sinks) {
      if (s) s->handle(e);
    }
  }

  // Si en tu proyecto inicializas el bus aquí, puedes exponer un begin opcional:
  void begin(uint32_t freq = 400000) {
    // Ojo: si usas un TwoWire distinto de 'Wire', asegúrate de haber llamado a .begin() antes desde fuera
    // Aquí NO llamamos a bus.begin(SDA, SCL) porque tus pines pueden variar por placa
    bus.setClock(freq);
  }

  TwoWire& getWire() { return bus; }

private:
  TwoWire& bus;
  std::vector<OutSink*> sinks;
};
