#include "ble_service.h"

NimBLEServer *pServer = nullptr;
static NimBLECharacteristic* pCharacteristic_soc = nullptr;
static NimBLECharacteristic* pCharacteristic_rpm = nullptr;

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID_SOC "abcd1234-abcd-1234-abcd-1234567890ab"
#define CHARACTERISTIC_UUID_RPM "abcd1234-abcd-1234-abcd-1234567890ac"

// old Mac address: 70:04:1D:38:75:76
// new Mac address: A0:85:E3:AE:B6:62

void initBLE() {
  NimBLEDevice::init("BLEC");
  pServer = NimBLEDevice::createServer();

  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic_soc = pService->createCharacteristic(
    CHARACTERISTIC_UUID_SOC,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );

  pCharacteristic_rpm = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RPM,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );

  pService->start();
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("The BLE initialized successfully.");
}

void sendSOC(uint8_t socValue) {
  if (pCharacteristic_soc) {
    pCharacteristic_soc->setValue(socValue);
    pCharacteristic_soc->notify();
    Serial.print("Updated SOC: ");
    Serial.println(socValue);
  }
}

void sendRPM(uint8_t rpmValue) {
  if (pCharacteristic_rpm) {
    pCharacteristic_rpm->setValue(rpmValue);
    pCharacteristic_rpm->notify();
    Serial.print("Updated RPM: ");
    Serial.println(rpmValue);
  }
}