#ifndef STRING_HANDLER_H
#define STRING_HANDLER_H

#include <Arduino.h>

class StringHandler {
public:
    // Constructor: recibe el índice de la cuerda
    StringHandler(uint8_t stringIndex);

    // Configuración de parámetros MIDI y expresivos
    void setConfig(uint8_t midiChannel, uint8_t noteStart, uint8_t noteRange,
                   bool trasteado, bool pitchBendEnabled, uint8_t pitchBendSemitones);

    // Reinicia estado interno (ej: tras pérdida de conexión)
    void reset();

    // Actualiza el estado interno en base a lecturas de sensores
    void update(uint16_t softpotVal, uint16_t fsrVal, uint16_t dotfsrVal, uint32_t timestamp);

    // Consulta de eventos detectados (para generar mensajes MIDI/OSC/CV)
    bool hasNoteOnEvent() const;
    bool hasNoteOffEvent() const;
    bool hasModulationEvent() const;

    // Consulta de valores actuales
    uint8_t getNote() const;           // Nota MIDI actual (0–127)
    uint8_t getVelocity() const;       // Velocidad calculada desde DotFSR
    uint8_t getModulation() const;     // Valor de modulación (0–127)
    int16_t getPitchBend() const;      // Pitch bend absoluto (0–16383)

private:
    // Parámetros configurables por cuerda
    uint8_t _midiChannel;
    uint8_t _noteStart;
    uint8_t _noteRange;
    bool _trasteadoMode;
    bool _pitchBendEnabled;
    uint8_t _pitchBendSemitones;

    // Estado de nota actual
    uint8_t _currentNote;
    bool _noteIsOn;

    // Últimas lecturas de sensores
    uint16_t _lastSoftpotVal;
    uint16_t _lastFsrVal;
    uint16_t _lastDotfsrVal;
    uint32_t _lastDotfsrTriggerTime;

    // Estado de eventos
    bool _noteOnTriggered;
    bool _noteOffTriggered;

    // Gestión de modulación (FSR)
    uint8_t _lastModValue;
    bool _modulationTriggered;

    // Métodos internos de procesamiento de sensores
    void processSoftPot(uint16_t value);
    void processFSR(uint16_t value);
    void processDotFSR(uint16_t value, uint32_t timestamp);

    // Métodos auxiliares
    uint8_t mapSoftPotToNote(uint16_t value) const;
    int16_t computePitchBend(uint16_t value) const;
    uint8_t computeVelocity(uint16_t value, uint32_t timestamp) const;
};

#endif // STRING_HANDLER_H
