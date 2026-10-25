#include "WiFiManager.h"
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <esp_mac.h>
#include "SH1106.h"
#include "pins_config.h"

// WiFi cấu hình phát ra khi không kết nối được mạng nào đã lưu.
// Tên WiFi = AP_SSID_PREFIX + 4 ký tự cuối địa chỉ MAC, truy cập http://192.168.4.1
#define AP_SSID_PREFIX "ESP32C5"
#define AP_PASSWORD    "123456789"   // tối thiểu 8 ký tự

SH1106 oled;
bool oledReady = false;
WiFiManager *wifi = nullptr;

// String check_firmware_url = "http://192.168.1.111/htqt/uploads/upload_firmware.php";
// String create_url = "http://192.168.1.111/htqt/create/insert.php";
String check_firmware_url = "https://htqt.vnkgu.edu.vn/upload_firmware.php";
String create_url = "https://htqt.vnkgu.edu.vn/create/insert.php";
String internet_test_url = "https://htqt.vnkgu.edu.vn/";
String TEMP = "", PH = "", EC = "", DO = "", OTA = "";
String sensorData = "";
String wifiLine1 = "", wifiLine2 = "";

bool internetOk = false;
uint8_t internetFailCount = 0;
bool wasWifiConnected = false;
bool hasUploaded = false, hasCheckedFirmware = false;

unsigned long lastTime_LCD = 0, lastTime_UploadData = 0, lastTime_checkFirmware = 0;
unsigned long lastTime_SensorData = 0, lastTime_InternetTest = 0;
const unsigned long interval_LCD = 6000, interval_UploadData = 300000, interval_checkFirmware = 300000;
const unsigned long interval_InternetTest = 30000, sensorDataTimeout = 30000;
const uint8_t INTERNET_FAIL_LIMIT = 3;   // 3 lần liên tiếp (~1 phút) không ra Internet thì phát AP

void drawLCD();

// Gọi từ WiFiManager khi trạng thái WiFi thay đổi (kể cả lúc đang chờ kết nối)
void onWifiStatus(const String &line1, const String &line2) {
  wifiLine1 = line1;
  wifiLine2 = line2;
  drawLCD();
}

// Gửi thử request lên hosting, cập nhật internetOk.
// WiFiManager cũng gọi hàm này để loại mạng WiFi không có Internet.
bool testInternet() {
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  http.begin(internet_test_url);
  int code = http.GET();
  http.end();

  Serial.println("Internet test: " + String(code));
  internetOk = code > 0;
  lastTime_InternetTest = millis();
  if (internetOk) internetFailCount = 0;
  return internetOk;
}

bool upload_datas(String url, String data) {
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(10000);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(data);

  Serial.println("URL: " + String(url));
  Serial.println("Data: " + String(data));
  Serial.println("Response: " + String(response));
  http.end();

  return response >= 200 && response < 300;
}

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

// Chờ board đọc cảm biến trả về ký tự 'K', bỏ qua các byte khác
bool waitForAck(unsigned long timeoutMs)
{
  unsigned long start = millis();
  bool gotAck = false;
  while (!gotAck && millis() - start < timeoutMs)
  {
    while (Serial1.available() > 0)
    {
      if (Serial1.read() == 'K') gotAck = true;
    }
    if (!gotAck) delay(1);
  }
  while (Serial1.available() > 0) Serial1.read();
  return gotAck;
}

void sendFirmwareUART(String url)
{
  HTTPClient http;
  WiFiClient client;

  Serial.println("Downloading firmware...");

  http.setTimeout(10000);
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

  while (Serial1.available() > 0) Serial1.read();
  Serial1.print(firmwareSize);
  Serial1.write('\n');

  if (!waitForAck(5000))
  {
    Serial.println("Read sensor board did not start update");
    http.end();
    return;
  }

  unsigned long lastData = millis();

  while (remaining > 0)
  {
    int len = stream->readBytes(buffer, min(remaining, 128));

    if (len <= 0)
    {
      if (!stream->connected() || millis() - lastData > 15000)
      {
        Serial.println("Firmware download stalled, abort");
        http.end();
        return;
      }
      delay(1);
      continue;
    }
    lastData = millis();

    Serial1.write(buffer, len);
    remaining -= len;

    if (!waitForAck(5000))
    {
      Serial.println("No ACK from read sensor board, abort");
      http.end();
      return;
    }
  }

  Serial.println("Firmware sent via UART");
  http.end();
}

