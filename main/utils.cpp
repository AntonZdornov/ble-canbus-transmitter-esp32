#include <Arduino.h>
#include "utils.h"

// SOC Parameters
const int rawMin = 0;    // raw ≈ уровень 0%
const int rawMax = 255;  // raw ≈ уровень 100%

int convertBatteryData(int raw) {
  int soc = 0;
  if (raw >= rawMin) {
    soc = (int)(((raw - rawMin) * 100.0 / (rawMax - rawMin)) + 0.5);
    return constrain(soc, 0, 100);
  }

  return -1;
}