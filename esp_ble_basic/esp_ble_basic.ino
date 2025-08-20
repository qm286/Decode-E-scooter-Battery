#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUID BLE UART (chuẩn Nordic)
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callback khi client kết nối/ngắt kết nối
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

  BLEDevice::init("ESP32_SIMPLE"); // Không cần bảo mật

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
  Serial.println("BLE Advertising started.");
}

void loop() {
  if (deviceConnected) {
    uint32_t tick = millis();
    char hexStr[9];
    sprintf(hexStr, "%08X", tick); // 8 ký tự HEX

    pCharacteristic->setValue((uint8_t*)hexStr, 8);
    pCharacteristic->notify();

    Serial.print("Sent HEX tick: ");
    Serial.println(hexStr);

    delay(400);
  }
}
