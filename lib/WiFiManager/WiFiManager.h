#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Arduino.h>

class WiFiManager {
private:
    const char* ssid;
    const char* password;

public:
    WiFiManager(const char* ssid, const char* password);

    void connect();
    void scanNetworks();
    void startAccessPoint(const char* apSSID, const char* apPassword);
    void stopAccessPoint();
};

#endif
