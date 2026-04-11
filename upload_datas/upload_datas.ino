#include <Wire.h>
#include <HTTPClient.h>
#include "WiFiManager.h"
#include "SH1106.h"
#include "pins_config.h"

SH1106 oled;
WiFiManager wifi("^_________^", "Tieunguu09@");

String TEMP = "", PH = "", EC = "", DO = "";
String create_url = "https://htqt.vnkgu.edu.vn/create/insert.php";

unsigned long lastTime_LCD = 0, lastTime_UploadData = 0;
const unsigned long interval_LCD = 5000, interval_UploadData = 1000;

void upload_datas(String data) {
  HTTPClient http;
  http.begin(create_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(data);

  Serial.println("URL: " + String(create_url)); 
  Serial.println("Data: " + String(data));
  Serial.println("Response: " + String(response));
  http.end();
}

void setup() {
  Serial.begin(115200);
  wifi.connect();
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!oled.begin(0x3C)) {
    Serial.println("OLED init failed!");
    while (1);
  }

  oled.clear();
  oled.refresh();
}

void loop() {
  if (Serial1.available()) {
    String postData = Serial1.readStringUntil('\n');

    if (millis() - lastTime_UploadData >= interval_UploadData) {
      lastTime_UploadData = millis();

      upload_datas();
      Serial.println(postData);
    }
  }

  if (millis() - lastTime_LCD >= interval_LCD) {
    lastTime_LCD = millis();

    oled.clear();

    oled.showText(1, 0,  8,  "TEMP: " + String(TEMP));
    oled.showText(1, 0,  16, "PH:   " + String(PH));
    oled.showText(1, 0,  24, "EC:   " + String(EC));
    oled.showText(1, 0,  32, "DO:   " + String(DO));

    oled.refresh();
  }
}

