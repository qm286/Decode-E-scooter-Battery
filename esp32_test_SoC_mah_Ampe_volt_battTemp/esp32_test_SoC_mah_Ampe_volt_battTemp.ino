#define RXD2 16
#define TXD2 17
#define RS485_DIR 25  // DTR dùng để điều khiển DE/RE

#define FRAME_LEN 308

uint8_t init1[] = { 0x01, 0x10, 0x16, 0x1C, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD3, 0xCD };
uint8_t init2[] = { 0x01, 0x10, 0x16, 0x1E, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD2, 0x2F };
uint8_t queryCmd[] = { 0x01, 0x10, 0x16, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD6, 0xF1 };

unsigned long lastQueryTime = 0;
uint8_t buffer[FRAME_LEN];

void sendCommand(const uint8_t* cmd, size_t len, const char* label) {
  digitalWrite(RS485_DIR, HIGH);
  delayMicroseconds(100);

  Serial.printf("[SEND] %s: ", label);
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
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(RS485_DIR, OUTPUT);
  digitalWrite(RS485_DIR, LOW);

  delay(500);

  // Gửi init1 và init2 cho đến khi có phản hồi
  bool initSuccess = false;
  while (!initSuccess) {
    sendCommand(init1, sizeof(init1), "Init1");
    delay(300);
    sendCommand(init2, sizeof(init2), "Init2");
    delay(300);

    if (Serial2.available()) {
      Serial.println("[OK] Received response after init.");
      while (Serial2.available()) Serial2.read();  // Clear buffer
      initSuccess = true;
    } else {
      Serial.println("[WAIT] No response, retrying...");
    }
  }
}

void loop() {
  if (millis() - lastQueryTime >= 400) {
    lastQueryTime = millis();

    sendCommand(queryCmd, sizeof(queryCmd), "Query");

    int received = 0;
    unsigned long start = millis();
    while (received < FRAME_LEN && millis() - start < 1000) {
      if (Serial2.available()) {
        buffer[received++] = Serial2.read();
      }
    }

    if (received == FRAME_LEN) {
      Serial.println("[FRAME RECEIVED] 308 bytes");

      // ---- Current (byte 159–160) ----
      // Little Endian, 2's complement, scale ×0.001
      int16_t rawCurrent = (buffer[159] << 8) | buffer[158];
      float current = rawCurrent * 0.001f;

      // ---- SOC (byte 174) ----
      uint8_t soc = buffer[173];  // 1 byte duy nhất

      // ---- Voltage (byte 235–236) ----
      uint16_t rawVoltage = (buffer[235] << 8) | buffer[234];
      float voltage = rawVoltage / 100.0f;

      // ---- Battery Capacity (byte 179–180) ----
      // Little Endian, uint16, scale ÷1000 → Ah
      uint16_t rawCapacity = (buffer[179] << 8) | buffer[178];
      float capacityAh = rawCapacity / 1000.0f;


      // ---- Remain Capacity (byte 175–176) ----
      // Little Endian, uint16, scale ÷1000 → Ah
      uint16_t rawRemain = (buffer[175] << 8) | buffer[174];
      float remainAh = rawRemain / 1000.0f;

      Serial.printf("Current: %.3f A | SoC: %d%% | Voltage: %.2f V | Capacity: %.3f Ah | Remain: %.3f Ah\n",
              current, soc, voltage, capacityAh, remainAh);

    } else {
      Serial.printf("[ERROR] Timeout. Only %d bytes received\n", received);
    }
  }
}
