#include "OrpheusProcessor.h"
#include <string.h>

OrpheusProcessor::OrpheusProcessor() {
  memset(values, 0, sizeof(values));
  memset(lastValues, 0, sizeof(lastValues));
}

void OrpheusProcessor::update(const std::vector<uint16_t>& raw) {
  for (int i = 0; i < 7; i++) {
    values[i] = raw[i];
  }
}

void OrpheusProcessor::printDebug(uint8_t address) const {
  Serial.printf("[Orpheus] Dirección 0x%02X\n", address);

  Serial.print("┌");
  for (int i = 0; i < 7; i++) {
    Serial.print("───────");
    if (i < 6) Serial.print("┬");
  }
  Serial.println("┐");

  Serial.print("│");
  for (int i = 0; i < 7; i++) {
    Serial.printf(" S%d    │", i);
  }
  Serial.println();

  Serial.print("├");
  for (int i = 0; i < 7; i++) {
    Serial.print("───────");
    if (i < 6) Serial.print("┼");
  }
  Serial.println("┤");

  Serial.print("│");
  for (int i = 0; i < 7; i++) {
    Serial.printf(" %4d  │", values[i]);
  }
  Serial.println();

  Serial.print("└");
  for (int i = 0; i < 7; i++) {
    Serial.print("───────");
    if (i < 6) Serial.print("┴");
  }
  Serial.println("┘\n");
}

bool OrpheusProcessor::hasChanged(uint16_t threshold) const {
  for (int i = 0; i < 7; i++) {
    if (abs((int)values[i] - (int)lastValues[i]) > threshold) return true;
  }
  return false;
}

void OrpheusProcessor::markAsSent() {
  memcpy(lastValues, values, sizeof(values));
}
