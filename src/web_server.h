#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "battery_manager.h"
#include "calculations.h"

class WeatherWebServer {
public:
    WeatherWebServer(SensorManager* sensor, WiFiManager* wifi, BatteryManager* battery);
    
    void begin();
    void handleClient();
    void broadcastLog(const String& message);
    
    // Statistics
    unsigned long getRequestCount() const;
    
private:
    WebServer _server;
    WebSocketsServer _wsServer;
    SensorManager* _sensor;
    WiFiManager* _wifi;
    BatteryManager* _battery;
    unsigned long _bootTime;
    unsigned long _requestCount;
    
    // Route handlers
    void handleRoot();
    void handleData();
    void handleStats();
    void handleHistory();
    void handleBattery();  // New: battery endpoint
    void handleReset();
    void handleReboot();
    void handleNotFound();
    
    // WebSocket event handler
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    
    // Helper functions
    String getUptimeString() const;
    String formatBytes(size_t bytes) const;
    float getCPUUsage() const;
    String getBatteryStatusString() const;
    String getPowerSourceString() const;
    
    // CORS headers
    void setCORSHeaders();
};

#endif // WEB_SERVER_H

