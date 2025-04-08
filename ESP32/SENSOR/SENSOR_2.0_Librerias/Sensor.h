#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    Sensor(int pin, const char* oscAddress, int minValue, int maxValue, float alpha);

    void update();                 // Actualiza el valor leído desde el pin
    bool hasChanged();            // Devuelve true si el valor cambió desde la última lectura
    float getValue();             // Devuelve el valor actual filtrado y normalizado (0.0 - 1.0)
    const char* getAddress();     // Devuelve la ruta OSC asociada al sensor

  private:
    int _pin;                     // Pin analógico al que está conectado el sensor
    const char* _oscAddress;      // Ruta OSC (ej. "/LAVANA/fsr1")
    int _minValue;                // Valor mínimo esperado de lectura analógica
    int _maxValue;                // Valor máximo esperado
    float _alpha;                 // Coeficiente del filtro exponencial
    float _filteredValue;        // Valor filtrado actual
    float _lastSentValue;        // Último valor enviado (para detectar cambios)
    float normalize(int raw);     // Convierte valor crudo a rango 0.0 - 1.0
};

#endif
