#include "WiFiManager.h"
#include <LittleFS.h>
#include <vector>
#include <algorithm>

#define WIFI_LIST_FILE        "/wifi.json"
#define WIFI_LIST_MAX         10
#define CONNECT_TIMEOUT       15000
#define PORTAL_RETRY_INTERVAL 300000   // đang phát AP mà không ai truy cập: 5 phút thử lại danh sách
#define LOST_WIFI_TIMEOUT     60000    // mất WiFi quá 1 phút: thử lại danh sách rồi phát AP

WiFiManager::WiFiManager(const char* apSsid, const char* apPassword)
    : _apSsid(apSsid), _apPassword(apPassword), _server(80),
      _portalActive(false), _fsReady(false), _statusCb(nullptr), _internetCheck(nullptr),
      _hasPending(false), _lostSince(0), _lastPortalRetry(0) {}

void WiFiManager::setStatusCallback(StatusCallback cb) {
    _statusCb = cb;
}

void WiFiManager::setInternetCheck(InternetCheck check) {
    _internetCheck = check;
}

// Không cài hàm kiểm tra thì coi như có Internet khi đã kết nối WiFi
bool WiFiManager::hasInternet() {
    return _internetCheck == nullptr || _internetCheck();
}

void WiFiManager::notify(const String &line1, const String &line2) {
    Serial.println(line1 + " " + line2);
    if (_statusCb) _statusCb(line1, line2);
}

void WiFiManager::begin() {
    _fsReady = LittleFS.begin(true);
    if (!_fsReady) Serial.println("LittleFS mount failed");

    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/save", HTTP_POST, [this]() { handleSave(); });
    _server.onNotFound([this]() { handleNotFound(); });

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    if (connectSaved()) {
        notify("WiFi: " + WiFi.SSID(), WiFi.localIP().toString());
    } else {
        startPortal();
    }
}

void WiFiManager::loop() {
    if (_portalActive) {
        _dns.processNextRequest();
        _server.handleClient();

        if (_hasPending) {
            _hasPending = false;
            notify("WiFi: dang thu", _pendingSsid);

            if (!tryConnect(_pendingSsid, _pendingPassword, CONNECT_TIMEOUT)) {
                _lastMessage = "Không kết nối được tới \"" + _pendingSsid + "\". Kiểm tra lại mật khẩu.";
                notify("AP: " + _apSsid, WiFi.softAPIP().toString());
            } else if (!hasInternet()) {
                WiFi.disconnect();
                _lastMessage = "Đã kết nối \"" + _pendingSsid + "\" nhưng mạng này không có Internet. Chọn mạng khác.";
                notify("AP: " + _apSsid, WiFi.softAPIP().toString());
            } else {
                saveNetwork(_pendingSsid, _pendingPassword);
                _lastMessage = "";
                stopPortal();
                notify("WiFi: " + WiFi.SSID(), WiFi.localIP().toString());
            }
            _pendingPassword = "";
            return;
        }

        // Không ai đang cấu hình thì định kỳ thử lại danh sách (ví dụ router vừa khởi động lại)
        if (WiFi.softAPgetStationNum() == 0 && millis() - _lastPortalRetry >= PORTAL_RETRY_INTERVAL) {
            _lastPortalRetry = millis();
            if (connectSaved()) {
                stopPortal();
                notify("WiFi: " + WiFi.SSID(), WiFi.localIP().toString());
            } else {
                notify("AP: " + _apSsid, WiFi.softAPIP().toString());
            }
        }
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        _lostSince = 0;
        return;
    }

    if (_lostSince == 0) {
        _lostSince = millis();
        notify("WiFi: mat ket noi", "");
        return;
    }

    if (millis() - _lostSince >= LOST_WIFI_TIMEOUT) {
        _lostSince = 0;
        if (connectSaved()) {
            notify("WiFi: " + WiFi.SSID(), WiFi.localIP().toString());
        } else {
            startPortal();
        }
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::isPortalActive() {
    return _portalActive;
}

String WiFiManager::apName() {
    return _apSsid;
}

String WiFiManager::currentSsid() {
    return isConnected() ? WiFi.SSID() : String("");
}

bool WiFiManager::tryConnect(const String &ssid, const String &password, unsigned long timeoutMs) {
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        if (_portalActive) {
            _dns.processNextRequest();
            _server.handleClient();
        }
        delay(50);
    }

    if (WiFi.status() == WL_CONNECTED) return true;

    // Dừng STA thử lại ngầm, tránh làm AP đổi kênh liên tục
    WiFi.disconnect();
    return false;
}

// Lần đầu chạy firmware mới (danh sách rỗng): thử mạng mà ESP32 đã lưu trong NVS
// từ firmware cũ, để trạm đang lắp đặt không bị mất kết nối sau khi OTA
bool WiFiManager::migrateFromSdk() {
    notify("WiFi: thu mang cu", "");
    WiFi.begin();

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECT_TIMEOUT) {
        delay(50);
    }

    // Từ đây danh sách nằm trong /wifi.json, không cần ghi NVS mỗi lần kết nối
    WiFi.persistent(false);

    if (WiFi.status() == WL_CONNECTED) {
        saveNetwork(WiFi.SSID(), WiFi.psk());
        if (hasInternet()) return true;
        notify("WiFi: khong Internet", WiFi.SSID());
    }

    WiFi.disconnect();
    return false;
}

