/*
  Basic ESP32 BLE Notify Example
  ------------------------------
  Purpose:
  This sketch creates a simple BLE peripheral on the ESP32 to test whether
  BLE advertising, connection, and notification transmission are working correctly.

  What it does:
  - Initializes the ESP32 as a BLE server
  - Advertises a Nordic UART-style BLE service
  - Creates a notify-only characteristic
  - Waits for a client (such as an Android app) to connect
  - Sends an 8-character hexadecimal value based on millis() every 400 ms
  - Prints connection status and transmitted data to the Serial Monitor

  Why this is useful:
  This is a minimal BLE test before integrating real telemetry such as speed,
  battery, temperature, or RS485-decoded data. It helps confirm that the BLE
  communication path works end to end before more complex logic is added.

  Output behavior:
  - Serial Monitor shows connection and disconnection events
  - Serial Monitor shows each transmitted HEX tick
  - BLE client receives periodic notifications while connected
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UART UUIDs (Nordic UART standard)
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callback for client connection/disconnection
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client disconnected.");
    pServer->startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE...");

  BLEDevice::init("ESP32_SIMPLE"); // No security required

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
  Serial.println("BLE advertising started.");
}

void loop() {
  if (deviceConnected) {
    uint32_t tick = millis();
    char hexStr[9];
    sprintf(hexStr, "%08X", tick); // 8-character HEX string

    pCharacteristic->setValue((uint8_t*)hexStr, 8);
    pCharacteristic->notify();

    Serial.print("Sent HEX tick: ");
    Serial.println(hexStr);

    delay(400);
  }
}
