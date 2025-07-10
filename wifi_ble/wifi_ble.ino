#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>

bool debugMode = true;

bool disableWifi = false;
bool disableDisplay = false;
bool disableBluetooth = false;

//Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
const int SDA_PIN = 8;
const int SCL_PIN = 9;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WIFISettings
WiFiClient client;
const char *ssid = "V-LINK";
const char *password = "";
const char *ELM327Socket = "192.168.0.10";
const uint16_t ELM327Port = 35000;
const uint16_t ELM327Timeout = 3000;

//SOC Parameters
const int rawMin = 0;    // raw ≈ уровень 0%
const int rawMax = 255;  // raw ≈ уровень 100%
int raw = 0;

NimBLEServer *pServer;
NimBLECharacteristic *pCharacteristic;

// UUID сервиса и характеристики
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-1234567890ab"
// mac address: 70:04:1D:38:75:76

void initDisplay() {
  if (disableDisplay) {
    return;
  }

  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (debugMode) Serial.println(F("SSD1306 failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();

  if (debugMode) Serial.println("The display initialized successfully.");
}

void initWifi() {
  if (disableWifi) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debugMode) Serial.print(".");
  }
  if (debugMode) Serial.println("Connected to WiFi");

  if (debugMode) Serial.println("The WiFi initialized successfully.");
}

void initBLE() {
  if (disableBluetooth) {
    return;
  }

  NimBLEDevice::init("Toyota-Battery");  // Название BLE устройства
  pServer = NimBLEDevice::createServer();

  // Создание сервиса и характеристики
  NimBLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);


  // Начальное значение SOC (например, -1%)
  uint8_t soc = 0;
  pCharacteristic->setValue(&soc, 1);

  pService->start();
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  NimBLEDevice::startAdvertising();

  if (debugMode) Serial.println("The BLE initialized successfully.");
}

void setup() {
  Serial.begin(115200);

  initDisplay();
  // initWifi();
  initBLE();
}

void sentMessage(uint8_t message) {
  pCharacteristic->setValue(message);
  pCharacteristic->notify();
  if (debugMode) {
    Serial.print("Updated SOC: ");
    Serial.println(message);
  }
}

int convertBatteryData() {
  int soc = 0;
  if (raw >= rawMin) {
    soc = (int)(((raw - rawMin) * 100.0 / (rawMax - rawMin)) + 0.5);
    return constrain(soc, 0, 100);
  }

  return -1;
}

void readSocRaw() {
  if (WiFi.status() != WL_CONNECTED) {
    if (debugMode) Serial.println("WiFi not connected");
    return;
  }

  if (!client.connected()) {
    if (!client.connect(ELM327Socket, ELM327Port, ELM327Timeout)) {
      if (debugMode) Serial.println("ELM327 not connected");
      return;
    }
    // Инициализация
    client.print("ATZ\r");
    delay(500);
    client.print("ATE0\r");
    delay(200);
    client.print("ATL0\r");
    delay(200);
    client.print("ATH0\r");
    delay(200);
    client.print("ATSP6\r");
    delay(500);
  }

  // Отправка запроса
  client.print("01 5B\r");
  Serial.println("Battery request: <01 5B>");

  unsigned long timeout = millis();
  char buffer[128];  // фиксированный буфер
  size_t idx = 0;

  while (millis() - timeout < 1000) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n' || idx >= sizeof(buffer) - 1) {
        buffer[idx] = '\0';  // завершение строки
        idx = 0;

        // Чистим строку от пробелов и символов '>'
        char *line = buffer;
        while (*line == ' ' || *line == '>') line++;  // убираем лишнее

        if (debugMode) {
          Serial.print("Received: ");
          Serial.println(line);
        }

        // Проверка на "41 5B"
        if (strncmp(line, "41 5B", 5) == 0) {
          // Разбиваем строку на токены
          char *token;
          int tokenIndex = 0;
          uint8_t parsedValue = 0;

          token = strtok(line, " ");
          while (token != nullptr) {
            if (tokenIndex == 2) {  // третий токен — нужный байт
              parsedValue = strtol(token, nullptr, 16);
              if (parsedValue >= 0 && parsedValue <= 255) raw = parsedValue;  // сохраняем значение
              if (debugMode) {
                Serial.print("Parsed raw value: ");
                Serial.println(raw);
              }
              return;
            }
            token = strtok(nullptr, " ");
            tokenIndex++;
          }
        }
      } else if (c != '\r') {
        buffer[idx++] = c;  // копируем в буфер
      }
    }
  }

  if (debugMode) Serial.println("Timeout waiting for response");
  return;
}

void displayText(String message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.print(message);
  display.display();
}

void loop() {
  readSocRaw();
  int batteryLevel = convertBatteryData();
  if (debugMode) {
    Serial.print("new val: ");
    Serial.println(batteryLevel);
  }
  displayText("------>");
  delay(1000);
  sentMessage(batteryLevel);
  displayText(String(batteryLevel) + "%");
  delay(500);
}
