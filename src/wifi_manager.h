#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);
    
    void begin();
    void checkConnection();
    bool isConnected();
    
    String getIP();
    int getRSSI();
    int getChannel();
    String getSSID();
    int getReconnectCount();
    
private:
    const char* _ssid;
    const char* _password;
    int _reconnectCount;
    unsigned long _lastCheck;
    
    void connect();
};

#endif // WIFI_MANAGER_H
