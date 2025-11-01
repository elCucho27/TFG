/*
  APOLO3_INBUS_1_1.ino
  ------------------------------------------------------------
  ESP32-WROOM-32 como ESCLAVO I²C que habla INBUS v1.0 (MSB-first).

  - Sirve un DESCRIPTOR (51 B) con la estructura de 9 canales.
  - Sirve DATA periódica (24 B) con 9 valores uint16 MSB-first.
  - Evita trabajo en interrupciones: buffers se construyen en loop().
  - Wire.begin(ADDRESS) sin especificar SDA/SCL → evita WDT observado.
  - Dirección de ejemplo: 0x13 (dentro del rango de “sensores Apolo3”).

  Requisitos:
  - Tu clase SensorManager (ya existente) para leer SoftPot/FSR/DotFSR.
  - Orden de alta de sensores = orden de canales INBUS:
      [SoftPot1, SoftPot2, SoftPot3, FSR1, FSR2, FSR3, DotFSR1, DotFSR2, DotFSR3]
  - INBUS = MSB-first (big-endian) para todos los uint16 en el cable.

  Autor: tú + asistente
  Fecha: hoy
*/

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "SensorManager.h"  // Tu gestor de sensores (usa getValues())  [verifica include path]

// ========================== CONFIGURACIÓN BÁSICA ============================

// Dirección I²C del módulo Apolo3 (elige una del rango reservado para “sensores Apolo3”)
static const uint8_t I2C_SLAVE_ADDR = 0x13;  // 0x0D..0x1B sugerido en tu diseño

// Velocidad recomendada (el esclavo no fija el clock, pero lo documentamos)
static const uint32_t I2C_BUS_FREQ_HZ = 400000;  // 400 kHz (lo fijará el maestro)

// Tiempos de refresco de DATA (control de “tick” de envío)
static const uint32_t DATA_PERIOD_MS = 16;  // ~60 Hz → 1000/16 ≈ 62.5 Hz

// Número de canales fijos del Apolo3
static const uint8_t N_CHANNELS = 9;

// Comandos simples del maestro (endpoint)
static const uint8_t CMD_DESCRIPTOR = 0x01;  // tras escribir 0x01, el maestro leerá 51 B
static const uint8_t CMD_DATA       = 0x02;  // tras escribir 0x02, el maestro leerá 24 B

// ========================== INBUS (constantes “wire”) =======================

// Cabecera/Tipos/Versión (INBUS v1.0)
static const uint8_t INBUS_SYNC  = 0xA5;
static const uint8_t INBUS_VER   = 0x01;
static const uint8_t TYPE_DESC   = 0xD1;  // DESCRIPTOR
static const uint8_t TYPE_DATA   = 0xD2;  // DATA

// Longitudes (payloads) y totales
static const uint8_t DESC_PAYLOAD_LEN = 5 * N_CHANNELS;  // 45
static const uint8_t DATA_PAYLOAD_LEN = 2 * N_CHANNELS;  // 18
static const uint8_t DESC_TOTAL_LEN   = 5 + DESC_PAYLOAD_LEN + 1;  // 51
static const uint8_t DATA_TOTAL_LEN   = 5 + DATA_PAYLOAD_LEN + 1;  // 24

// Enumeraciones del DESCRIPTOR (1 B cada campo)
static const uint8_t SENSOR_TYPE_SOFTPOT = 0x01;
static const uint8_t SENSOR_TYPE_FSR     = 0x02;
static const uint8_t SENSOR_TYPE_DOTFSR  = 0x03;

static const uint8_t OP_MODE_SOFT_CONT   = 0x01;  // SoftPot continuous
static const uint8_t OP_MODE_MODULATION  = 0x12;  // FSR/DotFSR “modulation”

static const uint8_t FUNC_POSITION       = 0x01;
static const uint8_t FUNC_MODULATION     = 0x41;

static const uint8_t RES_BITS_12         = 0x0C;

// ========================== GLOBALES DE MÓDULO ==============================

// Gestor de sensores (tuyo)
static SensorManager sensorManager;

// Buffers INBUS precocinados (DESCRIPTOR estático, DATA periódico)
static uint8_t descBuf[DESC_TOTAL_LEN];  // 51 bytes
static uint8_t dataBuf[DATA_TOTAL_LEN];  // 24 bytes

// Estado simple de protocolo I²C
volatile static uint8_t lastCmd = CMD_DATA;  // Por defecto, si el maestro pide sin “escribir” antes, servimos DATA
volatile static bool    cmdReceived = false; // Marca de que onReceive() guardó un comando

// Control de tick para reconstruir DATA
static uint32_t lastDataBuildMs = 0;

// ========================== UTILIDADES ======================================

