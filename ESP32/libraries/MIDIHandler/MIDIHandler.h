#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include "Adafruit_TinyUSB.h"
#include <MIDI.h>


// Clase MIDIHandler: se encarga de enviar mensajes MIDI USB desde el ESP32-S3
// Utiliza la interfaz USB proporcionada por Adafruit_TinyUSB
class MIDIHandler {
public:
    // Inicializa la interfaz MIDI (debe llamarse en setup())
    void begin();

    // Envía una nota MIDI "Note On" con nota, velocidad y canal
    void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);

    // Envía una nota "Note Off" (libera la nota)
    void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);

    // Envía un mensaje de Pitch Bend (14 bits, centro = 8192)
    void sendPitchBend(uint16_t value, uint8_t channel);

    // Envía un mensaje MIDI de tipo Control Change (por ejemplo, CC1 para modulación)
    void sendControlChange(uint8_t ccNumber, uint8_t value, uint8_t channel);
};
#endif