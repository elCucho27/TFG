#ifndef CONFIG_H
#define CONFIG_H

// I2C Configuration
#define SLAVE_ADDR 0x08
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define REQUEST_INTERVAL_MS 10

// Sensor Configuration
#define NUM_SENSORS 9
#define NUM_STRINGS 3

// Thresholds and timings
#define FSR_THRESHOLD 3300
#define MIDI_CHANNEL 1
#define DEBOUNCE_TIME_MS 30

#endif // CONFIG_H