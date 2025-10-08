#include <Wire.h>
#include "I2cInManager.h"   // Esto incluye indirectamente todo lo necesario

bool DEBUG_GENERAL = true;
bool DEBUG_APOLO3 = true;
bool DEBUG_ORPHEUS = true;

const uint32_t READ_INTERVAL_MS = 30;
const uint16_t CHANGE_THRESHOLD = 30;

I2cInManager manager;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(500);

  if (DEBUG_GENERAL) Serial.println("🎛 Maestro I2C iniciado");
  manager.scan();
  delay(3000);
}

void loop() {
  uint32_t now = millis();
  manager.update(now, READ_INTERVAL_MS, CHANGE_THRESHOLD, DEBUG_GENERAL);
  delay(1);
}