bool WiFiManager::connectSaved() {
    JsonDocument doc;
    loadNetworks(doc);
    JsonArray list = doc.as<JsonArray>();

    if (list.size() == 0) return migrateFromSdk();
    WiFi.persistent(false);

    struct Candidate {
        String ssid;
        String password;
        int32_t rssi;
    };
    std::vector<Candidate> candidates;

    notify("WiFi: dang quet", "");
    int count = WiFi.scanNetworks();

    for (JsonObject item : list) {
        Candidate c;
        c.ssid = item["ssid"] | "";
        c.password = item["password"] | "";
        c.rssi = -1000;   // mạng không thấy khi quét (có thể là mạng ẩn) thì thử sau cùng
        if (c.ssid.length() == 0) continue;

        for (int i = 0; i < count; i++) {
            if (WiFi.SSID(i) == c.ssid && WiFi.RSSI(i) > c.rssi) c.rssi = WiFi.RSSI(i);
        }
        candidates.push_back(c);
    }
    WiFi.scanDelete();

    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Candidate &a, const Candidate &b) { return a.rssi > b.rssi; });

    for (const Candidate &c : candidates) {
        notify("WiFi: dang thu", c.ssid);
        if (!tryConnect(c.ssid, c.password, CONNECT_TIMEOUT)) continue;

        if (hasInternet()) {
            saveNetwork(c.ssid, c.password);
            return true;
        }

        // Kết nối được nhưng không ra Internet: thử mạng tiếp theo
        notify("WiFi: khong Internet", c.ssid);
        WiFi.disconnect();
    }
    return false;
}

bool WiFiManager::loadNetworks(JsonDocument &doc) {
    doc.to<JsonArray>();
    if (!_fsReady || !LittleFS.exists(WIFI_LIST_FILE)) return false;

    File file = LittleFS.open(WIFI_LIST_FILE, "r");
    if (!file) return false;

    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err || !doc.is<JsonArray>()) {
        Serial.println("wifi.json invalid, ignored");
        doc.to<JsonArray>();
        return false;
    }
    return true;
}

// Mạng vừa kết nối thành công được đưa lên đầu danh sách
bool WiFiManager::saveNetwork(const String &ssid, const String &password) {
    if (!_fsReady || ssid.length() == 0) return false;

    JsonDocument oldDoc;
    loadNetworks(oldDoc);

    JsonDocument newDoc;
    JsonArray list = newDoc.to<JsonArray>();

    JsonObject first = list.add<JsonObject>();
    first["ssid"] = ssid;
    first["password"] = password;

    for (JsonObject item : oldDoc.as<JsonArray>()) {
        if (list.size() >= WIFI_LIST_MAX) break;
        String itemSsid = item["ssid"] | "";
        if (itemSsid == ssid || itemSsid.length() == 0) continue;
        list.add(item);
    }

    File file = LittleFS.open(WIFI_LIST_FILE, "w");
    if (!file) return false;
    serializeJson(newDoc, file);
    file.close();
    return true;
}

void WiFiManager::startPortal() {
    if (_portalActive) return;

    // Bỏ mạng hiện tại (ví dụ mạng không có Internet) để AP không bị kéo theo kênh của nó
    WiFi.disconnect();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(_apSsid.c_str(), _apPassword.c_str());
    delay(100);

    _dns.start(53, "*", WiFi.softAPIP());
    _server.begin();
    _portalActive = true;
    _lastPortalRetry = millis();

    notify("AP: " + _apSsid, WiFi.softAPIP().toString());
}

void WiFiManager::stopPortal() {
    _server.stop();
    _dns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _portalActive = false;
}

String WiFiManager::htmlEscape(const String &s) {
    String out;
    out.reserve(s.length() + 8);
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s[i];
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;
        }
    }
    return out;
}

String WiFiManager::signalBars(int32_t rssi) {
    int bars = rssi >= -55 ? 4 : rssi >= -67 ? 3 : rssi >= -75 ? 2 : 1;
    String out;
    for (int i = 0; i < 4; i++) out += (i < bars) ? "▮" : "▯";
    return out;
}