void check_firmware_status(String URL, int firmware_1, int firmware_2)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return;
  }
  String postData = "firmware_board_read_sensor=" + String(firmware_1) + "&firmware_board_upload_data=" + String(firmware_2);

  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(10000);
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  String payload = http.getString();
  http.end();
  Serial.println("Payload: " + payload);

  if (httpCode != 200) return;

  JsonDocument jsonDoc;
  if (deserializeJson(jsonDoc, payload)) return;
  if (!jsonDoc.is<JsonArray>()) return;   // {"status":"No_update !"}

  String uartFile = "", directFile = "";
  for (JsonObject obj : jsonDoc.as<JsonArray>())
  {
    int firmware = obj["firmware"] | 0;
    String file = obj["file"] | "";
    if (firmware == 1) uartFile = file;
    if (firmware == 2) directFile = file;
  }

  // Cập nhật board đọc cảm biến trước, vì cập nhật trực tiếp sẽ khởi động lại board này
  if (uartFile.length() > 0)
  {
    Serial.println("Software upgrade via UART");
    sendFirmwareUART(uartFile);
  }

  if (directFile.length() > 0)
  {
    Serial.println("Direct software upgrade");
    updateFirmware(directFile);
  }
}

// Lấy giá trị của key trong chuỗi dạng "temp=..&ph=..&ec=..&do=..&ota=.."
bool getField(const String &data, const String &key, String &value)
{
  String pattern = key + "=";
  int start;

  if (data.startsWith(pattern)) start = pattern.length();
  else
  {
    int pos = data.indexOf("&" + pattern);
    if (pos < 0) return false;
    start = pos + 1 + pattern.length();
  }

  int end = data.indexOf('&', start);
  if (end < 0) end = data.length();

  value = data.substring(start, end);
  value.trim();
  return value.length() > 0;
}

// Số thực hợp lệ: chỉ chữ số, tối đa một dấu chấm, dấu trừ ở đầu (loại bỏ nan, inf, chuỗi rác)
bool isValidNumber(const String &s)
{
  bool hasDigit = false, hasDot = false;
  for (unsigned int i = 0; i < s.length(); i++)
  {
    char c = s[i];
    if (isDigit(c)) hasDigit = true;
    else if (c == '.' && !hasDot) hasDot = true;
    else if (c == '-' && i == 0) continue;
    else return false;
  }
  return hasDigit;
}

void handleSensorLine(String data)
{
  data.trim();

  // Bỏ byte nhiễu phía trước (thường gặp khi board đọc cảm biến vừa khởi động)
  int start = data.indexOf("temp=");
  if (start > 0) data = data.substring(start);

  String t, p, e, d, o;
  if (!getField(data, "temp", t) || !getField(data, "ph", p) ||
      !getField(data, "ec", e) || !getField(data, "do", d))
  {
    Serial.println("Invalid sensor line: " + data);
    return;
  }
  if (!isValidNumber(t) || !isValidNumber(p) || !isValidNumber(e) || !isValidNumber(d))
  {
    Serial.println("Invalid sensor value: " + data);
    return;
  }
  getField(data, "ota", o);

  TEMP = t; PH = p; EC = e; DO = d; OTA = o;
  sensorData = "temp=" + t + "&ph=" + p + "&ec=" + e + "&do=" + d + "&ota=" + o;
  lastTime_SensorData = millis();
}

// Đọc UART không chặn: gom từng byte đến khi gặp '\n'.
// readStringUntil() sẽ treo cả vòng loop nếu chân RX nhận nhiễu liên tục (không có '\n')
#define SENSOR_LINE_MAX 128
String uartLine = "";

void readSensorUart()
{
  while (Serial1.available() > 0)
  {
    char c = Serial1.read();
    if (c == '\n')
    {
      if (uartLine.length() > 0) handleSensorLine(uartLine);
      uartLine = "";
    }
    else if (uartLine.length() < SENSOR_LINE_MAX)
    {
      uartLine += c;
    }
    else
    {
      uartLine = "";   // dòng quá dài: dữ liệu nhiễu, bỏ
    }
  }
}

