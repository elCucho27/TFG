// === StringHandler.h ===
#ifndef STRING_HANDLER_H
#define STRING_HANDLER_H

#include <Arduino.h>

// Clase que gestiona el comportamiento de una "cuerda" musical
class StringHandler {
public:
    StringHandler(uint8_t stringIndex);

    void setConfig(uint8_t midiChannel, uint8_t noteStart, uint8_t noteRange,
                   bool trasteado, bool pitchBendEnabled, uint8_t pitchBendSemitones);
    void reset();
    void update(uint16_t softpotVal, uint16_t fsrVal, uint16_t dotfsrVal, uint32_t timestamp);

    bool hasNoteOnEvent() const;
    bool hasNoteOffEvent() const;
    bool hasModulationEvent() const;

    uint8_t getNote() const;
    uint8_t getVelocity() const;
    uint8_t getModulation() const;
    int16_t getPitchBend() const;

    // NUEVO: acceso público para depuración
    uint16_t getBendReference() const { return _bendReferencePos; }

private:
    uint8_t _midiChannel;
    uint8_t _noteStart;
    uint8_t _noteRange;
    bool _trasteadoMode;
    bool _pitchBendEnabled;
    uint8_t _pitchBendSemitones;

    uint8_t _currentNote;
    bool _noteIsOn;

    uint16_t _lastSoftpotVal;
    uint16_t _lastFsrVal;
    uint16_t _lastDotfsrVal;
    uint32_t _lastDotfsrTriggerTime;
    uint16_t _bendReferencePos;

    bool _noteOnTriggered;
    bool _noteOffTriggered;

    uint8_t _lastModValue;
    bool _modulationTriggered;

    void processSoftPot(uint16_t value);
    void processFSR(uint16_t value);
    void processDotFSR(uint16_t value, uint32_t timestamp);

    uint8_t mapSoftPotToNote(uint16_t value) const;
    int16_t computePitchBend(uint16_t value) const;
    uint8_t computeVelocity(uint16_t value, uint32_t timestamp) const;
};

#endif // STRING_HANDLER_H
