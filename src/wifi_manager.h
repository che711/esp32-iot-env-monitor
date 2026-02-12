#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);
    
    bool begin();
    void checkConnection();
    bool isConnected() const;
    
    // Информация о подключении
    String getIP() const;
    int getRSSI() const;
    int getChannel() const;
    String getSSID() const;
    int getReconnectCount() const;
    
    // Статистика
    unsigned long getUptime() const;
    String getConnectionStatus() const;
    
private:
    const char* _ssid;
    const char* _password;
    int _reconnectCount;
    unsigned long _lastCheck;
    unsigned long _connectTime;
    
    bool connect();
    void printNetworkInfo();
};

#endif // WIFI_MANAGER_H

