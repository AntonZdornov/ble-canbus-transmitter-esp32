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
// #include "car.h"

// LV_IMG_DECLARE(car_image);

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>

// WIFISettings
WiFiClient client;

lv_obj_t *root_container;
lv_obj_t *time_label_title;
lv_obj_t *time_label;
lv_obj_t *soc_arc = nullptr;
lv_obj_t *soc_label = nullptr;
lv_obj_t *logs_label = nullptr;
extern const lv_font_t lv_font_montserrat_48;
extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t lv_font_montserrat_22;

unsigned long start_time;
void crete_ui() {
  // lv_obj_t* act = lv_scr_act();
  // lv_obj_clean(act);      // очищаем детей
  // lv_obj_del(act);        // удаляем сам экран
  // lv_disp_load_scr(lv_obj_create(NULL));

  // Контейнер со столбиком
  root_container = lv_obj_create(lv_scr_act());
  lv_obj_clean(root_container);                               // очищаем детей
  lv_obj_set_size(root_container, LV_PCT(100), LV_PCT(100));  // под размер экрана

  // Убираем отступы и границы
  lv_obj_set_style_pad_all(root_container, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(root_container, 0, LV_PART_MAIN);

  // Задаем черный фон
  lv_obj_set_style_bg_color(root_container, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(root_container, LV_OPA_COVER, LV_PART_MAIN);

  // Раскладка флекс колонкой, выравнивание вниз по центру
  lv_obj_set_layout(root_container, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(root_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(root_container,
                        LV_FLEX_ALIGN_CENTER,         // горизонтально по центру
                        LV_FLEX_ALIGN_SPACE_BETWEEN,  // вертикально вниз
                        LV_FLEX_ALIGN_CENTER);

  lv_obj_t *timer_group = lv_obj_create(root_container);
  lv_obj_set_height(timer_group, LV_PCT(20));  // или LV_SIZE_CONTENT + grow
  lv_obj_set_width(timer_group, LV_PCT(100));  // если нужно
  lv_obj_set_style_pad_all(timer_group, 0, LV_PART_MAIN);
  lv_obj_set_flex_grow(timer_group, 1);

  lv_obj_set_layout(timer_group, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(timer_group, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(timer_group,
                        LV_FLEX_ALIGN_CENTER,  // по горизонтали
                        LV_FLEX_ALIGN_CENTER,  // по вертикали
                        LV_FLEX_ALIGN_CENTER);

  lv_obj_set_style_bg_color(timer_group, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(timer_group, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(timer_group, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(timer_group, 0, LV_PART_MAIN);

  time_label_title = lv_label_create(timer_group);
  lv_label_set_text(time_label_title, "Drive time:");
  lv_obj_set_width(time_label_title, LV_PCT(100));
  lv_obj_set_style_text_align(time_label_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(time_label_title, lv_color_white(), 0);
  lv_obj_set_style_text_font(time_label_title, &lv_font_montserrat_22, 0);
  lv_obj_set_style_pad_top(time_label_title, 5, LV_PART_MAIN);  // Отступ сверху

  time_label = lv_label_create(timer_group);
  lv_label_set_text(time_label, "00:00");
  lv_obj_set_width(time_label, LV_PCT(100));
  lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_22, 0);
  // lv_obj_set_style_pad_top(time_label, 5, LV_PART_MAIN);  // Отступ сверху
  start_time = millis();

  lv_obj_t *arc_group = lv_obj_create(root_container);
  lv_obj_set_height(arc_group, LV_PCT(40));  // или LV_SIZE_CONTENT + grow
  lv_obj_set_width(arc_group, LV_PCT(100));  // если нужно
  lv_obj_set_style_pad_all(arc_group, 0, LV_PART_MAIN);
  lv_obj_set_flex_grow(arc_group, 1);

  lv_obj_set_layout(arc_group, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(arc_group, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(arc_group,
                        LV_FLEX_ALIGN_CENTER,  // по горизонтали
                        LV_FLEX_ALIGN_CENTER,  // по вертикали
                        LV_FLEX_ALIGN_CENTER);

  // Чёрный фон
  lv_obj_set_style_bg_color(arc_group, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(arc_group, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(arc_group, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(arc_group, 0, LV_PART_MAIN);

  // 1. Надпись "Hybrid" над кругом
  lv_obj_t *label = lv_label_create(arc_group);
  lv_label_set_text(label, "THS");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align_to(label, soc_arc, LV_ALIGN_OUT_TOP_MID, 8, 0);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_22, 0);

  // 2. Создаём круговой индикатор
  uint8_t soc = 33;
  soc_arc = lv_arc_create(arc_group);
  lv_obj_set_size(soc_arc, 100, 100);  // Размер круга
  lv_arc_set_rotation(soc_arc, 90);    // Начальная точка сверху

  // Настраиваем углы дуги: от 270° (вниз) до 270+360 = 630°
  // Это визуально по часовой стрелке круг
  lv_arc_set_bg_angles(soc_arc, 0, 360);  // Полный круг
  // lv_arc_set_angles(soc_arc, 270, 270 + 270);  // пример: от 270 до 540

  lv_arc_set_range(soc_arc, 0, 100);             // Диапазон значений
  lv_arc_set_value(soc_arc, soc);                // Текущее значение
  lv_arc_set_mode(soc_arc, LV_ARC_MODE_NORMAL);  // Режим

  // убираем точку
  lv_obj_set_style_arc_rounded(soc_arc, false, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(soc_arc, false, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(soc_arc, LV_OPA_TRANSP, LV_PART_KNOB);

  // lv_obj_center(soc_arc);                            // Позиционируем по центру
  // lv_obj_align(soc_arc, LV_ALIGN_BOTTOM_MID, 0, 0);

  // 2. Стили для фона
  lv_obj_set_style_arc_width(soc_arc, 5, LV_PART_MAIN);
  lv_obj_set_style_arc_color(soc_arc, lv_color_hex(0x444444), LV_PART_MAIN);

  // 3. Стили для индикатора
  lv_color_t color = lv_color_hex(0x00FF00);  // По умолчанию зелёный

  if (soc >= 80) color = lv_color_hex(0x00CFFF);       // Голубой
  else if (soc >= 45) color = lv_color_hex(0x00FF00);  // Зелёный
  else if (soc >= 25) color = lv_color_hex(0xFFFF00);  // Жёлтый
  else color = lv_color_hex(0xFF0000);                 // Красный

  lv_obj_set_style_arc_color(soc_arc, color, LV_PART_INDICATOR);

  // 4. Добавим текст в центр круга
  soc_label = lv_label_create(soc_arc);
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%", soc);
  lv_label_set_text(soc_label, buf);
  lv_obj_center(soc_label);
  lv_obj_set_style_text_font(soc_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(soc_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // (опционально) Убираем возможность крутить вручную
  lv_obj_clear_flag(soc_arc, LV_OBJ_FLAG_CLICKABLE);
}

void initUILOGS() {
  static lv_style_t style;
  lv_style_init(&style);
  logs_label = lv_label_create(root_container);
  lv_obj_set_style_text_color(logs_label, lv_color_white(), 0);
  lv_style_set_text_font(&style, &lv_font_montserrat_12);
  lv_obj_set_style_text_font(logs_label, &lv_font_montserrat_12, 0);
  lv_obj_add_style(logs_label, &style, LV_PART_MAIN);
  lv_obj_set_style_text_align(logs_label, LV_TEXT_ALIGN_CENTER, 0);  // центрирование текста
  lv_obj_align(logs_label, LV_ALIGN_CENTER, 0, -10);                 // выравнивание самого объекта
}

void update_soc_circle(uint8_t soc) {
  if (!soc_arc || !soc_label) return;  // проверяем, что элементы созданы

  // Обновим значение на круге
  lv_arc_set_value(soc_arc, soc);

  // Обновим цвет дуги в зависимости от уровня
  lv_color_t color = lv_color_hex(0x00FF00);  // по умолчанию зелёный

  if (soc >= 80) color = lv_color_hex(0x00CFFF);
  else if (soc >= 45) color = lv_color_hex(0x00FF00);
  else if (soc >= 25) color = lv_color_hex(0xFFFF00);
  else color = lv_color_hex(0xFF0000);

  lv_obj_set_style_arc_color(soc_arc, color, LV_PART_INDICATOR);

  // Обновим текст по центру
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%", soc);
  lv_label_set_text(soc_label, buf);
}

void update_time_label() {
  unsigned long now = millis();
  unsigned long seconds = (now - start_time) / 1000;

  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;

  static char buf[9];  // Формат HH:MM:SS
  snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);

  lv_label_set_text(time_label, buf);
}

void setup() {
  LOG_BEGIN(115200);

  LCD_Init();
  Lvgl_Init();
  crete_ui();

  initWifi();
  initBLE();

  // initUILOGS();
  // log_message("Loading...");

  // Таймер обновления времени
  lv_timer_create([](lv_timer_t *t) {
    update_time_label();
  },
                  1000, NULL);
}

void loop() {
  Timer_Loop();

  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 1000) {
    uint8_t soc = 0;
    if (readSocRaw(client, soc)) {
      uint8_t normalized = convertBatteryData(soc);
      sendSOC(normalized);
      update_soc_circle(normalized);
    }
    lastQuery = millis();
  }

  delay(5);
}
