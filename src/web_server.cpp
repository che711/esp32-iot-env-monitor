#include "web_server.h"
#include "html_pages.h"
#include "config.h"
#include <esp_system.h>

// External variable from main.cpp
extern float g_cpuUsage;
extern int bootCount;

WeatherWebServer::WeatherWebServer(SensorManager* sensor, WiFiManager* wifi, BatteryManager* battery)
    : _server(WEB_SERVER_PORT),
      _wsServer(WEBSOCKET_PORT),
      _sensor(sensor),
      _wifi(wifi),
      _battery(battery),
      _bootTime(0),
      _requestCount(0) {
}

void WeatherWebServer::begin() {
    _bootTime = millis();
    
    Serial.println("\n=== Web Server ===");
    Serial.println("Setting up routes...");
    
    // Routes
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/data", HTTP_GET, [this]() { handleData(); });
    _server.on("/stats", HTTP_GET, [this]() { handleStats(); });
    _server.on("/history", HTTP_GET, [this]() { handleHistory(); });
    _server.on("/battery", HTTP_GET, [this]() { handleBattery(); });
    _server.on("/reset", HTTP_GET, [this]() { handleReset(); });
    _server.on("/reboot", HTTP_GET, [this]() { handleReboot(); });
    _server.onNotFound([this]() { handleNotFound(); });
    
    _server.begin();
    Serial.printf("✓ HTTP server started on port %d\n", WEB_SERVER_PORT);
    Serial.printf("  URL: http://%s/\n", _wifi->getIP().c_str());
    
    // WebSocket server
    _wsServer.begin();
    _wsServer.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    });
    
    Serial.printf("✓ WebSocket server started on port %d\n", WEBSOCKET_PORT);
    Serial.printf("  ws://%s:%d/\n", _wifi->getIP().c_str(), WEBSOCKET_PORT);
    Serial.println("");
}

void WeatherWebServer::handleClient() {
    _server.handleClient();
    _wsServer.loop();
}

void WeatherWebServer::webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", num);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = _wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u connected from %s\n", num, ip.toString().c_str());
            
            String welcome = "✓ Serial Monitor connected";
            _wsServer.sendTXT(num, welcome);
            break;
        }
            
        case WStype_TEXT:
            Serial.printf("[WS] Received: %s\n", payload);
            break;
            
        case WStype_ERROR:
            Serial.printf("[WS] Error on client #%u\n", num);
            break;
            
        default:
            break;
    }
}

void WeatherWebServer::broadcastLog(const String& message) {
    String msg = message;
    _wsServer.broadcastTXT(msg);
}

