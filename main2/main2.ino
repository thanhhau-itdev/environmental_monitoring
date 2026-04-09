#include <Wire.h>
#include <HTTPClient.h>
#include "WiFiManager.h"
#include "SH1106.h"
#include "pins_config.h"

SH1106 oled;
WiFiManager wifi("^_________^", "Tieunguu09@");

String TEMP = "", PH = "", EC = "", DO = "";
String create_url = "https://htqt.vnkgu.edu.vn/create/insert.php";
String check_upload_firmware = "https://htqt.vnkgu.edu.vn/uploads/check_upload_sensors.php";

unsigned long lastTime_LCD = 0, lastTime_UploadData = 0;
const unsigned long interval_LCD = 5000, interval_UploadData = 1000;

int parseCSV(String csvLine, String fields[]) {
  int fieldIndex = 0;
  String current = "";

  csvLine.trim();

  for (int i = 0; i < csvLine.length(); i++) {
    char c = csvLine[i];

    if (c == ',') {
      if (fieldIndex < MAX_FIELDS) {
        fields[fieldIndex++] = current;
      }
      current = "";
    } else {
      current += c;
    }
  }

  if (current.length() > 0 && fieldIndex < MAX_FIELDS) {
    fields[fieldIndex++] = current;
  }

  return fieldIndex;
}

void create(String t, String p, String e, String d) {
  String postData = "temp=" + String(t) + "&ph=" + String(p) + "&ec=" + String(e) + "&do=" + String(d); 

  HTTPClient http;
  http.begin(create_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(postData);

  Serial.println("URL: " + String(create_url)); 
  Serial.println("Data: " + String(postData));
  Serial.println("Response: " + String(response));
  http.end();
}

void setup() {
  Serial.begin(115200);
  wifi.connect();

  Serial1.begin(9600, SERIAL_8N1, UART_RX, UART_TX);
  Serial1.setTimeout(50);

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
    String data = Serial1.readStringUntil('\n');

    String fields[MAX_FIELDS] = {"", "", "", "", ""};
    int count = parseCSV(data, fields);

    if (count >= 4) {
      TEMP = fields[0];
      PH   = fields[1];
      EC   = fields[2];
      DO  = fields[3];

      String postData = "temp=" + TEMP +
                        "&ph=" + PH +
                        "&ec=" + EC +
                        "&do=" + DO;

      if (millis() - lastTime_UploadData >= interval_UploadData) {
        lastTime_UploadData = millis();

        create(TEMP, PH, EC, DO);
        Serial.println(postData);
      }
    }
  }

  if (millis() - lastTime_UploadData >= interval_UploadData) {
    lastTime_UploadData = millis();

    TEMP = "1";
    PH = "2";
    EC = "3";
    DO = "4";

    create(TEMP, PH, EC, DO);
    Serial.println(TEMP);
  }

  if (millis() - lastTime_UploadData >= interval_UploadData) {
    lastTime_UploadData = millis();

    TEMP = "1";
    PH = "2";
    EC = "3";
    DO = "4";

    create(TEMP, PH, EC, DO);
    Serial.println(TEMP);
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
