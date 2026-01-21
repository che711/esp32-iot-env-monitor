#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password), _reconnectCount(0), _lastCheck(0) {
}

void WiFiManager::begin() {
    Serial.println("\n=== Подключение к WiFi ===");
    connect();
}

void WiFiManager::connect() {
    WiFi.disconnect(true);
    delay(1000);
    
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(_ssid, _password);
    
    Serial.print("Подключение");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi подключен!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("\n✗ Ошибка подключения к WiFi");
    }
}

void WiFiManager::checkConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi потерян! Переподключение...");
        _reconnectCount++;
        connect();
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() {
    return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() {
    return WiFi.RSSI();
}

int WiFiManager::getChannel() {
    return WiFi.channel();
}

String WiFiManager::getSSID() {
    return WiFi.SSID();
}

int WiFiManager::getReconnectCount() {
    return _reconnectCount;
}
