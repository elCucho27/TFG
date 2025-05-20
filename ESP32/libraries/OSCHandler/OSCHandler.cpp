#include "OSCHandler.h"

OSCHandler::OSCHandler(IPAddress remoteIP, uint16_t remotePort)
    : _remoteIP(remoteIP), _remotePort(remotePort) {}

void OSCHandler::begin() {
    _udp.begin(8888); // Puerto local arbitrario
}

void OSCHandler::sendNoteOn(uint8_t stringIndex, uint8_t note, uint8_t velocity) {
    String path = "/3stringController/String" + String(stringIndex) + "/noteOn";
    int32_t data[2] = { note, velocity };
    sendOSCMessage(path, ",ii", data, 2);
}

void OSCHandler::sendNoteOff(uint8_t stringIndex, uint8_t note) {
    String path = "/3stringController/String" + String(stringIndex) + "/noteOff";
    int32_t data[1] = { note };
    sendOSCMessage(path, ",i", data, 1);
}

void OSCHandler::sendModulation(uint8_t i, uint8_t modValue) {
    if (i >= MAX_STRINGS) return;
    if (modValue == lastModValue[i]) return;  // Sin cambio

    lastModValue[i] = modValue;

    String path = "/3stringController/String" + String(i) + "/modulation";
    int32_t data[1] = { modValue };
    sendOSCMessage(path, ",i", data, 1);
}


void OSCHandler::sendPitchBend(uint8_t i, int16_t bendValue) {
    if (i >= MAX_STRINGS) return;
    if (bendValue == lastPitchBend[i]) return;

    lastPitchBend[i] = bendValue;

    String path = "/3stringController/String" + String(i) + "/pitchBend";
    int32_t data[1] = { bendValue };
    sendOSCMessage(path, ",i", data, 1);
}


void OSCHandler::sendRawPosition(uint8_t i, uint16_t value) {
    if (i >= MAX_STRINGS) return;
    if (abs((int)value - (int)lastPosition[i]) < RAW_CHANGE_THRESHOLD) return;

    lastPosition[i] = value;
    String path = "/3stringController/String" + String(i) + "/rawPosition";
    int32_t data[1] = { value };
    sendOSCMessage(path, ",i", data, 1);
}

void OSCHandler::sendRawVelocity(uint8_t i, uint16_t value) {
    if (i >= MAX_STRINGS) return;
    if (abs((int)value - (int)lastVelocity[i]) < RAW_CHANGE_THRESHOLD) return;

    lastVelocity[i] = value;
    String path = "/3stringController/String" + String(i) + "/rawVelocity";
    int32_t data[1] = { value };
    sendOSCMessage(path, ",i", data, 1);
}

void OSCHandler::sendRawModulation(uint8_t i, uint16_t value) {
    if (i >= MAX_STRINGS) return;
    if (abs((int)value - (int)lastModulation[i]) < RAW_CHANGE_THRESHOLD) return;

    lastModulation[i] = value;
    String path = "/3stringController/String" + String(i) + "/rawModulation";
    int32_t data[1] = { value };
    sendOSCMessage(path, ",i", data, 1);
}

void OSCHandler::sendOSCMessage(const String& path, const String& format, const int32_t* data, uint8_t numArgs) {
    if (WiFi.status() != WL_CONNECTED) return;

    const uint8_t MAX_PACKET_SIZE = 255;
    uint8_t packet[MAX_PACKET_SIZE];
    uint8_t index = 0;

    // Construir la dirección OSC
    uint8_t pathLength = path.length();
    memcpy(&packet[index], path.c_str(), pathLength);
    index += pathLength;
    packet[index++] = '\0';
    while (index % 4 != 0) packet[index++] = '\0';

    // Construir la cadena de tipos
    uint8_t formatLength = format.length();
    memcpy(&packet[index], format.c_str(), formatLength);
    index += formatLength;
    packet[index++] = '\0';
    while (index % 4 != 0) packet[index++] = '\0';

    // Añadir los argumentos
    for (uint8_t i = 0; i < numArgs; ++i) {
        packet[index++] = (data[i] >> 24) & 0xFF;
        packet[index++] = (data[i] >> 16) & 0xFF;
        packet[index++] = (data[i] >> 8) & 0xFF;
        packet[index++] = data[i] & 0xFF;
    }

    _udp.beginPacket(_remoteIP, _remotePort);
    _udp.write(packet, index);
    _udp.endPacket();
}
