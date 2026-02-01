#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid),
      _password(password),
      _reconnectCount(0),
      _lastCheck(0),
      _connectTime(0) {}

bool WiFiManager::begin() {
    Serial.println("\n=== WiFi Manager ===");
    return connect();
}

bool WiFiManager::connect() {
    Serial.printf("Подключение к WiFi: %s\n", _ssid);
    
    // Настройка WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // Отключаем sleep mode для стабильности
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    
    // Отключаемся от предыдущей сети
    WiFi.disconnect(true);
    delay(100);
    
    // Сканирование сетей
    Serial.println("Сканирование WiFi сетей...");
    int networksFound = WiFi.scanNetworks();
    Serial.printf("Найдено сетей: %d\n", networksFound);
    
    bool targetFound = false;
    for (int i = 0; i < networksFound; i++) {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        
        Serial.printf("  %2d: %s (%d dBm) %s\n", 
                     i + 1, 
                     ssid.c_str(), 
                     rssi,
                     WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "[OPEN]" : "");
        
        if (ssid == _ssid) {
            targetFound = true;
            Serial.printf("  ✓ Целевая сеть найдена! Сигнал: %d dBm\n", rssi);
        }
    }
    
    if (!targetFound) {
        Serial.printf("  ✗ ВНИМАНИЕ: Сеть '%s' не найдена!\n", _ssid);
    }
    
    // Подключение
    Serial.print("Подключение");
    WiFi.begin(_ssid, _password);
    
    int attempts = 0;
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_RETRY) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        if (millis() - startTime > WIFI_TIMEOUT) {
            Serial.println("\n✗ Таймаут подключения!");
            break;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        _connectTime = millis();
        Serial.println("\n✓ WiFi подключен успешно!");
        printNetworkInfo();
        return true;
    } else {
        Serial.println("\n✗ Не удалось подключиться к WiFi");
        Serial.println("Проверьте:");
        Serial.println("  1. Правильность SSID и пароля");
        Serial.println("  2. Доступность сети");
        Serial.println("  3. Настройки роутера");
        return false;
    }
}

void WiFiManager::printNetworkInfo() {
    Serial.println("\n--- Информация о подключении ---");
    Serial.printf("SSID:    %s\n", WiFi.SSID().c_str());
    Serial.printf("IP:      %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS:     %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("RSSI:    %d dBm\n", WiFi.RSSI());
    Serial.printf("Канал:   %d\n", WiFi.channel());
    Serial.printf("MAC:     %s\n", WiFi.macAddress().c_str());
    Serial.println("--------------------------------\n");
}

void WiFiManager::checkConnection() {
    unsigned long currentMillis = millis();
    
    // Проверяем не чаще чем раз в 5 секунд
    if (currentMillis - _lastCheck < 5000) {
        return;
    }
    
    _lastCheck = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("✗ WiFi отключен! Попытка переподключения...");
        _reconnectCount++;
        
        if (connect()) {
            Serial.printf("✓ Переподключение успешно (попытка #%d)\n", _reconnectCount);
        } else {
            Serial.printf("✗ Переподключение не удалось (попытка #%d)\n", _reconnectCount);
        }
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Нет подключения";
}

int WiFiManager::getRSSI() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

int WiFiManager::getChannel() const {
    if (isConnected()) {
        return WiFi.channel();
    }
    return 0;
}

String WiFiManager::getSSID() const {
    if (isConnected()) {
        return WiFi.SSID();
    }
    return "Не подключено";
}

int WiFiManager::getReconnectCount() const {
    return _reconnectCount;
}

unsigned long WiFiManager::getUptime() const {
    if (_connectTime > 0) {
        return (millis() - _connectTime) / 1000;
    }
    return 0;
}

String WiFiManager::getConnectionStatus() const {
    switch(WiFi.status()) {
        case WL_CONNECTED:
            return "Подключено";
        case WL_NO_SHIELD:
            return "Нет WiFi модуля";
        case WL_IDLE_STATUS:
            return "Ожидание";
        case WL_NO_SSID_AVAIL:
            return "SSID не найден";
        case WL_SCAN_COMPLETED:
            return "Сканирование завершено";
        case WL_CONNECT_FAILED:
            return "Ошибка подключения";
        case WL_CONNECTION_LOST:
            return "Соединение потеряно";
        case WL_DISCONNECTED:
            return "Отключено";
        default:
            return "Неизвестно";
    }
}
