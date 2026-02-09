#include "web_server.h"
#include "html_pages.h"
#include "config.h"
#include <esp_system.h>

// Внешняя переменная из main.cpp
extern float g_cpuUsage;

WeatherWebServer::WeatherWebServer(SensorManager* sensor, WiFiManager* wifi)
    : _server(WEB_SERVER_PORT),
      _wsServer(81),  // WebSocket на порту 81
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
    
    // Запуск WebSocket сервера
    _wsServer.begin();
    _wsServer.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    });
    
    Serial.printf("✓ WebSocket сервер запущен на порту 81\n");
    Serial.printf("  ws://%s:81/\n", _wifi->getIP().c_str());
    Serial.println("");
}

void WeatherWebServer::handleClient() {
    _server.handleClient();
    _wsServer.loop();  // Обработка WebSocket событий
}

void WeatherWebServer::webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", num);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = _wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u connected from %s\n", num, ip.toString().c_str());
            
            // Отправляем приветственное сообщение
            String welcome = "✓ Serial Monitor connected";
            _wsServer.sendTXT(num, welcome);
            break;
        }
            
        case WStype_TEXT:
            // Если клиент отправляет команды через WebSocket
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
    // Отправка логов всем подключенным WebSocket клиентам
    String msg = message;  // Создаём копию для передачи по ссылке
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
