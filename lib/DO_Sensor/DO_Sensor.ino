#include "LM35_Sensor.h"
#include "DO_Sensor.h"
#include "pins_config.h"

LM35_Sensor lm35(TEMP_PIN);
DO_Sensor doSensor(DO_PIN);
DO_Sensor::DO_Data doData;

unsigned long lastTime_TEMP = 0;
unsigned long lastTime_DO   = 0;

const unsigned long interval_TEMP = 1000;
const unsigned long interval_DO   = 2000;

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
  }

  if (now - lastTime_DO >= interval_DO) {
    lastTime_DO = now;
    doSensor.update(doData, temp);

    Serial.print("Temp: "); Serial.print(temp); Serial.print(" | ");
    Serial.print("DO value: "); Serial.println(doData.value);
  }
}


