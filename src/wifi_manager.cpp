#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid),
      _password(password),
      _reconnectCount(0),
      _lastCheck(0) {}

void WiFiManager::begin() {
    connect();
}

void WiFiManager::connect() {
    Serial.printf("Connecting to SSID: [%s]\n", _ssid);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    WiFi.disconnect(false);
    delay(200);

    int n = WiFi.scanNetworks();
    Serial.printf("Found %d networks\n", n);
    for (int i = 0; i < n; i++) {
        Serial.printf("%2d: %s (%d dBm)\n",
                      i,
                      WiFi.SSID(i).c_str(),
                      WiFi.RSSI(i));
    }

    WiFi.begin(_ssid, _password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ WiFi connect failed");
    }
}

void WiFiManager::checkConnection() {
    if (millis() - _lastCheck < 5000) return;
    _lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
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