/*
  checksum8():
  Calcula CHECKSUM8 = suma de todos los bytes desde TYPE hasta el final del payload (mod 256).
  Nota: en INBUS, el SYNC (0xA5) NO se incluye en la suma.
*/
static uint8_t checksum8(const uint8_t* frame, uint8_t totalLen) {
  // frame[0] = SYNC
  // frame[1]..frame[totalLen-2] = TYPE..último de payload
  // frame[totalLen-1] = CHECKSUM (destino de este cálculo)
  uint16_t sum = 0;
  for (uint8_t i = 1; i < totalLen - 1; ++i) {
    sum += frame[i];
  }
  return static_cast<uint8_t>(sum & 0xFF);
}

/*
  writeU16_BE(ptr, value):
  Escribe un uint16 en MSB-first (big-endian) en *ptr y *ptr+1.
  IMPORTANTE: INBUS (y OUTBUS) usan MSB-first en el cable.
*/
static inline void writeU16_BE(uint8_t* dst, uint16_t v) {
  dst[0] = highByte(v);  // MSB primero
  dst[1] = lowByte(v);   // LSB después
}

// ========================== CONSTRUCCIÓN DE BUFFERS ==========================

/*
  buildDescriptor():
  Rellena descBuf con el DESCRIPTOR INBUS v1.0 del Apolo3.
  Formato (51 B):
    [0] SYNC=0xA5
    [1] TYPE=0xD1
    [2] VER =0x01
    [3] N_CHANNELS=9
    [4] LEN=45
    [5..49] payload: 9 entradas × 5 B:
         (CHAN_ID, SENSOR_TYPE, OP_MODE, FUNC_PREF, RES_BITS)
    [50] CHECKSUM8
*/
static void buildDescriptor() {
  // Cabecera
  descBuf[0] = INBUS_SYNC;
  descBuf[1] = TYPE_DESC;
  descBuf[2] = INBUS_VER;
  descBuf[3] = N_CHANNELS;
  descBuf[4] = DESC_PAYLOAD_LEN;  // 45

  // Payload por canal (5 bytes × 9)
  uint8_t* p = &descBuf[5];

  // CH1..3: SoftPot Continuous → Position
  for (uint8_t ch = 1; ch <= 3; ++ch) {
    *p++ = ch;                       // CHAN_ID
    *p++ = SENSOR_TYPE_SOFTPOT;      // SENSOR_TYPE
    *p++ = OP_MODE_SOFT_CONT;        // OP_MODE
    *p++ = FUNC_POSITION;            // FUNC_PREF
    *p++ = RES_BITS_12;              // RES_BITS
  }

  // CH4..6: FSR Modulation → Modulation
  for (uint8_t ch = 4; ch <= 6; ++ch) {
    *p++ = ch;
    *p++ = SENSOR_TYPE_FSR;
    *p++ = OP_MODE_MODULATION;
    *p++ = FUNC_MODULATION;
    *p++ = RES_BITS_12;
  }

  // CH7..9: DotFSR Modulation → Modulation (según tu decisión)
  for (uint8_t ch = 7; ch <= 9; ++ch) {
    *p++ = ch;
    *p++ = SENSOR_TYPE_DOTFSR;
    *p++ = OP_MODE_MODULATION;
    *p++ = FUNC_MODULATION;
    *p++ = RES_BITS_12;
  }

  // Checksum
  descBuf[DESC_TOTAL_LEN - 1] = checksum8(descBuf, DESC_TOTAL_LEN);
}

/*
  buildData():
  Rellena dataBuf con DATA INBUS v1.0 (24 B), a partir de los valores actuales de SensorManager.
  Formato:
    [0] SYNC=0xA5
    [1] TYPE=0xD2
    [2] VER =0x01
    [3] N_CHANNELS=9
    [4] LEN=18
    [5..22] 9×VALUE (uint16) en MSB-first, orden CH1..CH9
    [23] CHECKSUM8
*/
static void buildData(const std::vector<uint16_t>& values) {
  // Cabecera
  dataBuf[0] = INBUS_SYNC;
  dataBuf[1] = TYPE_DATA;
  dataBuf[2] = INBUS_VER;
  dataBuf[3] = N_CHANNELS;
  dataBuf[4] = DATA_PAYLOAD_LEN;  // 18

  // Payload: 9×uint16 (MSB-first), orden CH1..CH9
  // IMPORTANTE: el orden de alta en SensorManager DEBE ser: S1,S2,S3,F1,F2,F3,D1,D2,D3
  // Así, values[i] ya coincide con CH=i+1.
  uint8_t* p = &dataBuf[5];
  for (uint8_t i = 0; i < N_CHANNELS; ++i) {
    writeU16_BE(p, values[i]);  // MSB, LSB
    p += 2;
  }

  // Checksum
  dataBuf[DATA_TOTAL_LEN - 1] = checksum8(dataBuf, DATA_TOTAL_LEN);
}

// ========================== CALLBACKS I²C (ISR, trabajo mínimo) ==============

