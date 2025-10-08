/************************************************************
 * OutSink.h  (versión genérica, neutral a protocolo)
 * Define un "contrato" de salida basado en eventos abstractos.
 * Cada implementación (MIDI DIN/USB, OSC, CV/Gate) traduce
 * estos eventos a su transporte/formato concreto.
 ************************************************************/
#pragma once
#include <Arduino.h>

// Tipo de evento genérico (neutral)
enum class OutEventType : uint8_t {
  TriggerOn,    // nota/gate ON   (id = nota o gate-id, value = intensidad/vel)
  TriggerOff,   // nota/gate OFF  (id = nota o gate-id, value = 0 o release)
  Continuous,   // parámetro continuo (id = param-id, value = 0..16383)
  Pitch,        // pitch/afinación 14 bits (value = 0..16383, centro=8192)
  // Futuro:
  // Program,   // cambio de programa (id = bank/program, value opcional)
  // Aftertouch // presión canal o polifónica
};

// Evento genérico
// Convenciones:
//  - ch: 1..16 o el "canal lógico" que uses (para CV podría mapear a fila/salida)
//  - id: identifica la "cosa": nota, número de parámetro, índice de gate...
//  - value: resolución 0..16383 (los sinks que solo admiten 7 bits harán clamp)
struct OutEvent {
  OutEventType type;
  uint8_t  ch;      // canal lógico (1..16 típico; libre para CV/OSC)
  uint16_t id;      // nota / param-id / gate-id
  uint16_t value;   // 0..16383 (usar 0..127 si no necesitas 14 bits)
};

// Interfaz común de salida
class OutSink {
public:
  virtual ~OutSink() {}
  virtual void handle(const OutEvent& e) = 0;
};

// ---------- Helpers (opcionales) para crear eventos ----------

// Nota/gate ON: id=nota o gate-id; vel/intensidad en 0..127 (o 14 bits si quieres)
inline OutEvent evTriggerOn(uint8_t ch, uint16_t id, uint16_t vel = 127) {
  return OutEvent{ OutEventType::TriggerOn, ch, id, vel };
}

// Nota/gate OFF: id=nota o gate-id; value=0 por defecto
inline OutEvent evTriggerOff(uint8_t ch, uint16_t id, uint16_t val = 0) {
  return OutEvent{ OutEventType::TriggerOff, ch, id, val };
}

// Param continuo: id=param-id; value 0..16383 (0..127 si usas 7 bits)
inline OutEvent evContinuous(uint8_t ch, uint16_t paramId, uint16_t value) {
  return OutEvent{ OutEventType::Continuous, ch, paramId, value };
}

// Pitch 14 bits (0..16383, centro=8192)
inline OutEvent evPitch(uint8_t ch, uint16_t bend14) {
  return OutEvent{ OutEventType::Pitch, ch, /*id*/0, bend14 };
}
