#include <Arduino.h>
#include "display.h"
#include "lvgl_driver.h"
#include "elm327_service.h"
#include "wifi_service.h"
#include "ble_service.h"
#include "logger.h"
#include "utils.h"
#include <WiFi.h>
#include "ui_globals.h"

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>

// WIFISettings
WiFiClient client;

lv_obj_t *battery_container;
lv_obj_t *battery_bar;
lv_obj_t *battery_label = nullptr;
lv_obj_t *logs_label = nullptr;
extern const lv_font_t lv_font_montserrat_30;
extern const lv_font_t lv_font_montserrat_12;

void text() {
  static lv_style_t style;
  lv_style_init(&style);
  logs_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(logs_label, lv_color_white(), 0);
  lv_style_set_text_font(&style, &lv_font_montserrat_12);
  lv_label_set_text(logs_label, "Start");
  lv_obj_set_style_text_font(logs_label, &lv_font_montserrat_12, 0);
  lv_obj_add_style(logs_label, &style, LV_PART_MAIN);
  lv_obj_set_style_text_align(logs_label, LV_TEXT_ALIGN_CENTER, 0);  // центрирование текста
  lv_obj_align(logs_label, LV_ALIGN_CENTER, 0, 0);                   // выравнивание самого объекта
}

void create_battery_icon() {
  // Контейнер для всей батарейки
  battery_container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(battery_container, 40, 80);
  lv_obj_set_style_radius(battery_container, 6, 0);
  lv_obj_set_style_border_width(battery_container, 3, 0);
  lv_obj_set_style_border_color(battery_container, lv_color_white(), 0);
  lv_obj_set_style_bg_color(battery_container, lv_color_white(), 0);
  lv_obj_clear_flag(battery_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(battery_container, LV_ALIGN_BOTTOM_LEFT, 10, -10);

  // Верхняя "шляпка" батареи
  lv_obj_t *cap = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cap, 15, 6);
  lv_obj_set_style_radius(cap, 3, 0);
  lv_obj_set_style_border_width(cap, 0, 0);
  lv_obj_set_style_bg_color(cap, lv_color_white(), 0);
  lv_obj_clear_flag(cap, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align_to(cap, battery_container, LV_ALIGN_OUT_TOP_MID, 0, -2);

  // Бар внутри батареи
  // battery_bar = lv_bar_create(battery_container);
  // lv_obj_set_size(battery_bar, 28, 68);
  // lv_obj_align(battery_bar, LV_ALIGN_BOTTOM_MID, 0, -5);
  // lv_bar_set_range(battery_bar, 0, 100);
  // lv_bar_set_value(battery_bar, 50, LV_ANIM_OFF);
  // lv_obj_set_style_bg_color(battery_bar, lv_color_hex(0xf2c100), LV_PART_INDICATOR);
  // lv_obj_set_style_bg_color(battery_bar, lv_color_white(), LV_PART_MAIN);

  battery_label = lv_label_create(lv_scr_act());
  lv_label_set_text(battery_label, "100%");
  lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
  lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

  // ⚡ Молния (в виде текста)
  lv_obj_t *bolt = lv_label_create(battery_container);
  lv_label_set_text(bolt, LV_SYMBOL_CHARGE);  // ⚡ символ
  lv_obj_set_style_text_color(bolt, lv_color_black(), 0);
  lv_obj_align(bolt, LV_ALIGN_CENTER, 0, 0);
  lv_obj_move_foreground(bolt);
}

// В другом месте можно обновлять заряд:
void update_battery(uint8_t soc) {
  lv_color_t color;

  if (soc >= 80) {
    color = lv_color_hex(0x00CFFF);  // Голубой
  } else if (soc >= 45) {
    color = lv_color_hex(0x00FF00);  // Зелёный
  } else if (soc >= 25) {
    color = lv_color_hex(0xFFFF00);  // Жёлтый
  } else {
    color = lv_color_hex(0xFF0000);  // Красный
  }

  // Бар внутри батареи (заливка)
  battery_bar = lv_bar_create(battery_container);
  lv_obj_set_size(battery_bar, 30, 80);
  lv_obj_align(battery_bar, LV_ALIGN_BOTTOM_MID, 0, 15);  // выравнивание снизу
  lv_bar_set_range(battery_bar, 0, 100);
  lv_bar_set_value(battery_bar, soc, LV_ANIM_OFF);

  // Обязательно задать NORMAL режим
  lv_bar_set_mode(battery_bar, LV_BAR_MODE_NORMAL);  // 💡 обязательно!

  // Стили
  lv_obj_set_style_bg_color(battery_bar, lv_color_black(), LV_PART_MAIN);  // фон
  lv_obj_set_style_bg_color(battery_bar, color, LV_PART_INDICATOR);        // индикатор
  lv_obj_set_style_radius(battery_bar, 0, LV_PART_INDICATOR);              // убираем скругление
  lv_obj_set_style_radius(battery_bar, 0, LV_PART_MAIN);

  static lv_style_t style_large;
  lv_style_init(&style_large);
  lv_style_set_text_font(&style_large, &lv_font_montserrat_30);
  char buf[6];
  snprintf(buf, sizeof(buf), "%d%%", soc);
  lv_label_set_text(battery_label, buf);
  lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_30, 0);
  lv_obj_add_style(battery_label, &style_large, LV_PART_MAIN);
}

void background() {
  static lv_style_t style_scr;
  lv_style_init(&style_scr);
  lv_style_set_bg_color(&style_scr, lv_color_black());
  lv_style_set_bg_opa(&style_scr, LV_OPA_COVER);
  lv_obj_add_style(lv_scr_act(), &style_scr, 0);
}

void setup() {
  LOG_BEGIN(115200);
  LCD_Init();
  Lvgl_Init();

  text();

  initWifi();
  log_message("Start BLE");
  initBLE();


  background();
  create_battery_icon();
  update_battery(44);
}

void loop() {
  Timer_Loop();

  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 1000) {
    uint8_t soc = 0;
    if (readSocRaw(client, soc)) {
      uint8_t normalized = convertBatteryData(soc);
      sendSOC(normalized);
      update_battery(normalized);
    }
    lastQuery = millis();
  }

  delay(5);
}
