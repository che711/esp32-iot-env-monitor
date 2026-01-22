#include "web_server.h"
#include "html_pages.h"
#include "config.h"
#include <esp_system.h>

WeatherWebServer::WeatherWebServer(SensorManager* sensor, WiFiManager* wifi)
    : _server(WEB_SERVER_PORT), _sensor(sensor), _wifi(wifi), _bootTime(0) {
}

void WeatherWebServer::begin() {
    _bootTime = millis();
    
    // Настройка маршрутов
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/data", HTTP_GET, [this]() { handleData(); });
    _server.on("/stats", HTTP_GET, [this]() { handleStats(); });
    _server.on("/history", HTTP_GET, [this]() { handleHistory(); });
    _server.on("/reset", HTTP_GET, [this]() { handleReset(); });
    _server.on("/reboot", HTTP_GET, [this]() { handleReboot(); });
    _server.onNotFound([this]() { handleNotFound(); });
    
    _server.begin();
    Serial.println("✓ HTTP сервер запущен\n");
}

void WeatherWebServer::handleClient() {
    _server.handleClient();
}

void WeatherWebServer::handleRoot() {
    _server.send(200, "text/html", HTML_PAGE);
}

void WeatherWebServer::handleData() {
    float temp = _sensor->getTemperature();
    float humid = _sensor->getHumidity();
    
    if (isnan(temp) || isnan(humid)) {
        _server.send(500, "application/json", "{\"error\":\"Данные не готовы\"}");
        return;
    }
    
    float dewPoint = WeatherCalculations::calculateDewPoint(temp, humid);
    float heatIndex = WeatherCalculations::calculateHeatIndex(temp, humid);
    
    String json = "{";
    json += "\"temperature\":" + String(temp, 2) + ",";
    json += "\"humidity\":" + String(humid, 2) + ",";
    json += "\"minTemp\":" + String(_sensor->getMinTemp(), 2) + ",";
    json += "\"maxTemp\":" + String(_sensor->getMaxTemp(), 2) + ",";
    json += "\"minHumid\":" + String(_sensor->getMinHumid(), 2) + ",";
    json += "\"maxHumid\":" + String(_sensor->getMaxHumid(), 2) + ",";
    json += "\"avgTemp\":" + String(_sensor->getAvgTemp(), 2) + ",";
    json += "\"avgHumid\":" + String(_sensor->getAvgHumid(), 2) + ",";
    json += "\"dewPoint\":" + String(dewPoint, 2) + ",";
    json += "\"heatIndex\":" + String(heatIndex, 2);
    json += "}";
    
    _server.sendHeader("Cache-Control", "no-cache");
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleStats() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float heapUsagePercent = (float)usedHeap / totalHeap * 100.0;
    
    String json = "{";
    json += "\"uptime\":\"" + getUptimeString() + "\",";
    json += "\"freeHeap\":\"" + String(freeHeap / 1024) + " KB (" + String(heapUsagePercent, 1) + "%)\",";
    json += "\"cpuUsage\":\"" + String(getCPUUsage(), 1) + "\",";
    json += "\"wifiChannel\":" + String(_wifi->getChannel()) + ",";
    json += "\"ssid\":\"" + _wifi->getSSID() + "\",";
    json += "\"rssi\":\"" + String(_wifi->getRSSI()) + "\",";
    json += "\"ip\":\"" + _wifi->getIP() + "\",";
    json += "\"reconnects\":" + String(_wifi->getReconnectCount());
    json += "}";
    
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleHistory() {
    float tempHist[HISTORY_SIZE];
    float humidHist[HISTORY_SIZE];
    
    _sensor->getHistory(tempHist, humidHist, HISTORY_SIZE);
    
    String json = "{";
    json += "\"labels\":[";
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (i > 0) json += ",";
        json += "\"" + String(i * (SENSOR_INTERVAL / 1000)) + "s\"";
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
    
    _server.send(200, "application/json", json);
}

void WeatherWebServer::handleReset() {
    _sensor->resetMinMax();
    _server.send(200, "application/json", "{\"message\":\"Min/Max значения сброшены\"}");
}

void WeatherWebServer::handleReboot() {
    _server.send(200, "application/json", "{\"message\":\"Перезагрузка...\"}");
    Serial.println("Перезагрузка по запросу пользователя");
    delay(1000);
    ESP.restart();
}

void WeatherWebServer::handleNotFound() {
    _server.send(404, "text/plain", "404: Not Found");
}

String WeatherWebServer::getUptimeString() {
    unsigned long uptime = (millis() - _bootTime) / 1000;
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    
    char buffer[20];
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
    return String(buffer);
}

float WeatherWebServer::getCPUUsage() {
    // Упрощенная версия - возвращает 0
    // Для точного расчета нужно использовать глобальные переменные из main.cpp
    return 0.0;
}
