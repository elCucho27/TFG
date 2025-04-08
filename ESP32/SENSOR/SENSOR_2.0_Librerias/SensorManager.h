#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include "Sensor.h"

#define MAX_SENSORS 10 // Puedes aumentar esto si usas más sensores

class SensorManager {
  public:
    SensorManager();
    void addSensor(int pin, const char* address, int minVal, int maxVal, float alpha);
    void updateSensors();
    Sensor* getSensor(int index);
    int getSensorCount();

  private:
    Sensor* sensors[MAX_SENSORS]; // Arreglo de punteros a sensores
    int sensorCount;              // Número de sensores agregados
};

#endif
