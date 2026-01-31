#include "web_server.h"
#include "html_pages.h"
#include "config.h"
#include <esp_system.h>

// Внешняя переменная из main.cpp
extern float g_cpuUsage;

WeatherWebServer::WeatherWebServer(SensorManager* sensor, WiFiManager* wifi)
    : _server(WEB_SERVER_PORT), 
      _sensor(sensor), 
      _wifi(wifi), 
      _bootTime(0),
      _requestCount(0) {
}

void WeatherWebServer::begin() {
    _bootTime = millis();
    
    Serial.println("\n=== Web Server ===");
    Serial.println("Настройка маршрутов...");
    
    // Основные маршруты
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/data", HTTP_GET, [this]() { handleData(); });
    _server.on("/stats", HTTP_GET, [this]() { handleStats(); });
    _server.on("/history", HTTP_GET, [this]() { handleHistory(); });
    _server.on("/reset", HTTP_GET, [this]() { handleReset(); });
    _server.on("/reboot", HTTP_GET, [this]() { handleReboot(); });
    _server.onNotFound([this]() { handleNotFound(); });
    
    _server.begin();
    Serial.printf("✓ HTTP сервер запущен на порту %d\n", WEB_SERVER_PORT);
    Serial.printf("  Адрес: http://%s/\n", _wifi->getIP().c_str());
    Serial.println("");
}

void WeatherWebServer::handleClient() {
    _server.handleClient();
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
                    "{\"error\":\"Датчик недоступен\",\"code\":503}");
        return;
    }
    
    float temp = _sensor->getTemperature();
    float humid = _sensor->getHumidity();
    
    float dewPoint = WeatherCalculations::calculateDewPoint(temp, humid);
    float heatIndex = WeatherCalculations::calculateHeatIndex(temp, humid);
    
    // Формирование JSON
    String json;
    json.reserve(512);
    
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
    json.reserve(512);
    
    json = "{";
    json += "\"uptime\":\"" + getUptimeString() + "\"";
    json += ",\"freeHeap\":\"" + formatBytes(freeHeap) + "\"";
    json += ",\"heapUsage\":\"" + String(heapUsagePercent, 1) + "%\"";
    json += ",\"cpuUsage\":\"" + String(getCPUUsage(), 1) + "\"";
    json += ",\"ssid\":\"" + _wifi->getSSID() + "\"";
    json += ",\"rssi\":\"" + String(_wifi->getRSSI()) + "\"";
    json += ",\"ip\":\"" + _wifi->getIP() + "\"";
    json += ",\"requests\":" + String(_requestCount);
    json += ",\"errors\":" + String(_sensor->getReadErrorCount());
    json += "}";
    
    setCORSHeaders();
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleHistory() {
    _requestCount++;
    
    float tempHist[HISTORY_SIZE];
    float humidHist[HISTORY_SIZE];
    
    _sensor->getHistory(tempHist, humidHist, HISTORY_SIZE);
    
    String json;
    json.reserve(HISTORY_BUFFER_SIZE);
    
    json = "{\"labels\":[";
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (i > 0) json += ",";
        int seconds = i * (SENSOR_INTERVAL / 1000);
        if (seconds < 60) {
            json += "\"" + String(seconds) + "s\"";
        } else {
            json += "\"" + String(seconds / 60) + "m\"";
        }
    }
    
    json += "],\"temp\":[";
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (i > 0) json += ",";
        json += String(tempHist[i], 1);
    }
    
    json += "],\"humid\":[";
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (i > 0) json += ",";
        json += String(humidHist[i], 1);
    }
    
    json += "]}";
    
    setCORSHeaders();
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleReset() {
    _requestCount++;
    
    _sensor->resetMinMax();
    
    setCORSHeaders();
    _server.send(200, "application/json", 
                "{\"success\":true,\"message\":\"Min/Max значения сброшены\"}");
}

void WeatherWebServer::handleReboot() {
    _requestCount++;
    
    Serial.println("\n=== ПЕРЕЗАГРУЗКА ПО ЗАПРОСУ ПОЛЬЗОВАТЕЛЯ ===");
    
    setCORSHeaders();
    _server.send(200, "application/json", 
                "{\"success\":true,\"message\":\"Устройство перезагружается...\"}");
    
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

unsigned long WeatherWebServer::getRequestCount() const {
    return _requestCount;
}
