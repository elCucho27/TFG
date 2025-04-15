# SensorManager

## Propósito
Clase encargada de instanciar, almacenar y actualizar múltiples objetos de tipo `Sensor`, actuando como punto centralizado de acceso a los sensores conectados a la placa.

## Descripción
La clase `SensorManager` permite manejar múltiples sensores analógicos conectados a una misma placa de forma escalable. Internamente mantiene un arreglo de punteros a objetos `Sensor`, y proporciona métodos para:
- Añadir nuevos sensores.
- Actualizar todos los sensores.
- Acceder a sensores individuales por índice.
- Obtener el número total de sensores activos.

## Métodos públicos

### SensorManager()
Constructor por defecto. Inicializa el gestor con cero sensores.

### void addSensor(int pin, const char* OSCaddress, uint8_t I2Cid, int minVal, int maxVal, float alpha)
Crea una nueva instancia de `Sensor` y la almacena en el arreglo interno. Limita el número de sensores a `MAX_SENSORS`.

### void updateSensors()
Llama al método `update()` de todos los sensores registrados, para mantener actualizados sus valores.

### Sensor* getSensor(int index)
Devuelve un puntero al sensor en la posición `index`. Devuelve `nullptr` si el índice no es válido.

### int getSensorCount()
Devuelve el número de sensores registrados hasta el momento.

## Atributos privados
- `sensors[MAX_SENSORS]`: arreglo de punteros a objetos `Sensor`.
- `sensorCount`: contador de sensores registrados.
