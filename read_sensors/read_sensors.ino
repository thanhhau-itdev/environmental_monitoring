#include <Arduino.h>
#include "LM35_Sensor.h"
#include "PH_Sensor.h"
#include "EC_Sensor.h"
#include "DO_Sensor.h"
#include <Update.h>
#include "pins_config.h"

#define FIRMWARE_MIN_SIZE 10000
#define FIRMWARE_MAX_SIZE 0x180000   // kích thước phân vùng app trong partitions.csv
#define CMD_BUFFER_SIZE   16

bool receivingFirmware = false;
int firmwareSize = 0;
int receivedBytes = 0;
uint8_t buffer[128];
unsigned long lastReceive = 0;

char cmdBuffer[CMD_BUFFER_SIZE];
uint8_t cmdIndex = 0;

LM35_Sensor lm35(TEMP_PIN);
DO_Sensor doSensor(DO_PIN);
PH_Sensor phSensor(PH_PIN);
EC_Sensor ecSensor(EC_PIN);

DO_Sensor::DO_Data doData;
PH_Sensor::PH_Data phData;
EC_Sensor::EC_Data ecData;

unsigned long lastTime_TEMP = 0, lastTime_DO = 0,
              lastTime_PH = 0, lastTime_EC = 0,
              lastTime_Serial = 0, lastTime_SendData = 0;

const unsigned long interval_TEMP = 1000, interval_DO = 1000,
                    interval_PH = 1000, interval_EC = 1000,
                    interval_Serial = 1000, interval_SendData = 5000;

float temp = 0;
float DO_value = 0;
float PH_value = 0, EC_value = 0;

// Chỉ chấp nhận chuỗi toàn chữ số, tránh hiểu nhầm dữ liệu rác thành kích thước firmware
bool isNumber(const String &s)
{
  if (s.length() == 0) return false;
  for (unsigned int i = 0; i < s.length(); i++)
  {
    if (!isDigit(s[i])) return false;
  }
  return true;
}

// Đọc một dòng lệnh hiệu chuẩn từ Serial rồi chuyển cho cả pH và EC,
// để thư viện pH không đọc mất lệnh của EC
void handleCalibrationCommand()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == '\r') continue;

    if (c == '\n' || cmdIndex == CMD_BUFFER_SIZE - 1)
    {
      cmdBuffer[cmdIndex] = '\0';
      if (cmdIndex > 0)
      {
        phSensor.calibration(temp, cmdBuffer);
        ecSensor.calibration(temp, cmdBuffer);
      }
      cmdIndex = 0;
    }
    else
    {
      cmdBuffer[cmdIndex++] = c;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  Serial1.setRxBufferSize(16384);
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  ecSensor.begin();
  phSensor.begin();
}

void loop()
{
  unsigned long now = millis();

  if (!receivingFirmware)
  {
    if (Serial1.available())
    {
      String sizeStr = Serial1.readStringUntil('\n');
      sizeStr.trim();
      if (!isNumber(sizeStr)) return;

      firmwareSize = sizeStr.toInt();
      if (firmwareSize < FIRMWARE_MIN_SIZE || firmwareSize > FIRMWARE_MAX_SIZE) return;

      Serial.println("Firmware size: " + String(firmwareSize));

      if (!Update.begin(firmwareSize))
      {
        Serial.println("Update begin failed");
        return;
      }

      receivingFirmware = true;
      receivedBytes = 0;
      lastReceive = millis();

      Serial1.print("K");
    }
  }
  else
  {
    int remaining = firmwareSize - receivedBytes;
    int available = Serial1.available();

    if (available > 0)
    {
      int expected = min(remaining, 128);
      int len = Serial1.readBytes(buffer, expected);

      if (len > 0)
      {
        if (Update.write(buffer, len) != (size_t)len)
        {
          Serial.println("Update write failed");
          receivingFirmware = false;
          Update.abort();
          return;
        }
        receivedBytes += len;
        lastReceive = millis();

        Serial1.print("K");
      }
    }

    if (millis() - lastReceive > 8000)
    {
      Serial.println("UART Timeout");
      receivingFirmware = false;
      Update.abort();
      return;
    }

    if (receivedBytes >= firmwareSize)
    {
      if (Update.end(true))
      {
        Serial.println("Update Success");
        ESP.restart();
      }
      else
      {
        Serial.println("Update Failed");
      }

      receivingFirmware = false;
    }
  }

  if (now - lastTime_TEMP >= interval_TEMP) {
    lastTime_TEMP = now;
    temp = lm35.readTemperature();
  }

  if (now - lastTime_DO >= interval_DO) {
    lastTime_DO = now;
    // Bảng bão hòa DO tra theo từng độ C nên làm tròn nhiệt độ
    doSensor.update(doData, (uint8_t)lroundf(constrain(temp, 0.0f, 40.0f)));

    DO_value = doData.value;
  }

  if (now - lastTime_PH >= interval_PH) {
    lastTime_PH = now;
    phSensor.update(phData, temp);
    PH_value = phData.value;
  }

  if (now - lastTime_EC >= interval_EC) {
    lastTime_EC = now;
    ecSensor.update(ecData, temp);

    EC_value = ecData.value;
  }

  if (now - lastTime_Serial >= interval_Serial) {
    lastTime_Serial = now;

    Serial.print("Temp: "); Serial.print(temp, 1);
    Serial.print(" DO: "); Serial.print(DO_value, 2);
    Serial.print(" PH: "); Serial.print(PH_value, 2);
    Serial.print(" EC: "); Serial.println(EC_value, 2);
  }

  // Không gửi số liệu khi đang nhận firmware, board upload sẽ hiểu nhầm là ACK
  if (!receivingFirmware && now - lastTime_SendData >= interval_SendData) {
    lastTime_SendData = now;
    Serial1.println("temp=" + String(temp, 1) +
                    "&ph=" + String(PH_value, 2) +
                    "&ec=" + String(EC_value, 2) +
                    "&do=" + String(DO_value, 2) +
                    "&ota=" + "TN_TEST"
                  );
  }

  handleCalibrationCommand();
}
