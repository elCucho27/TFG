# Sensor

## Propósito
Clase encargada de representar un sensor analógico individual, incluyendo su lectura, filtrado, normalización y gestión de identificación tanto para OSC como para I2C.

## Descripción
La clase `Sensor` encapsula toda la lógica necesaria para manejar un sensor físico conectado a un pin analógico de la placa ESP32. Esta clase proporciona:
- Lectura cruda desde el pin.
- Filtrado exponencial para estabilizar la señal.
- Normalización del valor a un rango 0.0 – 1.0.
- Detección de cambios relevantes.
- Identificación mediante una ruta OSC (string) y un identificador I2C (uint8_t).

## Métodos públicos

### Sensor(int pin, const char* OSCaddress, uint8_t I2Cid, int minVal, int maxVal, float alpha)
Constructor. Inicializa el sensor configurando el pin y almacenando todos los parámetros necesarios para el filtrado y la identificación.

### void update()
Lee el valor actual del sensor, lo normaliza y lo filtra mediante un filtro exponencial.

### bool hasChanged()
Devuelve `true` si el valor del sensor ha cambiado significativamente desde la última lectura enviada.

### float getValue()
Devuelve el valor actual del sensor, ya normalizado y filtrado.

### const char* getAddress()
Devuelve la ruta OSC asociada al sensor (útil para comunicación por red).

### uint8_t getId()
Devuelve el identificador I2C asociado al sensor, utilizado para protocolos binarios.

## Atributos privados
- `_pin`: pin analógico al que está conectado el sensor.
- `_oscAddress`: string con la ruta OSC del sensor.
- `_I2Cid`: identificador I2C único para cada sensor.
- `_minValue`, `_maxValue`: límites de normalización.
- `_alpha`: coeficiente del filtro exponencial.
- `_filteredValue`: último valor calculado y filtrado.
- `_lastSentValue`: último valor enviado, usado para comparar cambios.
