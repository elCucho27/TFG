#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <WiFi.h>
#include <WiFiUdp.h>

class OSCHandler {
public:
    OSCHandler(IPAddress remoteIP, uint16_t remotePort);
    void begin();

    // Eventos musicales procesados
    void sendNoteOn(uint8_t stringIndex, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t stringIndex, uint8_t note);
    void sendModulation(uint8_t stringIndex, uint8_t modValue);
    void sendPitchBend(uint8_t stringIndex, int16_t bendValue);

    // Datos crudos
    void sendRawPosition(uint8_t stringIndex, uint16_t softpotVal);
    void sendRawVelocity(uint8_t stringIndex, uint16_t dotfsrVal);
    void sendRawModulation(uint8_t stringIndex, uint16_t fsrVal);

private:
    WiFiUDP _udp;
    IPAddress _remoteIP;
    uint16_t _remotePort;

    void sendOSCMessage(const String& path, const String& format, const int32_t* data, uint8_t numArgs);

    // === control de saturación para datos crudos ===
    static const uint8_t MAX_STRINGS = 3;
    static const uint16_t RAW_CHANGE_THRESHOLD = 2;

    uint16_t lastPosition[MAX_STRINGS] = {0};
    uint16_t lastModulation[MAX_STRINGS] = {0};
    uint16_t lastVelocity[MAX_STRINGS] = {0};

    int16_t lastPitchBend[MAX_STRINGS] = {8192};   // 8192 = centro neutro
    uint8_t lastModValue[MAX_STRINGS] = {255};     // valor imposible al inicio

};

#endif // OSC_HANDLER_H
