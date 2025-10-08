#include <Wire.h>
#include "Apolo3Processor.h"
#include "Adafruit_TinyUSB.h"
#include "MIDIHandler.h"

#define APOLLO3_ADDRESS 0x0D
#define POLL_INTERVAL_MS 30

struct Apolo3I2CData {
  uint16_t softpots[3];
  uint16_t fsrs[3];
  uint16_t dotfsrs[3];
};

Apolo3I2CData sensorData;
Apolo3Processor apolo3;
MIDIHandler midiHandler;

uint32_t lastPollTime = 0;
bool noteActive[3] = { false, false, false };

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // deja tiempo a TinyUSB

  // Inicializa el bus I2C (SDA=8, SCL=9 por defecto en ESP32-S3)
  Wire.begin();

  // Inicializa el MIDI USB
  midiHandler.begin();
  Serial.println("[SETUP] MIDIHandler iniciado.");

  // Configura Apolo3Processor con parámetros para cada cuerda
  for (uint8_t i = 0; i < 3; ++i) {
    apolo3.setStringConfig(i, i + 1, 60, 20, false, true, 2);
  }

  Serial.println("🎼 CONTROLADOR_APOLO3_MIDI_TEST listo");
}

void loop() {
  if (millis() - lastPollTime >= POLL_INTERVAL_MS) {
    lastPollTime = millis();

    if (requestApolo3Data()) {
      std::vector<uint16_t> raw(9);
      for (int i = 0; i < 3; ++i) {
        raw[i * 3 + 0] = sensorData.softpots[i];
        raw[i * 3 + 1] = sensorData.fsrs[i];
        raw[i * 3 + 2] = sensorData.dotfsrs[i];
      }

      apolo3.update(raw);
      handleMIDIOutput();
      apolo3.printMusicalState();
    }
  }
}

bool requestApolo3Data() {
  Wire.requestFrom(APOLLO3_ADDRESS, sizeof(Apolo3I2CData));
  if (Wire.available() == sizeof(Apolo3I2CData)) {
    Wire.readBytes((char*)&sensorData, sizeof(Apolo3I2CData));
    return true;
  } else {
    Serial.println("⚠️ No se recibieron 18 bytes desde Apolo3.");
    return false;
  }
}

void handleMIDIOutput() {
  if (!TinyUSBDevice.mounted()) {
    Serial.println("⚠ TinyUSB NO montado → NO envío datos MIDI.");
    return;
  }

  for (uint8_t i = 0; i < 3; ++i) {
    StringHandler* s = apolo3.getString(i);
    uint8_t ch = i + 1;

    if (s != nullptr) {
      if (s->hasNoteOnEvent()) {
        midiHandler.sendNoteOn(s->getNote(), s->getVelocity(), ch);
        noteActive[i] = true;
        Serial.printf("[MIDI] NoteOn - String %d - Note %d Vel %d\n", i, s->getNote(), s->getVelocity());
      }
      else if (s->hasNoteOffEvent()) {
        midiHandler.sendNoteOff(s->getNote(), 0, ch);
        noteActive[i] = false;
        Serial.printf("[MIDI] NoteOff - String %d - Note %d\n", i, s->getNote());
      }

      if (noteActive[i]) {
        midiHandler.sendPitchBend(s->getPitchBend(), ch);
        Serial.printf("[MIDI] PitchBend - String %d - PB %d\n", i, s->getPitchBend());
      }

      if (s->hasModulationEvent()) {
        midiHandler.sendControlChange(1, s->getModulation(), ch);
        Serial.printf("[MIDI] CC1 Modulation - String %d - Mod %d\n", i, s->getModulation());
      }
    } else {
      Serial.printf("⚠ ERROR: StringHandler %d es NULL\n", i);
    }
  }
}
