/*
 * Serial Monitor:
 *   enterec -> vào chế độ hiệu chuẩn
 *   calec -> hiệu chuẩn với dung dịch đệm chuẩn, hai dung dịch đệm (1413us/cm và 12.88ms/cm) sẽ được tự động nhận diện
 *   exitec -> lưu các thông số đã hiệu chuẩn và thoát khỏi chế độ hiệu chuẩn
 * Nếu không có cảm biến nhiệt độ gán cứng biến temp thành nhiệt độ đo thực tế.
*/

#include "LM35_Sensor.h"
#include "EC_Sensor.h"

LM35_Sensor lm35(4);
EC_Sensor   ecSensor(2);
EC_Sensor::EC_Data ecData;

unsigned long lastTime_TEMP = 0;
unsigned long lastTime_EC   = 0;

const unsigned long interval_TEMP = 1000;
const unsigned long interval_EC   = 1000;

uint8_t temp = 0;

void setup()
{
  Serial.begin(115200);
  ecSensor.begin();
}

void loop()
{
  unsigned long now = millis();

  if (now - lastTime_TEMP >= interval_TEMP) {
    lastTime_TEMP = now;
    temp = lm35.readTemperature();
  }

  if (now - lastTime_EC >= interval_EC) {
    lastTime_EC = now;
    ecSensor.update(ecData, temp);

    Serial.print("Temp: "); Serial.print(temp); Serial.print(" | ");
    Serial.print("EC voltage: "); Serial.print(ecData.voltage); Serial.print(" | ");
    Serial.print("EC value: "); Serial.println(ecData.value, 2);
  }

  ecSensor.calibration(temp);
}
