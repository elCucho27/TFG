#include "SensorManager.h"

SensorManager::SensorManager() {
  sensorCount = 0;
}

void SensorManager::addSensor(int pin, const char* OSCaddress, uint8_t I2Cid, int minVal, int maxVal, float alpha) {
  if (sensorCount < MAX_SENSORS) {
    sensors[sensorCount] = new Sensor(pin, OSCaddress, I2Cid, minVal, maxVal, alpha); // Crea un nuevo sensor
    sensorCount++;
  }
}

void SensorManager::updateSensors() {
  for (int i = 0; i < sensorCount; i++) {
    sensors[i]->update(); // Actualiza cada sensor
  }
}

Sensor* SensorManager::getSensor(int index) {
  if (index >= 0 && index < sensorCount) {
    return sensors[index];
  }
  return nullptr;
}

int SensorManager::getSensorCount() {
  return sensorCount;
}
