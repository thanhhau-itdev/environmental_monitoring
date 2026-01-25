// Chuẩn bị 2 cốc nước
// Cốc 1 nước lạnh ~15 độ
// Cốc 2 nước nóng ~30 độ
// NaOH: 40 g / mol
// 0.5 mol → 20 g NaOH trên 1 lít nước
// Đổ dung dịch vào 2/3 nắp màng
// Đổ quá nhiều -> khi vặn nắp áp suất tăng -> tràn dung dịch ra ngoài -> rách màng
// Đổ quá ít -> xảy ra hiện tượng có bọt khí -> sai giá trị
#include <Arduino.h>

#define VREF 3300
#define RES 4095
#define SAMPLE_COUNT 20

#define TEMP_PIN 4
#define DO_PIN   6

void setup() {
  Serial.begin(115200);
}

void loop() {
  static bool done = false;
  if (done) return;

  uint32_t sumTemp = 0;
  uint32_t sumVolt = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sumTemp += (uint32_t)VREF * analogRead(TEMP_PIN) / RES;
    sumVolt += (uint32_t)VREF * analogRead(DO_PIN)   / RES;
    delay(200);
  }

  float avgTempC = (sumTemp / SAMPLE_COUNT) / 10.0; // LM35
  uint16_t avgVolt = sumVolt / SAMPLE_COUNT;

  Serial.print("Temperature (C): ");
  Serial.print(avgTempC, 1);

  Serial.print("DO Voltage (mV): ");
  Serial.println(avgVolt);

  done = true;
}

