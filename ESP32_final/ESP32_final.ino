#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ==== BLE UUID ====
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ==== BLE Callback ====
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Client connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Client disconnected.");
    pServer->startAdvertising();
  }
};

// ==== Hall Sensor Setup ====
#define HALL_PIN  21
#define SAMPLE_INTERVAL_US 1000
#define ANALYSIS_INTERVAL_MS 100
#define BUFFER_SIZE (ANALYSIS_INTERVAL_MS * 1000 / SAMPLE_INTERVAL_US)
#define AVG_WINDOW 4

uint8_t buffer[BUFFER_SIZE];
int buffer_index = 0;

float velocity_buffer[AVG_WINDOW] = {0};
int velocity_index = 0;
bool buffer_filled = false;

unsigned long last_sample_time = 0;
unsigned long last_analysis_time = 0;
int analysis_count = 0;

// ==== RS485 Setup ====
#define RXD2 16
#define TXD2 17
#define RS485_DIR 25
#define FRAME_LEN 308

uint8_t init1[] = { 0x01, 0x10, 0x16, 0x1C, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD3, 0xCD };
uint8_t init2[] = { 0x01, 0x10, 0x16, 0x1E, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD2, 0x2F };
uint8_t queryCmd[] = { 0x01, 0x10, 0x16, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD6, 0xF1 };

uint8_t rs485_buffer[FRAME_LEN];

float current = 0.0f;
uint8_t soc = 0;
float voltage = 0.0f;
float capacityAh = 0.0f;
float remainAh = 0.0f;

void sendCommand(const uint8_t* cmd, size_t len, const char* label) {
  digitalWrite(RS485_DIR, HIGH);
  delayMicroseconds(100);

  Serial.printf("[RS485] Sending %s: ", label);
  for (size_t i = 0; i < len; i++) {
    Serial2.write(cmd[i]);
    Serial.printf("%02X ", cmd[i]);
  }
  Serial.println();

  Serial2.flush();
  delayMicroseconds(100);
  digitalWrite(RS485_DIR, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(HALL_PIN, INPUT);

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(RS485_DIR, OUTPUT);
  digitalWrite(RS485_DIR, LOW);

  BLEDevice::init("MINH'S FELIZ E-SCOOTER");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("[BLE] Advertising started.");

  delay(500);

  bool initSuccess = false;
  while (!initSuccess) {
    sendCommand(init1, sizeof(init1), "Init1");
    delay(300);
    sendCommand(init2, sizeof(init2), "Init2");
    delay(300);

    if (Serial2.available()) {
      Serial.println("[RS485] Device responded.");
      while (Serial2.available()) Serial2.read();
      initSuccess = true;
    } else {
      Serial.println("[RS485] Waiting for device...");
    }
  }
}

void loop() {
  unsigned long now = micros();

  if (now - last_sample_time >= SAMPLE_INTERVAL_US) {
    last_sample_time = now;

    unsigned long t_start = micros();
    unsigned long t_high = 0;
    bool last = digitalRead(HALL_PIN);
    unsigned long t_last_change = t_start;

    while (micros() - t_start < SAMPLE_INTERVAL_US) {
      bool current = digitalRead(HALL_PIN);
      if (current != last) {
        unsigned long t_now = micros();
        if (last) t_high += t_now - t_last_change;
        t_last_change = t_now;
        last = current;
      }
    }

    if (last) t_high += micros() - t_last_change;

    bool state = (t_high > (SAMPLE_INTERVAL_US / 2)) ? 1 : 0;
    buffer[buffer_index] = state;
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;
  }

  if (millis() - last_analysis_time >= ANALYSIS_INTERVAL_MS) {
    last_analysis_time = millis();
    analysis_count++;

    int transitions = 0;
    bool last_state = buffer[(buffer_index - BUFFER_SIZE + BUFFER_SIZE) % BUFFER_SIZE];
    for (int i = 1; i < BUFFER_SIZE; i++) {
      int idx = (buffer_index - BUFFER_SIZE + i + BUFFER_SIZE) % BUFFER_SIZE;
      if (buffer[idx] != last_state) {
        transitions++;
        last_state = buffer[idx];
      }
    }

    int pulses = transitions / 2;
    float avg_velocity = 0;

    float velocity_kph = pulses * 3.66 / 2;
    velocity_buffer[velocity_index] = velocity_kph;
    velocity_index = (velocity_index + 1) % AVG_WINDOW;
    if (velocity_index == 0) buffer_filled = true;
    int count = buffer_filled ? AVG_WINDOW : velocity_index;
    for (int i = 0; i < count; i++) {
      avg_velocity += velocity_buffer[i];
    }
    avg_velocity /= count;

    if (analysis_count >= 4) {
      analysis_count = 0;

      sendCommand(queryCmd, sizeof(queryCmd), "Query");

      int received = 0;
      unsigned long start = millis();
      while (received < FRAME_LEN && millis() - start < 1000) {
        if (Serial2.available()) {
          rs485_buffer[received++] = Serial2.read();
        }
      }

      if (received == FRAME_LEN) {
        int16_t rawCurrent = (rs485_buffer[159] << 8) | rs485_buffer[158];
        current = rawCurrent * 0.001f;
        soc = rs485_buffer[173];
        uint16_t rawVoltage = (rs485_buffer[235] << 8) | rs485_buffer[234];
        voltage = rawVoltage / 100.0f;
        uint16_t rawCapacity = (rs485_buffer[179] << 8) | rs485_buffer[178];
        capacityAh = rawCapacity / 1000.0f;
        uint16_t rawRemain = (rs485_buffer[175] << 8) | rs485_buffer[174];
        remainAh = rawRemain / 1000.0f;

        Serial.println("==== DATA ====");
        Serial.printf("Speed: %.1f km/h | Pulses: %d\n", avg_velocity, pulses);
        Serial.printf("I: %.3f A | V: %.2f V | SoC: %d%% | Cap: %.3f Ah | Remain: %.3f Ah\n",
                      current, voltage, soc, capacityAh, remainAh);
        Serial.println("================");

        if (deviceConnected) {
          uint8_t payload[12];
          payload[0] = 0xAA;

          uint16_t speedX10 = (uint16_t)(avg_velocity * 10);
          payload[1] = (speedX10 >> 8) & 0xFF;
          payload[2] = speedX10 & 0xFF;

          int16_t currentX1000 = (int16_t)(current * 1000);
          payload[3] = (currentX1000 >> 8) & 0xFF;
          payload[4] = currentX1000 & 0xFF;

          uint16_t voltageX100 = (uint16_t)(voltage * 100);
          payload[5] = (voltageX100 >> 8) & 0xFF;
          payload[6] = voltageX100 & 0xFF;

          payload[7] = soc;

          uint16_t capX1000 = (uint16_t)(capacityAh * 1000);
          payload[8]  = (capX1000 >> 8) & 0xFF;
          payload[9]  = capX1000 & 0xFF;

          uint16_t remX1000 = (uint16_t)(remainAh * 1000);
          payload[10] = (remX1000 >> 8) & 0xFF;
          payload[11] = remX1000 & 0xFF;

          pCharacteristic->setValue(payload, sizeof(payload));
          pCharacteristic->notify();

          Serial.print("[BLE] Notify HEX: ");
          for (int i = 0; i < sizeof(payload); i++) {
            Serial.printf("%02X ", payload[i]);
          }
          Serial.println();
        }
      } else {
        Serial.printf("[RS485] Timeout, only %d bytes received.\n", received);
      }
    }
  }
}