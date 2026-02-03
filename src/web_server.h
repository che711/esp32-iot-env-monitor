#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "calculations.h"

class WeatherWebServer {
public:
    WeatherWebServer(SensorManager* sensor, WiFiManager* wifi);
    
    void begin();
    void handleClient();
    void broadcastLog(const String& message);
    
    // Статистика
    unsigned long getRequestCount() const;
    
private:
    WebServer _server;
    WebSocketsServer _wsServer;
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
    
    // WebSocket event handler
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    
    // Вспомогательные функции
    String getUptimeString() const;
    String formatBytes(size_t bytes) const;
    float getCPUUsage() const;
    
    // CORS headers
    void setCORSHeaders();
};

#endif // WEB_SERVER_H
