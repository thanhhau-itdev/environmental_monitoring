#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password) : ssid(ssid), password(password) {}

void WiFiManager::connect() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
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
}

void WiFiManager::startAccessPoint(const char* apSSID, const char* apPassword) {
    Serial.print("Starting Access Point: ");
    Serial.println(apSSID);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiManager::stopAccessPoint() {
    Serial.println("Stopping Access Point...");
    WiFi.softAPdisconnect(true);
}
