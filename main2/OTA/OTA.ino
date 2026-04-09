#include "WiFiManager.h"
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <HTTPUpdate.h>

#define STATUS 1
WiFiManager wifi("dev", "0788806282");
String check_firmware_url = "http://10.235.152.242/htqt/uploads/upload_firmware.php";

void updateFirmware(String url) {
  WiFiClient client;

  switch (httpUpdate.update(client, url)) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Update Failed. Error (%d): %s\n",
                    httpUpdate.getLastError(),
                    httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No Update.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update Success. Rebooting...");
      break;
  }
}

void check_firmware_status(String URL, int firmware) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  String postData = "firmware=" + firmware;

  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);
  String payload = http.getString();

  Serial.println("URL: " + URL);
  Serial.println("Data: " + postData);
  Serial.println("HTTP Code: " + String(httpCode));
  Serial.println("Payload: " + payload);

  if (httpCode == 200) {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, payload);

    String file = jsonDoc["file"];
    updateFirmware(file);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  wifi.connect();

  check_firmware_status(check_firmware_url, STATUS);
  Serial.println("Hello version 1");
}

void loop() {}
