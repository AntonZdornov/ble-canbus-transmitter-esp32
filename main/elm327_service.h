#pragma once
#include <WiFiClient.h>

// Вызывается в loop для чтения SOC
// Возвращает true, если удалось получить значение
bool readSocRaw(WiFiClient &client, uint8_t &soc);