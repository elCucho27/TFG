#ifndef APOLO3_PROCESSOR_H
#define APOLO3_PROCESSOR_H

#include "SensorProcessor.h"     // Clase base abstracta
#include "StringHandler.h"       // Lógica por cuerda
#include <WiFiUdp.h>             // Requerido por OSC
#include <OSCMessage.h>          // Para crear mensajes OSC
#include <memory>                // Para usar std::unique_ptr

#define NUM_STRINGS 3            // Número fijo de cuerdas para Apolo3

class Apolo3Processor : public SensorProcessor {
public:
  Apolo3Processor();

  void update(const std::vector<uint16_t>& raw) override;
  void printDebug(uint8_t address) const override;
  bool hasChanged(uint16_t threshold) const override;
  void markAsSent() override;

  // Métodos útiles para pruebas y configuración
  StringHandler* getString(uint8_t index) const;
  void setStringConfig(uint8_t index,
                       uint8_t midiChannel,
                       uint8_t noteStart,
                       uint8_t noteRange,
                       bool trasteado,
                       bool pitchBendEnabled,
                       uint8_t pitchBendSemitones);
  void printMusicalState() const;

  // Envío separado de datos interpretados y crudos
  void sendOSCMusical(IPAddress destIP, uint16_t destPort, WiFiUDP& udp);
  void sendOSCRaw(const IPAddress& destIP, uint16_t destPort, WiFiUDP& udp);

private:
  uint16_t softpots[NUM_STRINGS];
  uint16_t fsrs[NUM_STRINGS];
  uint16_t dotfsrs[NUM_STRINGS];

  uint16_t lastSoftpots[NUM_STRINGS];
  uint16_t lastFsrs[NUM_STRINGS];
  uint16_t lastDotfsrs[NUM_STRINGS];

  std::unique_ptr<StringHandler> strings[NUM_STRINGS];
};

#endif
