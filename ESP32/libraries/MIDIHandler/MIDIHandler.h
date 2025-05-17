#pragma once

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Clase MIDIHandler: Encapsula la funcionalidad MIDI USB
class MIDIHandler {
public:
    // Inicializa el dispositivo MIDI USB y la interfaz MIDI
    void begin();

    // Envía un mensaje MIDI Note On
    void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);

    // Envía un mensaje MIDI Note Off
    void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);

private:
    // Instancia del dispositivo USB MIDI
    Adafruit_USBD_MIDI usbMIDI;

    // Interfaz MIDI utilizando la instancia de usbMIDI
    MIDI_NAMESPACE::MidiInterface<Adafruit_USBD_MIDI> midiInterface = 
        MIDI_NAMESPACE::MidiInterface<Adafruit_USBD_MIDI>(usbMIDI);
};
