#include <Arduino.h>
#include <Update.h>
#include "pins_config.h"

bool receivingFirmware = false;
int firmwareSize = 0;
int receivedBytes = 0;
uint8_t buffer[128];
unsigned long lastReceive = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello firmware sensor version 2");

  Serial1.setRxBufferSize(16384); 
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop()
{
  if (!receivingFirmware)
  {
    if (Serial1.available())
    {
      String sizeStr = Serial1.readStringUntil('\n');
      sizeStr.trim();
      firmwareSize = sizeStr.toInt();
      if (firmwareSize < 10000) return;

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
        Update.write(buffer, len);
        receivedBytes += len;

        Serial.println(receivedBytes);
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
}