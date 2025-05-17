#include "StringHandler.h"

// === CONSTRUCTOR ===
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
    _noteOnTriggered = false;
    _noteOffTriggered = false;

    _lastModValue = 0;
    _modulationTriggered = false;
}

// === CONFIGURACIÓN ===
void StringHandler::setConfig(uint8_t midiChannel, uint8_t noteStart, uint8_t noteRange, bool trasteado, bool pitchBendEnabled, uint8_t pitchBendSemitones) {
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

// === ACTUALIZACIÓN PRINCIPAL ===
void StringHandler::update(uint16_t softpotVal, uint16_t fsrVal, uint16_t dotfsrVal, uint32_t timestamp) {
    _noteOnTriggered = false;
    _noteOffTriggered = false;
    _modulationTriggered = false;

    processSoftPot(softpotVal);
    processFSR(fsrVal);
    processDotFSR(dotfsrVal, timestamp);

    _lastSoftpotVal = softpotVal;
    _lastFsrVal = fsrVal;
    _lastDotfsrVal = dotfsrVal;
}

// === PROCESAR SOFTPOT ===
void StringHandler::processSoftPot(uint16_t value) {
    uint8_t newNote = mapSoftPotToNote(value);

    if (_trasteadoMode) {
        if (newNote != _currentNote) {
            if (_noteIsOn) {
                _noteOffTriggered = true;
            }
            _currentNote = newNote;
            if (_noteIsOn) {
                _noteOnTriggered = true;
            }
        }
    } else {
        _currentNote = newNote;
    }
}

// === MAPEO DE POSICIÓN A NOTA MIDI ===
uint8_t StringHandler::mapSoftPotToNote(uint16_t value) const {
    return _noteStart + map(value, 0, 4095, 0, _noteRange - 1);
}

// === CÁLCULO DE PITCH BEND ===
int16_t StringHandler::computePitchBend(uint16_t value) const {
    if (!_pitchBendEnabled || _trasteadoMode) return 8192;

    float norm = (float)value / 4095.0f;
    int16_t maxBend = _pitchBendSemitones * 8192 / 2;
    int16_t bend = (int16_t)(norm * (2 * maxBend)) - maxBend;
    return 8192 + bend;
}

// === PROCESAR DOT FSR (NOTE ON/OFF) ===
void StringHandler::processDotFSR(uint16_t value, uint32_t timestamp) {
    const uint16_t thresholdOn = 50;
    const uint16_t thresholdOff = 30;

    if (!_noteIsOn && value > thresholdOn) {
        _noteIsOn = true;
        _lastDotfsrTriggerTime = timestamp;
        _noteOnTriggered = true;
    } else if (_noteIsOn && value < thresholdOff) {
        _noteIsOn = false;
        _noteOffTriggered = true;
    }
}

// === PROCESAR FSR (MODULACIÓN / CC) ===
void StringHandler::processFSR(uint16_t value) {
    uint8_t modVal = map(value, 0, 4095, 0, 127);

    // Solo marcar evento si el valor cambió significativamente
    if (abs(modVal - _lastModValue) > 3) {
        _modulationTriggered = true;
        _lastModValue = modVal;
    } else {
        _modulationTriggered = false;
    }
}

// === CÁLCULO DE VELOCIDAD MIDI ===
// Mide aceleración del golpe para determinar la intensidad expresiva
uint8_t StringHandler::computeVelocity(uint16_t value, uint32_t timestamp) const {
    // Calculamos el tiempo desde el último evento DotFSR
    // El casting a uint32_t es necesario para evitar error de tipo en std::max()
    uint32_t deltaT = max(timestamp - _lastDotfsrTriggerTime, (uint32_t)1);

    // Cuanto más rápido suba la presión, mayor la velocidad
    float rate = (float)value / (float)deltaT;

    // Ajuste empírico: mínimo 20, máximo 127
    uint8_t velocity = constrain(rate * 800.0f, 20, 127);
    return velocity;
}


// === ACCESO A EVENTOS ===
bool StringHandler::hasNoteOnEvent() const {
    return _noteOnTriggered;
}

bool StringHandler::hasNoteOffEvent() const {
    return _noteOffTriggered;
}

bool StringHandler::hasModulationEvent() const {
    return _modulationTriggered;
}

// === ACCESO A VALORES ===
uint8_t StringHandler::getNote() const {
    return _currentNote;
}

uint8_t StringHandler::getVelocity() const {
    return computeVelocity(_lastDotfsrVal, millis());
}

uint8_t StringHandler::getModulation() const {
    return _lastModValue;
}

int16_t StringHandler::getPitchBend() const {
    return computePitchBend(_lastSoftpotVal);
}
