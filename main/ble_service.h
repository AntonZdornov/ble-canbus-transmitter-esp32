#pragma once
#include <NimBLEDevice.h>

// Инициализация BLE и характеристик
void initBLE();

// Уведомление BLE-клиенту
void sendSOC(uint8_t socValue);
void sendRPM(uint8_t rpmValue);