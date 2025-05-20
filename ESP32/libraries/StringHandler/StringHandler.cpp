// === StringHandler.cpp ===
#include "StringHandler.h"

StringHandler::StringHandler(uint8_t stringIndex) {
    _midiChannel = stringIndex + 1;
    _noteStart = 48;
    _noteRange = 20;
    _trasteadoMode = true;
    _pitchBendEnabled = true;
    _pitchBendSemitones = 2;

    _currentNote = 0;
    _noteIsOn = false;
    _lastSoftpotVal = 0;
    _lastFsrVal = 0;
    _lastDotfsrVal = 0;
    _lastDotfsrTriggerTime = 0;
    _bendReferencePos = 0;

    _noteOnTriggered = false;
    _noteOffTriggered = false;
    _lastModValue = 0;
    _modulationTriggered = false;
}

void StringHandler::setConfig(uint8_t midiChannel, uint8_t noteStart, uint8_t noteRange,
                               bool trasteado, bool pitchBendEnabled, uint8_t pitchBendSemitones) {
    _midiChannel = midiChannel;
    _noteStart = noteStart;
    _noteRange = noteRange;
    _trasteadoMode = trasteado;
    _pitchBendEnabled = pitchBendEnabled;
    _pitchBendSemitones = pitchBendSemitones;
}

void StringHandler::reset() {
    _noteIsOn = false;
    _noteOnTriggered = false;
    _noteOffTriggered = false;
    _modulationTriggered = false;
}

void StringHandler::update(uint16_t softpotVal, uint16_t fsrVal, uint16_t dotfsrVal, uint32_t timestamp) {
    _noteOnTriggered = false;
    _noteOffTriggered = false;
    _modulationTriggered = false;

    // actualizar primero el valor, antes de llamar a processDotFSR()
    _lastSoftpotVal = softpotVal;
    _lastFsrVal = fsrVal;
    _lastDotfsrVal = dotfsrVal;

    processSoftPot(softpotVal);
    processFSR(fsrVal);
    processDotFSR(dotfsrVal, timestamp);
}


void StringHandler::processSoftPot(uint16_t value) {
    uint8_t newNote = mapSoftPotToNote(value);

    if (_trasteadoMode) {
        if (newNote != _currentNote) {
            if (_noteIsOn) _noteOffTriggered = true;
            _currentNote = newNote;
            if (_noteIsOn) _noteOnTriggered = true;
        }
    } else {
        _currentNote = newNote;
    }
}

void StringHandler::processFSR(uint16_t value) {
    uint8_t modVal = map(value, 0, 4095, 0, 127);

    const uint8_t MODULATION_THRESHOLD = 1300;  // umbral ajustable

    if (abs(modVal - _lastModValue) >= MODULATION_THRESHOLD) {
        _modulationTriggered = true;
        _lastModValue = modVal;
    } else {
        _modulationTriggered = false;
    }
}

void StringHandler::processDotFSR(uint16_t value, uint32_t timestamp) {
    const uint16_t thresholdOn = 100;
    const uint16_t thresholdOff = 50;

    if (!_noteIsOn && value > thresholdOn) {
        _noteIsOn = true;
        _lastDotfsrTriggerTime = timestamp;
        _noteOnTriggered = true;
        _bendReferencePos = _lastSoftpotVal;
    } else if (_noteIsOn && value < thresholdOff) {
        _noteIsOn = false;
        _noteOffTriggered = true;
    }
}

uint8_t StringHandler::mapSoftPotToNote(uint16_t value) const {
    return _noteStart + map(value, 0, 4095, 0, _noteRange - 1);
}
int16_t StringHandler::computePitchBend(uint16_t value) const {
    if (!_pitchBendEnabled || _trasteadoMode || !_noteIsOn) return 8192;

    int delta = (int)value - (int)_bendReferencePos;

    // Zona muerta para evitar ruido
    const int DEADZONE = 8;
    if (abs(delta) < DEADZONE) return 8192;

    // Convertimos desplazamiento en pitch bend de ±_pitchBendSemitones
    float norm = (float)delta / 4095.0f;  // desplazamiento relativo (-1..1 aprox)
    float bendSemitones = norm * _pitchBendSemitones;

    // MIDI usa un rango de 14 bits (0..16383), centro en 8192
    int16_t bend = (int16_t)(bendSemitones * (8192.0f / _pitchBendSemitones));
    return constrain(8192 + bend, 0, 16383);
}


uint8_t StringHandler::computeVelocity(uint16_t value, uint32_t timestamp) const {
    uint32_t deltaT = max(timestamp - _lastDotfsrTriggerTime, (uint32_t)1);
    float rate = (float)value / (float)deltaT;
    return constrain(rate * 800.0f, 20, 127);
}

bool StringHandler::hasNoteOnEvent() const { return _noteOnTriggered; }
bool StringHandler::hasNoteOffEvent() const { return _noteOffTriggered; }
bool StringHandler::hasModulationEvent() const { return _modulationTriggered; }

uint8_t StringHandler::getNote() const { return _currentNote; }
uint8_t StringHandler::getVelocity() const { return computeVelocity(_lastDotfsrVal, millis()); }
uint8_t StringHandler::getModulation() const { return _lastModValue; }
int16_t StringHandler::getPitchBend() const { return computePitchBend(_lastSoftpotVal); }