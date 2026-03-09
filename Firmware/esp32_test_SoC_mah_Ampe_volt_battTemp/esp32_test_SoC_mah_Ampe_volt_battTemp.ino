/*
  RS485 Battery Data Query and Decode for ESP32
  ---------------------------------------------
  This code uses ESP32 UART2 to communicate with a battery system over RS485.

  What it does:
  - Configures ESP32 as an RS485 master using a direction-control pin
  - Sends two initialization commands until the battery responds
  - Periodically sends a query command every 400 ms
  - Reads a full 308-byte response frame
  - Extracts key battery parameters from specific byte positions
  - Prints decoded values to the Serial Monitor

  Decoded parameters:
  - Current (A)
  - State of Charge, SoC (%)
  - Voltage (V)
  - Total battery capacity (Ah)
  - Remaining capacity (Ah)

  Notes:
  - UART2 is used for RS485 communication
  - RS485_DIR controls transmit/receive direction on the transceiver
  - Multi-byte values are decoded in little-endian format
  - Some fields use scaling factors to convert raw data into physical units
*/

#define RXD2 16
#define TXD2 17
#define RS485_DIR 25  // Direction-control pin for RS485 DE/RE

#define FRAME_LEN 308

// Initialization command 1
uint8_t init1[] = { 0x01, 0x10, 0x16, 0x1C, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD3, 0xCD };

// Initialization command 2
uint8_t init2[] = { 0x01, 0x10, 0x16, 0x1E, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD2, 0x2F };

// Periodic query command
uint8_t queryCmd[] = { 0x01, 0x10, 0x16, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD6, 0xF1 };

unsigned long lastQueryTime = 0;
uint8_t buffer[FRAME_LEN];

// Send one RS485 command and print it to Serial
void sendCommand(const uint8_t* cmd, size_t len, const char* label) {
  digitalWrite(RS485_DIR, HIGH);      // Enable transmit mode
  delayMicroseconds(100);             // Allow transceiver to switch

  Serial.printf("[SEND] %s: ", label);
  for (size_t i = 0; i < len; i++) {
    Serial2.write(cmd[i]);
    Serial.printf("%02X ", cmd[i]);
  }
  Serial.println();

  Serial2.flush();                    // Wait until all bytes are transmitted
  delayMicroseconds(100);             // Small guard time before switching back
  digitalWrite(RS485_DIR, LOW);       // Return to receive mode
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(RS485_DIR, OUTPUT);
  digitalWrite(RS485_DIR, LOW);       // Default to receive mode

  delay(500);

  // Send init1 and init2 repeatedly until the device responds
  bool initSuccess = false;
  while (!initSuccess) {
    sendCommand(init1, sizeof(init1), "Init1");
    delay(300);

    sendCommand(init2, sizeof(init2), "Init2");
    delay(300);

    if (Serial2.available()) {
      Serial.println("[OK] Received response after init.");
      while (Serial2.available()) Serial2.read();  // Clear response buffer
      initSuccess = true;
    } else {
      Serial.println("[WAIT] No response, retrying...");
    }
  }
}

void loop() {
  // Send one query every 400 ms
  if (millis() - lastQueryTime >= 400) {
    lastQueryTime = millis();

    sendCommand(queryCmd, sizeof(queryCmd), "Query");

    int received = 0;
    unsigned long start = millis();

    // Read incoming bytes until full frame is received or timeout occurs
    while (received < FRAME_LEN && millis() - start < 1000) {
      if (Serial2.available()) {
        buffer[received++] = Serial2.read();
      }
    }

    if (received == FRAME_LEN) {
      Serial.println("[FRAME RECEIVED] 308 bytes");

      // ---- Current (bytes 159-160) ----
      // Little-endian signed 16-bit value, scaled by 0.001
      int16_t rawCurrent = (buffer[159] << 8) | buffer[158];
      float current = rawCurrent * 0.001f;

      // ---- State of Charge (byte 174) ----
      // Single-byte percentage value
      uint8_t soc = buffer[173];

      // ---- Voltage (bytes 235-236) ----
      // Little-endian unsigned 16-bit value, scaled by /100
      uint16_t rawVoltage = (buffer[235] << 8) | buffer[234];
      float voltage = rawVoltage / 100.0f;

      // ---- Battery Capacity (bytes 179-180) ----
      // Little-endian unsigned 16-bit value, scaled by /1000 to Ah
      uint16_t rawCapacity = (buffer[179] << 8) | buffer[178];
      float capacityAh = rawCapacity / 1000.0f;

      // ---- Remaining Capacity (bytes 175-176) ----
      // Little-endian unsigned 16-bit value, scaled by /1000 to Ah
      uint16_t rawRemain = (buffer[175] << 8) | buffer[174];
      float remainAh = rawRemain / 1000.0f;

      Serial.printf(
        "Current: %.3f A | SoC: %d%% | Voltage: %.2f V | Capacity: %.3f Ah | Remain: %.3f Ah\n",
        current, soc, voltage, capacityAh, remainAh
      );

    } else {
      Serial.printf("[ERROR] Timeout. Only %d bytes received\n", received);
    }
  }
}
