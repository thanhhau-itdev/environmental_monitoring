#include <Wire.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include "ArduinoJson.h"

#include "WiFiManager.h"
#include "pins_config.h"

WiFiManager wifi("^_________^", "Tieunguu09@");

String TEMP = "", PH = "", EC = "", DO = "";

String create_url = "https://htqt.vnkgu.edu.vn/create/insert.php";
String check_upload_firmware = "https://htqt.vnkgu.edu.vn/uploads/check_upload_sensors.php";

unsigned long lastTime_UploadData = 0;
unsigned long lastTime_CheckFirmware = 0;

const unsigned long interval_UploadData = 5000;
const unsigned long interval_CheckFirmware = 60000;

void ota_update(String url) {
  Serial.println("Starting OTA Update...");
  Serial.println(url);

  WiFiClientSecure client;
  client.setInsecure();   // bỏ verify SSL
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Firmware download failed: ");
    Serial.println(httpCode);

    http.end();
    return;
  }

  int contentLength = http.getSize();

  Serial.print("Firmware size: ");
  Serial.println(contentLength);

  if (contentLength <= 0) {
    Serial.println("Invalid firmware size");

    http.end();
    return;
  }

  bool canBegin = Update.begin(contentLength);

  if (!canBegin) {
    Serial.println("Not enough space for OTA");

    http.end();
    return;
  }

  WiFiClient * stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  if (written == contentLength) {
    Serial.println("Firmware written successfully");
  } else {
    Serial.println("Firmware write failed");
  }

  if (Update.end()) {
    if (Update.isFinished()) {
      Serial.println("OTA Update complete!");
      Serial.println("Rebooting...");

      delay(2000);
      ESP.restart();
    } else {
      Serial.println("OTA not finished");
    }
  } else {
    Serial.print("OTA Error #: ");
    Serial.println(Update.getError());
  }

  http.end();
}

void upload_data_sensors(String t, String p, String e, String d) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String postData =
    "temp=" + t +
    "&ph=" + p +
    "&ec=" + e +
    "&do=" + d;

  http.begin(client, create_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(postData);

  Serial.println("Upload sensor data");
  Serial.println("Response code: " + String(response));
  http.end();
}

void check_firmware_status(String f) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String postData = "firmware=" + f;

  http.begin(client, check_upload_firmware);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(postData);

  if (response <= 0) {
    Serial.println("Firmware check failed");
    http.end();
    return;
  }

  String payload = http.getString();
  Serial.println("Server response:");
  Serial.println(payload);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println("JSON parse failed");
    http.end();
    return;
  }

  int update = doc["update"];

  if (update == 1) {
    String firmware_url = doc["firmware"];

    Serial.println("New firmware available:");
    Serial.println(firmware_url);

    ota_update(firmware_url);
  } else {
    Serial.println("Firmware up to date");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  wifi.connect();
  WiFi.setSleep(false);
}

void loop() {
  if (millis() - lastTime_UploadData >= interval_UploadData) {
    lastTime_UploadData = millis();

    TEMP = "1";
    PH = "2";
    EC = "3";
    DO = "4";

    upload_data_sensors(TEMP, PH, EC, DO);
  }

  if (millis() - lastTime_CheckFirmware >= interval_CheckFirmware) {
    lastTime_CheckFirmware = millis();

    check_firmware_status("1");
  }
}
