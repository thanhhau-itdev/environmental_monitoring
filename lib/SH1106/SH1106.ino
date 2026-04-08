#include <Wire.h>
#include "SH1106.h"

SH1106 oled;

unsigned long lastTime_LCD = 0;
const unsigned long interval_LCD = 1000;

void setup() {
  Wire.begin(2, 3);

  if (!oled.begin(0x3C)) {
      Serial.println(F("OLED init failed!"));
      while (1);
  }
  oled.clear();
}

void loop() {
  unsigned long now = millis();

  if (now - lastTime_LCD >= interval_LCD) {
    lastTime_LCD = now;

    oled.clear();
     oled.showText(1, 0, 8, "Temp");
    oled.showText(1, 0, 16, "DO");
    oled.showText(1, 0, 24, "PH");
    oled.showText(1, 0, 32, "EC");
    oled.refresh();
  }
}