void drawLCD()
{
  if (!oledReady) return;

  oled.clear();

  if (wifi != nullptr && wifi->isPortalActive())
  {
    oled.showText(1, 0, 0, "WIFI SETUP");
  }
  else if (wifi != nullptr && wifi->isConnected() && internetOk)
  {
    oled.showText(1, 0, 0, "Connected");
  }
  else if (wifi != nullptr && wifi->isConnected())
  {
    oled.showText(1, 0, 0, "NOT CONNECTED");
  }
  else
  {
    oled.showText(1, 0, 0, wifiLine1);
  }

  bool sensorFresh = lastTime_SensorData != 0 && millis() - lastTime_SensorData < sensorDataTimeout;

  if (sensorFresh)
  {
    oled.showText(1, 0,  10, "TEMP: " + String(TEMP));
    oled.showText(1, 0,  18, "PH:   " + String(PH));
    oled.showText(1, 0,  26, "EC:   " + String(EC));
    oled.showText(1, 0,  34, "DO:   " + String(DO));
    oled.showText(1, 0,  44, "OTA:  " + String(OTA));
  }
  else
  {
    oled.showText(1, 0, 24, "SENSOR: NO DATA");
  }

  if (wifi != nullptr && wifi->isPortalActive())
  {
    oled.showText(1, 0, 56, wifi->apName() + " " + WiFi.softAPIP().toString());
  }
  else if (wifi != nullptr && wifi->isConnected())
  {
    oled.showText(1, 0, 56, wifi->currentSsid());
  }
  else
  {
    oled.showText(1, 0, 56, wifiLine2);
  }

  oled.refresh();
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  Wire.begin(I2C_SDA, I2C_SCL);

  // Lỗi màn hình không được làm dừng cả trạm
  oledReady = oled.begin(0x3C);
  if (!oledReady) {
    Serial.println("OLED init failed!");
  } else {
    oled.clear();
    oled.refresh();
  }

  // Đọc MAC từ eFuse: WiFi.macAddress() trả về toàn 0 khi WiFi chưa khởi động
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char apSsid[24];
  snprintf(apSsid, sizeof(apSsid), "%s%02X%02X", AP_SSID_PREFIX, mac[4], mac[5]);

  wifi = new WiFiManager(apSsid, AP_PASSWORD);
  wifi->setStatusCallback(onWifiStatus);
  wifi->setInternetCheck(testInternet);
  wifi->begin();
}

void loop()
{
  wifi->loop();

  readSensorUart();

  bool wifiConnected = wifi->isConnected() && !wifi->isPortalActive();

  // Vừa kết nối WiFi: thử gửi request lên hosting trước khi gửi dữ liệu
  if (wifiConnected && !wasWifiConnected) {
    if (!testInternet()) internetFailCount++;
    drawLCD();
  }
  if (!wifiConnected) internetOk = false;
  wasWifiConnected = wifiConnected;

  // Có WiFi nhưng không ra Internet: thử lại mỗi 30 giây, quá số lần thì phát AP để chọn mạng khác
  if (wifiConnected && !internetOk && millis() - lastTime_InternetTest >= interval_InternetTest) {
    if (!testInternet() && ++internetFailCount >= INTERNET_FAIL_LIMIT) {
      internetFailCount = 0;
      wifi->startPortal();
    }
    drawLCD();
  }

  bool sensorFresh = lastTime_SensorData != 0 && millis() - lastTime_SensorData < sensorDataTimeout;

  if (internetOk && sensorFresh &&
      (!hasUploaded || millis() - lastTime_UploadData >= interval_UploadData)) {
    if (upload_datas(create_url, sensorData)) {
      hasUploaded = true;
      lastTime_UploadData = millis();
    } else {
      // Gửi lỗi: kiểm tra lại kết nối, khi thông thì gửi lại ngay
      internetOk = false;
      lastTime_InternetTest = millis();
      drawLCD();
    }
  }

  if (internetOk &&
      (!hasCheckedFirmware || millis() - lastTime_checkFirmware >= interval_checkFirmware)) {
    hasCheckedFirmware = true;
    lastTime_checkFirmware = millis();

    check_firmware_status(check_firmware_url, FIRMWARE_BOARD_READ_SENSOR, FIRMWARE_BOARD_UPLOAD_DATA);
  }

  if (millis() - lastTime_LCD >= interval_LCD) {
    lastTime_LCD = millis();
    drawLCD();
  }
}