void WiFiManager::handleRoot() {
    int count = WiFi.scanNetworks();

    // Bỏ SSID trùng (nhiều điểm phát cùng tên), giữ cái sóng mạnh nhất, sắp xếp mạnh -> yếu
    std::vector<int> indexes;
    for (int i = 0; i < count; i++) {
        if (WiFi.SSID(i).length() == 0) continue;
        bool replaced = false;
        for (int &j : indexes) {
            if (WiFi.SSID(j) == WiFi.SSID(i)) {
                if (WiFi.RSSI(i) > WiFi.RSSI(j)) j = i;
                replaced = true;
                break;
            }
        }
        if (!replaced) indexes.push_back(i);
    }
    std::sort(indexes.begin(), indexes.end(),
              [](int a, int b) { return WiFi.RSSI(a) > WiFi.RSSI(b); });

    String html;
    html.reserve(4096);
    html += F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
              "<meta name='viewport' content='width=device-width,initial-scale=1'>"
              "<title>Cấu hình WiFi</title><style>"
              "body{font-family:sans-serif;max-width:420px;margin:24px auto;padding:0 16px}"
              "label{display:block;margin-top:14px;font-weight:bold}"
              "select,input,button{width:100%;padding:10px;margin-top:6px;box-sizing:border-box;font-size:16px}"
              "button{background:#0b6ed1;color:#fff;border:0;border-radius:6px;margin-top:20px}"
              ".err{background:#fde8e8;color:#9b1c1c;padding:10px;border-radius:6px}"
              "</style></head><body><h2>Cấu hình WiFi</h2>");

    if (_lastMessage.length() > 0) {
        html += "<p class='err'>" + htmlEscape(_lastMessage) + "</p>";
    }

    html += F("<form method='POST' action='/save'><label>Mạng WiFi</label><select name='ssid'>");
    if (indexes.empty()) {
        html += F("<option value=''>Không tìm thấy mạng nào</option>");
    }
    for (int i : indexes) {
        String ssid = htmlEscape(WiFi.SSID(i));
        int32_t rssi = WiFi.RSSI(i);
        String band = WiFi.channel(i) > 14 ? "5GHz" : "2.4GHz";
        String open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? ", mở" : "";

        html += "<option value='" + ssid + "'>" + ssid + "  " + signalBars(rssi) +
                " (" + String(rssi) + " dBm, " + band + open + ")</option>";
    }
    WiFi.scanDelete();

    html += F("</select>"
              "<label>Hoặc nhập tên mạng ẩn</label><input name='ssid_manual' maxlength='32' placeholder='Bỏ trống nếu đã chọn ở trên'>"
              "<label>Mật khẩu</label><input type='password' name='password' maxlength='64'>"
              "<button type='submit'>Kết nối</button></form>"
              "<p><a href='/'>Quét lại</a></p></body></html>");

    _server.send(200, "text/html; charset=utf-8", html);
}

void WiFiManager::handleSave() {
    String ssid = _server.arg("ssid_manual");
    ssid.trim();
    if (ssid.length() == 0) ssid = _server.arg("ssid");

    if (ssid.length() == 0) {
        _lastMessage = "Chưa chọn mạng WiFi.";
        _server.sendHeader("Location", "/");
        _server.send(302, "text/plain", "");
        return;
    }

    _pendingSsid = ssid;
    _pendingPassword = _server.arg("password");
    _hasPending = true;

    String html = F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                    "<meta http-equiv='refresh' content='25;url=/'>"
                    "<title>Đang kết nối</title></head>"
                    "<body style='font-family:sans-serif;max-width:420px;margin:24px auto;padding:0 16px'>");
    html += "<h2>Đang kết nối tới " + htmlEscape(ssid) + "...</h2>";
    html += F("<p>Theo dõi màn hình LCD của thiết bị. Nếu kết nối thành công, WiFi cấu hình sẽ tắt.</p>"
              "<p>Nếu thất bại, trang này sẽ quay lại form sau khoảng 25 giây để nhập lại.</p></body></html>");
    _server.send(200, "text/html; charset=utf-8", html);
}

void WiFiManager::handleNotFound() {
    // Điện thoại kiểm tra captive portal bằng nhiều URL khác nhau: chuyển hết về trang cấu hình
    _server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
    _server.send(302, "text/plain", "");
}

void WiFiManager::scanNetworks() {
    Serial.println("Scanning for WiFi networks...");
    int networkCount = WiFi.scanNetworks();
    for (int i = 0; i < networkCount; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (Signal: ");
        Serial.print(WiFi.RSSI(i));
        Serial.println(" dBm)");
    }
    WiFi.scanDelete();
}
