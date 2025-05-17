#ifndef CONFIG_H
#define CONFIG_H

// --- I2C ---
#define SLAVE_ADDR         0x08
#define I2C_SDA_PIN        8
#define I2C_SCL_PIN        9
#define REQUEST_INTERVAL_MS 10

// --- MIDI ---
#define MIDI_CHANNEL       1

// --- Lógica Sensor ---
#define FSR_THRESHOLD      100       // Umbral para detección de presión en FSR
#define DEBOUNCE_TIME_MS   30        // Tiempo mínimo entre cambios de estado
#endif // CONFIG_H
