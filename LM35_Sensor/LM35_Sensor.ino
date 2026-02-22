#include "LM35_Sensor.h"

LM35_Sensor lm35(4);
unsigned long lastTime_TEMP = 0;
const unsigned long interval_TEMP = 1000;

uint8_t temp = 0;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  unsigned long now = millis();

  if (now - lastTime_TEMP >= interval_TEMP) {
    lastTime_TEMP = now;
    temp = lm35.readTemperature();

    Serial.print("Temp: "); Serial.println(temp);
  }
}
