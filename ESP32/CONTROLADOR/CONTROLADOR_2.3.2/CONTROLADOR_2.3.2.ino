#include <Wire.h>  

// === CONFIGURACIÓN I2C DEL SISTEMA ============================================================================================
#define SLAVE_ADDR 0x08               
#define SDA_PIN 8                     
#define SCL_PIN 9                     
#define NUM_SENSORS 9                 
#define PACKET_SIZE (NUM_SENSORS * 2 + 1) 
#define REQUEST_INTERVAL_MS 10        

// === VARIABLES DE DATOS DE SENSOR =============================================================================================
uint8_t buffer[PACKET_SIZE];         
uint16_t sensorValues[NUM_SENSORS];  
uint16_t lastValues[NUM_SENSORS] = {0}; 
const uint16_t VALUE_CHANGE_THRESHOLD = 10;  

uint32_t lastRequestTime = 0;        

// === DEFINICIÓN DE ESTADOS DE COMUNICACIÓN ====================================================================================
enum class SensorCommState {
    Waiting,          
    Connected,        
    NoResponse,       
    PacketSizeError,  
    InvalidHeader     
};

SensorCommState lastState = SensorCommState::Waiting;  

void setup() {
    Serial.begin(115200);                
    delay(10);                            
    Wire.begin(SDA_PIN, SCL_PIN);         
    Serial.println("[MASTER] I2C inicializado.");
}

void loop() {
    uint32_t now = millis();              
    if (now - lastRequestTime < REQUEST_INTERVAL_MS) return; 
    lastRequestTime = now;             

    SensorCommState currentState;        

// ========== TEST DE CONEXIÓN CON EL SENSOR ====================================================================================
    Wire.beginTransmission(SLAVE_ADDR);     
    uint8_t error = Wire.endTransmission(); 

    if (error != 0) {                       
        currentState = SensorCommState::NoResponse;
    } else {

// =========== PASO 2: SOLICITAR EL PAQUETE BINARIO COMPLETO ====================================================================
        Wire.requestFrom(SLAVE_ADDR, PACKET_SIZE); 
        size_t bytesRead = Wire.available();       

        if (bytesRead != PACKET_SIZE) {
                                                 // Si no llegan todos los bytes esperados, se limpias el buffer y se marca error
            while (Wire.available()) Wire.read(); 
            currentState = SensorCommState::PacketSizeError;
        } else {
            
// =========== LECTURA Y VALIDACIÓN DEL PAQUETE =================================================================================
            for (size_t i = 0; i < PACKET_SIZE; ++i) {
                buffer[i] = Wire.read();  // Guardamos cada byte en el buffer
            }

            if (buffer[0] != 0xAA) {
                                                                                                       // COMPROBACIÓN DE CABECERA
                currentState = SensorCommState::InvalidHeader;
            } else {

// =========== DESEMPAQUETAR LOS DATOS DEL SENSOR ===============================================================================
                
                currentState = SensorCommState::Connected;

                for (size_t i = 0; i < NUM_SENSORS; ++i) {
                    size_t index = 1 + i * 2;                                                          // BYTE 1 TRAS LA CABECERA
                                                                            // Se reconstruye el valor de 16 bits (MSB << 8 | LSB)
                    sensorValues[i] = (buffer[index] << 8) | buffer[index + 1];
                }
            }
        }
    }

// =========== COMPROBACIÓN DE CAMBIO DE ESTADO DE CONEXIÓN DEL SENSOR ==========================================================
    if (currentState != lastState) {
        switch (currentState) {
            case SensorCommState::Waiting:
                Serial.println("Esperando conexión con el sensor...");
                whyle(true){
                    Serial.print(".");
                }
                break;
            case SensorCommState::NoResponse:
                Serial.println("[ALERTA] Sensor no responde.");
                break;
            case SensorCommState::PacketSizeError:
                Serial.println("[ERROR] Paquete incompleto.");
                break;
            case SensorCommState::InvalidHeader:
                Serial.println("[ERROR] Cabecera inválida.");
                break;
            case SensorCommState::Connected:
                if (lastState == SensorCommState::Waiting) {
                    Serial.println("[INFO] Sensor conectado por primera vez.");
                } else {
                    Serial.println("[INFO] Sensor reconectado correctamente.");
                }
                break;
        }
        lastState = currentState; 
    }

// ============== COMPROBACIÓN "HAS CHANGED" E IMPRESIÓN DEBUG ==================================================================
    if (currentState == SensorCommState::Connected) {
        bool hasChanged = false;

        for (int i = 0; i < NUM_SENSORS; ++i) {
            if (abs((int)sensorValues[i] - (int)lastValues[i]) > VALUE_CHANGE_THRESHOLD) {
                hasChanged = true;  
                break;
            }
        }

        if (hasChanged) {
            
            for (int i = 0; i < NUM_SENSORS; ++i)
                lastValues[i] = sensorValues[i];

            
            printSensorData();
        }
    }
}



// === FUNCIÓN PARA IMPRIMIR LA TABLA DE LECTURAS DE SENSORES ===================================================================
void printSensorData() {
    Serial.println(F("====== SENSOR DATA ======"));
    Serial.println(F("| # | SoftPot |   FSR   | DotFSR |"));
    Serial.println(F("|---|---------|---------|--------|"));
    for (int i = 0; i < 3; ++i) {
        uint16_t softpot = sensorValues[i];        // SoftPot en posición i
        uint16_t fsr     = sensorValues[i + 3];    // FSR en posición i + 3
        uint16_t dotfsr  = sensorValues[i + 6];    // DotFSR en posición i + 6
        Serial.printf("| %d |  %5d  |  %5d  |  %5d |\n", i + 1, softpot, fsr, dotfsr);
    }
    Serial.println(F("==========================\n"));
}
