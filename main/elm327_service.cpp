#include "elm327_service.h"
#include "logger.h"
#include <vector>
#include <wifi.h>

const char *ELM327Socket = "192.168.0.10";
const uint16_t ELM327Port = 35000;
const uint16_t ELM327Timeout = 3000;

bool readSocRaw(WiFiClient &client, uint8_t &soc) {
  if (WiFi.status() != WL_CONNECTED) {
    LOG_PRINTLN("WiFi not connected");
    return false;
  }

  if (!client.connected()) {
    if (!client.connect(ELM327Socket, ELM327Port, ELM327Timeout)) {
      LOG_PRINTLN("ELM327 not connected");
      return false;
    }
    client.print("ATZ\r"); delay(500);
    client.print("ATE0\r"); delay(200);
    client.print("ATL0\r"); delay(200);
    client.print("ATH0\r"); delay(200);
    client.print("ATSP6\r"); delay(500);
  }

  client.print("01 5B\r");
  LOG_PRINTLN("Battery request: <01 5B>");

  unsigned long timeout = millis();
  String line;

  while (millis() - timeout < 1000) {
    if (client.available()) {
      line = client.readStringUntil('\n');
      line.trim();
      LOG_PRINTLN(line);

      line = line.substring(line.indexOf('4'));
      line.trim();

      if (line.startsWith("41 5B")) {
        std::vector<String> parts;
        char *token = strtok(const_cast<char *>(line.c_str()), " ");
        while (token) {
          parts.push_back(String(token));
          token = strtok(NULL, " ");
        }
        if (parts.size() >= 3) {
          soc = strtol(parts[2].c_str(), nullptr, 16);
          LOG_PRINT("SOC parsed: "); LOG_PRINTLN(soc);
          return true;
        }
      }
    }
  }

  LOG_PRINTLN("Timeout waiting for response");
  return false;
}
