// SensorManager.cpp
#include "SensorManager.h"
#include "SoftPotReader.h"
#include "FSRReader.h"   // Solo si quieres también FSRs

SensorManager::SensorManager() {}

SensorManager::~SensorManager() {
    // Importante: liberar la memoria reservada dinámicamente
    for (auto s : _sensors) {
        delete s;
    }
}

void SensorManager::addSensor(SensorType type, uint8_t pin) {
    ISensor* sensor = nullptr;
    switch (type) {
        case SensorType::SoftPot:
            sensor = new SoftPotReader(pin);
            break;
        case SensorType::FSR:
            sensor = new FSRReader(pin);
            break;
        default:
            return; // No hacemos nada si no reconocemos el tipo
    }
    _sensors.push_back(sensor);  // Añadir a la lista
}

void SensorManager::begin() {
    for (auto s : _sensors) {
        s->begin();  // Inicializar cada sensor individualmente
    }
    _values.resize(_sensors.size()); // Reservamos el espacio justo
}

void SensorManager::update() {
    for (size_t i = 0; i < _sensors.size(); ++i) {
        _sensors[i]->update();           // Actualizar cada sensor
        _values[i] = _sensors[i]->getValue(); // Guardar su valor
    }
}

const std::vector<uint16_t>& SensorManager::getValues() const {
    return _values;
}

std::vector<uint8_t> SensorManager::packetize() const {
    std::vector<uint8_t> buf;
    buf.reserve(_values.size() * 2);    // Reservamos espacio para optimizar

    for (auto v : _values) {
        buf.push_back(highByte(v));     // Byte más significativo primero
        buf.push_back(lowByte(v));      // Byte menos significativo después
    }

    return buf;
}
