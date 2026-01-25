#include <Arduino.h>
#include <Wire.h>
#include "SH1106.h"

void setup() {
  Serial.begin(115200);
  Wire.begin(25, 26);

  if (!oled.begin(0x3C)) {
    Serial.println(F("OLED init failed!"))
    while (1);
  }
  oled.clear();
}

void loop() {
  oled.showText(1, 12, 8, "Hello world !");
  oled.showText(1, 12, 16, "Hello world !");
  oled.showText(1, 12, 24, "Hello world !");
  oled.showText(1, 12, 32, "Hello world !");
  oled.showText(1, 12, 40, "Hello world !");
  oled.showText(1, 12, 48, "Hello world !");
  oled.showText(1, 12, 56, "Hello world !");

  delay(2000);
}