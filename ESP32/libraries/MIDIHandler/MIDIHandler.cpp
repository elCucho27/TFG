#include "MIDIHandler.h"

void MIDIHandler::begin() {
    // Inicializa el dispositivo USB MIDI
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // Espera hasta que el dispositivo esté montado o se alcance el tiempo máximo de espera
    unsigned long startTime = millis();
    while (!TinyUSBDevice.mounted()) {
        if (millis() - startTime > 5000) {
            Serial.println("[MIDIHandler] Advertencia: USB no montado tras 5 segundos.");
            break;
        }
        delay(10);
    }

    // Inicializa la interfaz MIDI para escuchar todos los canales
    midiInterface.begin(MIDI_CHANNEL_OMNI);
    Serial.println("[MIDIHandler] MIDI USB inicializado correctamente.");
}

void MIDIHandler::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    midiInterface.sendNoteOn(note, velocity, channel);
}

void MIDIHandler::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    midiInterface.sendNoteOff(note, velocity, channel);
}
