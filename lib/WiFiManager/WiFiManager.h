#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

// Quản lý WiFi:
// 1. Khởi động: thử lần lượt các mạng đã lưu trong /wifi.json (LittleFS)
// 2. Không kết nối được: phát WiFi (AP) + web server để chọn mạng và nhập mật khẩu
// 3. Kết nối thành công qua web: lưu mạng vào /wifi.json cho lần sau
// 4. Đang chạy mà mất WiFi lâu: thử lại danh sách, vẫn không được thì phát AP
// 5. Mạng nào kết nối được nhưng không có Internet (InternetCheck trả false) thì coi như thất bại
class WiFiManager {
public:
    typedef void (*StatusCallback)(const String &line1, const String &line2);
    typedef bool (*InternetCheck)();

    WiFiManager(const char* apSsid, const char* apPassword);

    void begin();
    void loop();

    bool isConnected();
    bool isPortalActive();
    String apName();
    String currentSsid();

    void setStatusCallback(StatusCallback cb);
    void setInternetCheck(InternetCheck check);
    void startPortal();
    void scanNetworks();

private:
    String _apSsid;
    String _apPassword;
    WebServer _server;
    DNSServer _dns;
    bool _portalActive;
    bool _fsReady;
    StatusCallback _statusCb;
    InternetCheck _internetCheck;

    String _pendingSsid;
    String _pendingPassword;
    bool _hasPending;
    String _lastMessage;

    unsigned long _lostSince;
    unsigned long _lastPortalRetry;

    bool connectSaved();
    bool tryConnect(const String &ssid, const String &password, unsigned long timeoutMs);
    bool migrateFromSdk();

    bool loadNetworks(JsonDocument &doc);
    bool saveNetwork(const String &ssid, const String &password);

    bool hasInternet();
    void stopPortal();
    void handleRoot();
    void handleSave();
    void handleNotFound();

    void notify(const String &line1, const String &line2 = "");
    static String htmlEscape(const String &s);
    static String signalBars(int32_t rssi);
};

#endif
