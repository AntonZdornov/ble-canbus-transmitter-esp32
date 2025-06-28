# hybrid-battery-indicator-arduino

# esp32-obd-soc-ssd1306

ESP32‑C3 project to display hybrid battery level (SOC) on a 128×32 SSD1306 OLED screen via Wi‑Fi OBD-II adapter (ELM327).

## 🚗 Features

- Reads raw SOC data from Toyota hybrid ECU using non-standard PID `01 B5`
- Applies linear calibration to convert raw value to true percentage (0–100%)
- Displays both battery level (graphically and numerically) and engine RPM
- Runs standalone on ESP32‑C3 — no need for phone or computer

## 📦 Hardware Required

- ESP32‑C3 board
- SSD1306 128×32 OLED display (I²C: SDA → GPIO21, SCL → GPIO22)
- Wi‑Fi ELM327 OBD-II adapter (typically at `192.168.0.10:35000`)
- Power: USB or vehicle battery

## 🧮 How SOC Calibration Works

We map raw sensor values to realistic percentages using:


## 🛠️ How It Works

1. ESP32 connects to specified Wi‑Fi network and opens TCP to ELM327 adapter
2. Sends initialization commands (ATZ, ATE0, ATL0, ATH0, ATSP6)
3. Requests raw SOC using `01 B5`, then raw RPM using `01 0C`
4. Parses hex responses (`41 5B XX`, `41 0C AA BB`)
5. Calibrates SOC to percentage, maps RPM value
6. Updates OLED display with battery icon and text

## 🔭 Why This Matters

Toyota hybrids don’t normally expose SOC via standard OBD PIDs. This solution provides a **simple DIY SOC sensor** you can install in your car for real-time feedback — with minimal hardware and no smartphone required.

## ✅ Possible Improvements

- Automatic reconnection to ELM327 on drop-outs
- Adaptive calibration via user input
- Expand display data: voltage, temperature, etc.
- Log data to SPIFFS or MQTT for analytics

## 📚 References

- Adafruit SSD1306 OLED library with ESP32 – Random Nerd Tutorials :contentReference[oaicite:1]{index=1}
- Typical I₂C wiring (SDA → 21, SCL → 22) :contentReference[oaicite:2]{index=2}

---

## 📝 License

MIT © Your Name
