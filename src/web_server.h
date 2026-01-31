#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "calculations.h"

class WeatherWebServer {
public:
    WeatherWebServer(SensorManager* sensor, WiFiManager* wifi);
    
    void begin();
    void handleClient();
    
    // Статистика
    unsigned long getRequestCount() const;
    
private:
    WebServer _server;
    SensorManager* _sensor;
    WiFiManager* _wifi;
    unsigned long _bootTime;
    unsigned long _requestCount;
    
    // Обработчики маршрутов
    void handleRoot();
    void handleData();
    void handleStats();
    void handleHistory();
    void handleReset();
    void handleReboot();
    void handleNotFound();
    
    // Вспомогательные функции
    String getUptimeString() const;
    String formatBytes(size_t bytes) const;
    float getCPUUsage() const;
    
    // CORS headers
    void setCORSHeaders();
};

#endif // WEB_SERVER_H