void WeatherWebServer::setCORSHeaders() {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WeatherWebServer::handleRoot() {
    _requestCount++;
    _server.send_P(200, "text/html", HTML_PAGE);
}

void WeatherWebServer::handleData() {
    _requestCount++;
    
    if (!_sensor->isValid()) {
        setCORSHeaders();
        _server.send(503, "application/json", 
                    "{\"error\":\"Sensor unavailable\",\"code\":503}");
        return;
    }
    
    float temp = _sensor->getTemperature();
    float humid = _sensor->getHumidity();
    float dewPoint = WeatherCalculations::calculateDewPoint(temp, humid);
    float heatIndex = WeatherCalculations::calculateHeatIndex(temp, humid);
    
    String json;
    json.reserve(768);
    
    json = "{";
    json += "\"temperature\":" + String(temp, 2);
    json += ",\"humidity\":" + String(humid, 2);
    json += ",\"minTemp\":" + String(_sensor->getMinTemp(), 2);
    json += ",\"maxTemp\":" + String(_sensor->getMaxTemp(), 2);
    json += ",\"minHumid\":" + String(_sensor->getMinHumid(), 2);
    json += ",\"maxHumid\":" + String(_sensor->getMaxHumid(), 2);
    json += ",\"avgTemp\":" + String(_sensor->getAvgTemp(), 2);
    json += ",\"avgHumid\":" + String(_sensor->getAvgHumid(), 2);
    json += ",\"dewPoint\":" + String(dewPoint, 2);
    json += ",\"heatIndex\":" + String(heatIndex, 2);
    
    // Battery info
    json += ",\"battery\":{";
    json += "\"voltage\":" + String(_battery->getVoltage(), 2);
    json += ",\"percentage\":" + String(_battery->getPercentage(), 1);
    json += ",\"isCharging\":" + String(_battery->isCharging() ? "true" : "false");
    json += ",\"isCharged\":" + String(_battery->isCharged() ? "true" : "false");
    json += ",\"usbConnected\":" + String(_battery->isUSBConnected() ? "true" : "false");
    json += ",\"status\":\"" + getBatteryStatusString() + "\"";
    json += ",\"powerSource\":\"" + getPowerSourceString() + "\"";
    json += "}";
    
    json += ",\"timestamp\":" + String(millis());
    json += "}";
    
    setCORSHeaders();
    _server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleStats() {
    _requestCount++;
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float heapUsagePercent = (float)usedHeap / totalHeap * 100.0;
    
    String json;
    json.reserve(768);
    
    json = "{";
    json += "\"uptime\":\"" + getUptimeString() + "\"";
    json += ",\"bootCount\":" + String(bootCount);
    json += ",\"freeHeap\":\"" + formatBytes(freeHeap) + "\"";
    json += ",\"heapUsage\":\"" + String(heapUsagePercent, 1) + "%\"";
    json += ",\"cpuUsage\":\"" + String(getCPUUsage(), 1) + "\"";
    json += ",\"cpuFreq\":\"" + String(ESP.getCpuFreqMHz()) + " MHz\"";
    json += ",\"ssid\":\"" + _wifi->getSSID() + "\"";
    json += ",\"rssi\":\"" + String(_wifi->getRSSI()) + "\"";
    json += ",\"ip\":\"" + _wifi->getIP() + "\"";
    json += ",\"requests\":" + String(_requestCount);
    json += ",\"errors\":" + String(_sensor->getReadErrorCount());
    
    // Battery stats
    json += ",\"battery\":{";
    json += "\"voltage\":" + String(_battery->getVoltage(), 2);
    json += ",\"percentage\":" + String(_battery->getPercentage(), 1);
    json += ",\"status\":\"" + getBatteryStatusString() + "\"";
    json += ",\"powerSource\":\"" + getPowerSourceString() + "\"";
    json += ",\"isCharging\":" + String(_battery->isCharging() ? "true" : "false");
    json += ",\"isCharged\":" + String(_battery->isCharged() ? "true" : "false");
    json += ",\"usbConnected\":" + String(_battery->isUSBConnected() ? "true" : "false");
    json += "}";
    
    json += "}";
    
    setCORSHeaders();
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleHistory() {
    _requestCount++;
    
    float tempHist[HISTORY_SIZE];
    float humidHist[HISTORY_SIZE];
    
    _sensor->getHistory(tempHist, humidHist, HISTORY_SIZE);
    int count = _sensor->getHistoryCount();
    
    unsigned long nowSec = millis() / 1000;
    
    String json;
    json.reserve(HISTORY_BUFFER_SIZE);
    
    json = "{\"labels\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        unsigned long pointSec = nowSec - (unsigned long)(count - 1 - i) * (SENSOR_INTERVAL / 1000);
        int h = (pointSec / 3600) % 24;
        int m = (pointSec % 3600) / 60;
        int s = pointSec % 60;
        char buf[12];
        sprintf(buf, "%02d:%02d:%02d", h, m, s);
        json += "\"";
        json += buf;
        json += "\"";
    }
    
    json += "],\"temp\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += String(tempHist[i], 1);
    }
    
    json += "],\"humid\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += String(humidHist[i], 1);
    }
    
    json += "]}";
    
    setCORSHeaders();
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleBattery() {
    _requestCount++;
    
    String json;
    json.reserve(512);
    
    json = "{";
    json += "\"voltage\":" + String(_battery->getVoltage(), 3);
    json += ",\"percentage\":" + String(_battery->getPercentage(), 2);
    json += ",\"status\":\"" + getBatteryStatusString() + "\"";
    json += ",\"powerSource\":\"" + getPowerSourceString() + "\"";
    json += ",\"isCharging\":" + String(_battery->isCharging() ? "true" : "false");
    json += ",\"isCharged\":" + String(_battery->isCharged() ? "true" : "false");
    json += ",\"usbConnected\":" + String(_battery->isUSBConnected() ? "true" : "false");
    json += ",\"isLow\":" + String(_battery->isBatteryLow() ? "true" : "false");
    json += ",\"isCritical\":" + String(_battery->isBatteryCritical() ? "true" : "false");
    json += ",\"lastUpdate\":" + String(_battery->getLastUpdateTime());
    json += ",\"readCount\":" + String(_battery->getReadCount());
    json += "}";
    
    setCORSHeaders();
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleReset() {
    _requestCount++;
    
    _sensor->resetMinMax();
    
    setCORSHeaders();
    _server.send(200, "application/json", 
                "{\"success\":true,\"message\":\"Min/Max values reset\"}");
}

void WeatherWebServer::handleReboot() {
    _requestCount++;
    
    Serial.println("\n=== REBOOT BY USER REQUEST ===");
    
    setCORSHeaders();
    _server.send(200, "application/json", 
                "{\"success\":true,\"message\":\"Device rebooting...\"}");
    
    delay(500);
    ESP.restart();
}

void WeatherWebServer::handleNotFound() {
    _requestCount++;
    
    String message = "404: Not Found\n\n";
    message += "URI: " + _server.uri() + "\n";
    message += "Method: " + String(_server.method() == HTTP_GET ? "GET" : "POST") + "\n";
    
    _server.send(404, "text/plain", message);
}

String WeatherWebServer::getUptimeString() const {
    unsigned long uptime = (millis() - _bootTime) / 1000;
    
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    
    char buffer[32];
    if (days > 0) {
        sprintf(buffer, "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
    } else {
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
    }
    
    return String(buffer);
}

String WeatherWebServer::formatBytes(size_t bytes) const {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return String(bytes / 1024.0, 1) + " KB";
    } else {
        return String(bytes / (1024.0 * 1024.0), 2) + " MB";
    }
}

float WeatherWebServer::getCPUUsage() const {
    return g_cpuUsage;
}

String WeatherWebServer::getBatteryStatusString() const {
    switch (_battery->getStatus()) {
        case BatteryStatus::CHARGING:
            return "Charging";
        case BatteryStatus::CHARGED:
            return "Charged";
        case BatteryStatus::DISCHARGING:
            return "Discharging";
        case BatteryStatus::BATTERY_LOW:  // Updated name
            return "Low";
        case BatteryStatus::CRITICAL:
            return "Critical";
        default:
            return "Unknown";
    }
}

String WeatherWebServer::getPowerSourceString() const {
    switch (_battery->getPowerSource()) {
        case PowerSource::BATTERY:
            return "Battery";
        case PowerSource::USB_CHARGING:
            return "USB";
        default:
            return "Unknown";
    }
}

unsigned long WeatherWebServer::getRequestCount() const {
    return _requestCount;
}

