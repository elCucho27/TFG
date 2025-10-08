#include "MIDIHandler.h"

// Crea el dispositivo MIDI USB utilizando Adafruit TinyUSB.
// Esta es la interfaz física que aparecerá como "MIDI" en tu ordenador.
Adafruit_USBD_MIDI usb_midi;

// Crea una interfaz MIDI usando la clase MIDI.h (FortySevenEffects) sobre el transporte USB.
// Esta macro define un objeto llamado "MIDI" que usaremos para enviar mensajes.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// Método de inicialización. Se llama en setup().
void MIDIHandler::begin() {
    Serial.println("[MIDIHandler] Iniciando TinyUSB...");
    TinyUSBDevice.begin();

    // Espera máximo 5 segundos a que el USB se monte
    unsigned long start = millis();
    while (!TinyUSBDevice.mounted()) {
        if (millis() - start > 5000) {
            Serial.println("[MIDIHandler] ERROR: USB no se montó tras 5s.");
            return;
        }
        delay(10);
    }

    Serial.println("[MIDIHandler] TinyUSB montado y listo.");
}
// Envía un mensaje MIDI de nota activa (Note On)
void MIDIHandler::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    MIDI.sendNoteOn(note, velocity, channel);
}

// Envía un mensaje MIDI de nota liberada (Note Off)
void MIDIHandler::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    MIDI.sendNoteOff(note, velocity, channel);
}

// Envía un mensaje de Pitch Bend (valor entre 0 y 16383, centro = 8192)
void MIDIHandler::sendPitchBend(uint16_t value, uint8_t channel) {
    MIDI.sendPitchBend(value, channel);
}

// Envía un mensaje MIDI de tipo Control Change (como modulación, expresión, etc.)
void MIDIHandler::sendControlChange(uint8_t ccNumber, uint8_t value, uint8_t channel) {
    MIDI.sendControlChange(ccNumber, value, channel);
}
