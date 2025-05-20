#include "SensorManager.h"     
#include "SoftPotReader.h"      // Implementación de sensores SoftPot
#include "FSRReader.h"          // Implementación de sensores FSR
#include "DotFSRReader.h"       // Implementación de sensores DotFSR
// Constructor vacío
SensorManager::SensorManager() {}

// ====================================== Destructor
SensorManager::~SensorManager() {
    for (auto s : _sensors) delete s;  // Elimina cada sensor creado con `new`
}

// ====================================== Agrega un nuevo sensor a la lista según su tipo
void SensorManager::addSensor(SensorType type, uint8_t pin) {
    ISensor* sensor = nullptr;

    switch (type) {
        case SensorType::SoftPot:
            sensor = new SoftPotReader(pin);  
            break;
        case SensorType::FSR:
            sensor = new FSRReader(pin);     
            break;
	case SensorType::DotFSR:
	    sensor = new DotFSRReader(pin);  
    	    break;
        default:
            return;  // Si el tipo no es válido, no se hace nada
    }

    _sensors.push_back(sensor);  // Añade el sensor creado a la lista
}

// ================================================ Inicializa todos los sensores registrados
void SensorManager::begin() {
    for (auto s : _sensors) s->begin();             // Llama al `begin()` de cada sensor
    _values.resize(_sensors.size());                // Reserva espacio para guardar lecturas
    _lastSentValues.resize(_sensors.size());        // Lo mismo para las últimas enviadas
}

// Actualiza todas las lecturas de sensores
void SensorManager::update() {
    for (size_t i = 0; i < _sensors.size(); ++i) {
        _sensors[i]->update();                      // Pide nueva lectura al sensor
        _values[i] = _sensors[i]->getValue();       // Guarda el valor filtrado
    }
}

// ================================================ Devuelve los valores actuales (referencia constante)
const std::vector<uint16_t>& SensorManager::getValues() const {
    return _values;
}

// ================================================ Convierte cada valor uint16_t en dos bytes (formato MSB primero)
std::vector<uint8_t> SensorManager::packetize() const {

    std::vector<uint8_t> buf;
    buf.reserve(_values.size() * 2 + 1);  // 1 byte para la cabecera
    buf.push_back(0xAA);  // byte de cabecera

    for (auto v : _values) {
        buf.push_back(highByte(v));  // Byte más significativo
        buf.push_back(lowByte(v));   // Byte menos significativo
    }

    return buf;
}

// ================================================ Devuelve true si al menos un valor ha cambiado más que el umbral
bool SensorManager::hasSignificantChange(uint16_t threshold) {
    for (size_t i = 0; i < _values.size(); ++i) {
        if (abs((int)_values[i] - (int)_lastSentValues[i]) > threshold)
            return true; // Cambio significativo detectado
    }
    return false; // Ningún cambio importante
}

// ================================================ Copia los valores actuales como nueva "referencia" para la siguiente comparación
void SensorManager::markValuesAsSent() {
    for (size_t i = 0; i < _values.size(); ++i)
        _lastSentValues[i] = _values[i];
}