/*
  onReceive():
  El maestro “escribe” un byte con el comando que desea (0x01=DESC, 0x02=DATA).
  Aquí SOLO almacenamos el comando. No hacemos cálculos.
*/
static void onReceiveHandler(int numBytes) {
  if (numBytes <= 0) return;
  // Leemos el último byte recibido como comando (si envía más de uno, nos quedamos con el último)
  while (Wire.available()) {
    lastCmd = Wire.read();
  }
  cmdReceived = true;  // Marcamos que hay un comando “pendiente de servir”
}

/*
  onRequest():
  El maestro realiza una lectura I²C. Aquí SOLO enviamos el buffer correspondiente a lastCmd.
  Nada de recalcular; los buffers ya están construidos.
*/
static void onRequestHandler() {
  if (lastCmd == CMD_DESCRIPTOR) {
    Wire.write(descBuf, DESC_TOTAL_LEN);  // 51 B
  } else {
    Wire.write(dataBuf, DATA_TOTAL_LEN);  // 24 B
  }
  // Tras enviar, podemos dejar lastCmd igual (el maestro suele reescribir antes de leer de nuevo).
}

// ========================== INIT DE SENSORES (ORDEN CANALES) =================

// Ajusta estos pines a tu cableado real:
static const uint8_t PIN_SOFTPOT_1 = 32;
static const uint8_t PIN_SOFTPOT_2 = 33;
static const uint8_t PIN_SOFTPOT_3 = 34;
static const uint8_t PIN_FSR_1     = 35;
static const uint8_t PIN_FSR_2     = 36;
static const uint8_t PIN_FSR_3     = 39;
static const uint8_t PIN_DOTFSR_1  = 25;
static const uint8_t PIN_DOTFSR_2  = 26;
static const uint8_t PIN_DOTFSR_3  = 27;

/*
  setupSensorManager():
  Da de alta 9 sensores en el orden EXACTO del mapa INBUS.
  De este modo, sensorManager.getValues()[i] coincide con CH=i+1.
*/
static void setupSensorManager() {
  // 1..3 SoftPot
  sensorManager.addSensor(SensorType::SoftPot, PIN_SOFTPOT_1);
  sensorManager.addSensor(SensorType::SoftPot, PIN_SOFTPOT_2);
  sensorManager.addSensor(SensorType::SoftPot, PIN_SOFTPOT_3);

  // 4..6 FSR
  sensorManager.addSensor(SensorType::FSR, PIN_FSR_1);
  sensorManager.addSensor(SensorType::FSR, PIN_FSR_2);
  sensorManager.addSensor(SensorType::FSR, PIN_FSR_3);

  // 7..9 DotFSR
  sensorManager.addSensor(SensorType::DotFSR, PIN_DOTFSR_1);
  sensorManager.addSensor(SensorType::DotFSR, PIN_DOTFSR_2);
  sensorManager.addSensor(SensorType::DotFSR, PIN_DOTFSR_3);

  sensorManager.begin();  // inicializa hardware/estado de todos los sensores
}

// ========================== SETUP / LOOP =====================================

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1) Sensores en el orden EXACTO del mapa INBUS
  setupSensorManager();

  // 2) Construye DESCRIPTOR una sola vez (estático)
  buildDescriptor();

  // 3) Arranca I²C como ESCLAVO, solo con dirección (NO fijamos pines → evita WDT)
  Wire.begin(I2C_SLAVE_ADDR);  // ¡No uses (addr, SDA, SCL) en ESP32 esclavo para evitar TG1WDT!
  Wire.onReceive(onReceiveHandler);
  Wire.onRequest(onRequestHandler);

  Serial.println(F("[Apolo3] INBUS v1.0 listo. Dirección I2C: 0x13"));
  Serial.println(F("          CMD 0x01 → DESCRIPTOR (51B), CMD 0x02 → DATA (24B)."));
}

void loop() {
  // 1) Actualiza lecturas crudas de todos los sensores (a la máxima cadencia que quieras)
  sensorManager.update();

  // 2) Tick para reconstruir DATA a ~60 Hz
  const uint32_t now = millis();
  if (now - lastDataBuildMs >= DATA_PERIOD_MS) {
    lastDataBuildMs = now;

    // Obtiene los 9 valores en el orden de alta
    const std::vector<uint16_t>& vals = sensorManager.getValues();  // 9 elementos esperados

    // Reconstruye el paquete DATA completo (cabecera + 9×u16 MSB-first + checksum)
    buildData(vals);
  }

  // 3) (Opcional) Depuración simple
  //    Descomenta si quieres ver cada 500 ms la primera tercia de valores:
  /*
  static uint32_t lastDbg = 0;
  if (now - lastDbg >= 500) {
    lastDbg = now;
    const auto& v = sensorManager.getValues();
    if (v.size() >= 9) {
      Serial.printf("S1=%4u S2=%4u S3=%4u | F1=%4u F2=%4u F3=%4u | D1=%4u D2=%4u D3=%4u\n",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
    }
  }
  */
}
