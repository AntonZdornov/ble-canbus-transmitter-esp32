#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>

bool debugMode = false;

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
    displayText("Connecting...", 1);
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
  initWifi();
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
  if (debugMode) Serial.println("Battery request: <01 5B>");

  unsigned long timeout = millis();
  String line;
  while (millis() - timeout < 1000) {
    Serial.println("Проверка доступности клиента");
    if (client.available()) {
      if (debugMode) Serial.println("Клиент доступен");
      String line = client.readStringUntil('\n');
      line.trim();  // убираем пробелы и символы вокруг
      if (debugMode) Serial.println(line);

      // Убираем ведущие символы '>' и пробелы
      line = line.substring(line.indexOf('4'));  // начиная с первого '4'
      line.trim();

      // Проверяем начинается ли с "41 5B"
      if (line.startsWith("41 5B")) {
        // Разбираем строку на части
        std::vector<String> parts;
        char *token = strtok(const_cast<char *>(line.c_str()), " ");
        while (token) {
          parts.push_back(String(token));
          token = strtok(NULL, " ");
        }
        if (parts.size() >= 3) {
          raw = strtol(parts[2].c_str(), nullptr, 16);
          if (debugMode) Serial.println(raw);
          return;
        }
      }
    }
  }

  if (debugMode) Serial.println("Timeout waiting for response");
  return;
}

void displayText(String message, int textSize) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 15);  // x , y
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
  displayText(String(batteryLevel), 4);
  delay(1000);
  sentMessage(batteryLevel);
  displayText(String(batteryLevel) + "%", 4);
  delay(500);
}
