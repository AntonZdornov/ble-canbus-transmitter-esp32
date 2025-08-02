#include "logger.h"
#include "wifi_service.h"
#include <WiFi.h>
#include "ui_globals.h"

const char *ssid = "V-LINK";
const char *password = "";

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  // WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  bool animation = true;

  LOG_PRINTLN("Wifi");
  // log_message("Wifi Connecting");

  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 30000;  // 30 секунд
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    LOG_PRINT(".");
    // log_message(animation ? "Connecting.." : "Connecting...");

    animation = !animation;
  }
  if (WiFi.status() != WL_CONNECTED) {
    LOG_PRINTLN("Wifi not Connected");
    // log_message("Wifi not Connected");
    delay(2000);
    return;
  }

  LOG_PRINTLN("Connected to WiFi");
  LOG_PRINTLN("The WiFi initialized successfully.");
  // log_message("Connected to V-LINK");
  delay(500);
}