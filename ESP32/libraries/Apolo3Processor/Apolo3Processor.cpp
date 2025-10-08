#include "Apolo3Processor.h"
#include <string.h>  // Para memset y memcpy

// ============================
// Constructor
// ============================
Apolo3Processor::Apolo3Processor() {
  // Inicializamos todos los arrays de lectura en 0
  memset(softpots, 0, sizeof(softpots));
  memset(fsrs, 0, sizeof(fsrs));
  memset(dotfsrs, 0, sizeof(dotfsrs));
  memset(lastSoftpots, 0, sizeof(lastSoftpots));
  memset(lastFsrs, 0, sizeof(lastFsrs));
  memset(lastDotfsrs, 0, sizeof(lastDotfsrs));

  // Creamos los objetos StringHandler con su índice correspondiente
  for (int i = 0; i < NUM_STRINGS; ++i) {
    strings[i] = std::make_unique<StringHandler>(i);
  }
}

// ============================
// update(): recibe valores crudos
// ============================
void Apolo3Processor::update(const std::vector<uint16_t>& raw) {
  uint32_t now = millis(); // Usado para calcular velocidad

  for (int i = 0; i < NUM_STRINGS; ++i) {
    softpots[i] = raw[i * 3 + 0];
    fsrs[i]     = raw[i * 3 + 1];
    dotfsrs[i]  = raw[i * 3 + 2];

    strings[i]->update(softpots[i], dotfsrs[i], fsrs[i], now);
  }
}

// ============================
// printDebug(): salida por Serial
// ============================
void Apolo3Processor::printDebug(uint8_t address) const {
  Serial.printf("[Apolo3] Dirección 0x%02X\n", address);
  Serial.println(F("====== SENSOR DEBUG ======"));
  Serial.println(F("| # | SoftPot |   FSR   | DotFSR |"));
  Serial.println(F("|---|---------|---------|--------|"));
  for (int i = 0; i < NUM_STRINGS; ++i) {
    Serial.printf("| %d |  %5d  |  %5d  |  %5d |\n", i + 1, softpots[i], fsrs[i], dotfsrs[i]);
  }
  Serial.println(F("==========================\n"));
}

// ============================
// hasChanged(): detecta cambios
// ============================
bool Apolo3Processor::hasChanged(uint16_t threshold) const {
  for (int i = 0; i < NUM_STRINGS; ++i) {
    if (abs(softpots[i] - lastSoftpots[i]) > threshold) return true;
    if (abs(fsrs[i]     - lastFsrs[i])     > threshold) return true;
    if (abs(dotfsrs[i]  - lastDotfsrs[i])  > threshold) return true;
  }
  return false;
}

// ============================
// markAsSent(): guarda estado actual como último
// ============================
void Apolo3Processor::markAsSent() {
  memcpy(lastSoftpots, softpots, sizeof(softpots));
  memcpy(lastFsrs, fsrs, sizeof(fsrs));
  memcpy(lastDotfsrs, dotfsrs, sizeof(dotfsrs));
}

// ============================
// sendOSC(): construye y envía 3 paquetes OSC (uno por cuerda)
// ============================

void Apolo3Processor::sendOSCMusical(IPAddress destIP, uint16_t destPort, WiFiUDP& udp) {
  for (int i = 0; i < NUM_STRINGS; ++i) {
    String path = "/3stringController/String" + String(i + 1) + "/musical";

    OSCMessage msg(path.c_str());
    msg.add((int)strings[i]->getNote());
    msg.add((int)strings[i]->getVelocity());
    msg.add((int)strings[i]->getModulation());
    msg.add((int)strings[i]->getPitchBend());


    udp.beginPacket(destIP, destPort);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
  }
}

void Apolo3Processor::sendOSCRaw(const IPAddress& destIP, uint16_t destPort, WiFiUDP& udp) {
	for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
    	    if (!strings[i]) continue;

    	    String basePath = "/3stringController/String" + String(i + 1);

            OSCMessage rawMsg(basePath.c_str());
            rawMsg.add((intOSC_t)strings[i]->getRawPosition());
            rawMsg.add((intOSC_t)strings[i]->getRawModulation());
            rawMsg.add((intOSC_t)strings[i]->getRawVelocity());

            udp.beginPacket(destIP, destPort);
            rawMsg.send(udp);
            udp.endPacket();
            rawMsg.empty();
}

}

// ============================
// Acceso a StringHandler individual
// ============================
StringHandler* Apolo3Processor::getString(uint8_t index) const {
  if (index >= NUM_STRINGS) return nullptr;
  return strings[index].get();
}

// ============================
// Configuración externa de cada cuerda
// ============================
void Apolo3Processor::setStringConfig(uint8_t index,
                                      uint8_t midiChannel,
                                      uint8_t noteStart,
                                      uint8_t noteRange,
                                      bool trasteado,
                                      bool pitchBendEnabled,
                                      uint8_t pitchBendSemitones) {
  if (index >= NUM_STRINGS || !strings[index]) return;
  strings[index]->setConfig(midiChannel, noteStart, noteRange, trasteado, pitchBendEnabled, pitchBendSemitones);
}

// ============================
// Debug musical completo por Serial
// ============================
void Apolo3Processor::printMusicalState() const {
  for (int i = 0; i < NUM_STRINGS; ++i) {
    auto* s = strings[i].get();
    Serial.printf("Cuerda %d | Nota: %3d | Bend: %5d | Vel: %3d | Mod: %3d | ",
                  i + 1,
                  s->getNote(),
                  s->getPitchBend(),
                  s->getVelocity(),
                  s->getModulation());

    if (s->hasNoteOnEvent()) Serial.print("⏺ NoteOn");
    else if (s->hasNoteOffEvent()) Serial.print("⏹ NoteOff");
    else Serial.print("-");
    Serial.println();
  }
}

