/*
 * Serial Monitor:
 * enterph -> vào chế độ hiệu chuẩn
 * calph   -> hiệu chuẩn với dung dịch đệm chuẩn, hai dung dịch đệm (4.0 và 7.0) sẽ được tự động nhận diện
 * exitph  -> ưu các thông số đã hiệu chuẩn và thoát khỏi chế độ hiệu chuẩn
*/

#include "PH_Sensor.h"
#include "LM35_Sensor.h"
#include "pins_config.h"

LM35_Sensor lm35(TEMP_PIN);
PH_Sensor phSensor(PH_PIN);
PH_Sensor::PH_Data phData;

unsigned long lastTime_TEMP = 0;
unsigned long lastTime_PH   = 0;

const unsigned long interval_TEMP = 1000;
const unsigned long interval_PH   = 1000;

uint8_t temp = 0;

void setup()
{
  Serial.begin(115200);
  phSensor.begin();
}

void loop()
{
  unsigned long now = millis();

  if (now - lastTime_TEMP >= interval_TEMP) {
    lastTime_TEMP = now;
    temp = lm35.readTemperature();
  }

  if (now - lastTime_PH >= interval_PH) {
    lastTime_PH = now;
    phSensor.update(phData, temperature);

    Serial.print("Temp: "); Serial.print(temp); Serial.print(" | ");
    Serial.print("PH voltage: "); Serial.print(phData.voltage); Serial.print(" | ");
    Serial.print("PH value: "); Serial.print(phData.value, 2); Serial.println(" ");
  }

  phSensor.calibration(temperature);
}
