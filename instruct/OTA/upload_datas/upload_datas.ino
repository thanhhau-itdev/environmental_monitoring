#include "WiFiManager.h"
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "pins_config.h"

#define FIRMWARE_BOARD_READ_SENSOR 1
#define FIRMWARE_BOARD_UPLOAD_DATA 2

WiFiManager wifi("^_________^", "Tieunguu09@");
String check_firmware_url = "http://192.168.1.111/htqt/uploads/upload_firmware.php";

void sendFirmwareUART(String url)
{
  HTTPClient http;
  WiFiClient client;

  Serial.println("Downloading firmware...");

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode != 200)
  {
    Serial.println("Firmware download failed");
    http.end();
    return;
  }

  int firmwareSize = http.getSize();

  if (firmwareSize <= 0)
  {
    Serial.println("Invalid firmware size");
    http.end();
    return;
  }

  Serial.println("Firmware size: " + String(firmwareSize));

  WiFiClient *stream = http.getStreamPtr();
  uint8_t buffer[128];
  int remaining = firmwareSize;

  Serial1.print(firmwareSize);
  Serial1.write('\n');

  unsigned long waitSetup = millis();
  while(Serial1.available() == 0) {
      if(millis() - waitSetup > 5000) break;
      delay(1);
  }
  while(Serial1.available() > 0) Serial1.read();

  while (remaining > 0)
  {
    int len = stream->readBytes(buffer, min(remaining, 128));

    if (len <= 0)
    {
      delay(1);
      continue;
    }

    Serial1.write(buffer, len);
    remaining -= len;

    Serial.println("Remaining: " + String(remaining));

    unsigned long waitAck = millis();
    while(Serial1.available() == 0) {
        if (millis() - waitAck > 3000) break;
        delay(1);
    }
    while(Serial1.available() > 0) Serial1.read(); 
  }

  Serial.println("Firmware sent via UART");
  http.end();
}

void check_firmware_status(String URL, int firmware)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return;
  }
  String postData = "firmware=" + String(firmware);

  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  String payload = http.getString();
  Serial.println("Payload: " + payload);

  if (httpCode == 200)
  {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, payload);
    String file = jsonDoc["file"];
    int status = jsonDoc["status"];

    if (status == 2)
    {
      Serial.println("Send Firmware via UART");
      sendFirmwareUART(file);
    }
  }

  http.end();
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  wifi.connect();

  check_firmware_status(check_firmware_url, FIRMWARE_BOARD_UPLOAD_DATA);
  Serial.println("Hello firmware upload version 1");
}

void loop()
{
}