#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);

    bool begin();           // Первоначальное подключение (блокирующее — ок в setup)
    void checkConnection(); // Вызывать в loop() — НЕ блокирует
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

    // Состояние неблокирующего реконнекта
    bool _reconnecting;
    unsigned long _reconnectStarted;
    static constexpr unsigned long RECONNECT_TIMEOUT = 15000; // мс

    bool connectBlocking(); // Только для setup()
    void printNetworkInfo();
};

#endif // WIFI_MANAGER_H
