#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid),
      _password(password),
      _reconnectCount(0),
      _lastCheck(0),
      _connectTime(0),
      _reconnecting(false),
      _reconnectStarted(0) {}

bool WiFiManager::begin() {
    Serial.println("\n=== WiFi Manager ===");
    return connectBlocking();
}

// ============================================
// Первоначальное подключение (блокирующее)
// Используется ТОЛЬКО в setup() — здесь scan нормален
// ============================================
bool WiFiManager::connectBlocking() {
    Serial.printf("Подключение к WiFi: %s\n", _ssid);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    WiFi.disconnect(true);
    delay(100);

    // Scan только при первоначальном подключении
    Serial.println("Сканирование WiFi сетей...");
    int networksFound = WiFi.scanNetworks();
    Serial.printf("Найдено сетей: %d\n", networksFound);

    bool targetFound = false;
    for (int i = 0; i < networksFound; i++) {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        Serial.printf("  %2d: %s (%d dBm) %s\n",
                      i + 1, ssid.c_str(), rssi,
                      WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "[OPEN]" : "");
        if (ssid == _ssid) {
            targetFound = true;
            Serial.printf("  ✓ Целевая сеть найдена! Сигнал: %d dBm\n", rssi);
        }
    }
    if (!targetFound)
        Serial.printf("  ✗ ВНИМАНИЕ: Сеть '%s' не найдена!\n", _ssid);

    WiFi.begin(_ssid, _password);

    Serial.print("Подключение");
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
        _reconnecting = false;
        Serial.println("\n✓ WiFi подключен успешно!");
        printNetworkInfo();
        return true;
    }

    Serial.println("\n✗ Не удалось подключиться к WiFi");
    return false;
}

// ============================================
// Проверка соединения — НЕ БЛОКИРУЕТ loop()
// Использует WiFi.reconnect() вместо полного пересоздания
// соединения со сканированием сетей.
// ============================================
void WiFiManager::checkConnection() {
    unsigned long now = millis();

    // Уже подключены и не в процессе реконнекта — просто раз в 5 с проверяем
    if (!_reconnecting) {
        if (WiFi.status() == WL_CONNECTED) {
            _lastCheck = now;
            return;
        }

        // Соединение потеряно — запускаем неблокирующий реконнект
        Serial.println("✗ WiFi отключен! Запуск реконнекта...");
        _reconnectCount++;
        _reconnectStarted = now;
        _reconnecting = true;

        // WiFi.reconnect() — НЕ блокирует, НЕ сканирует каналы
        // Просто просит стек переподключиться к последнему AP
        WiFi.reconnect();
        return;
    }

    // Ждём завершения неблокирующего реконнекта
    if (WiFi.status() == WL_CONNECTED) {
        _connectTime = now;
        _reconnecting = false;
        Serial.printf("✓ WiFi переподключён (попытка #%d): %s (%d dBm)\n",
                      _reconnectCount,
                      WiFi.SSID().c_str(),
                      WiFi.RSSI());
        printNetworkInfo();
        return;
    }

    // Таймаут реконнекта истёк — пробуем ещё раз
    if (now - _reconnectStarted > RECONNECT_TIMEOUT) {
        Serial.printf("✗ Реконнект #%d не удался (таймаут %lu с). Пробуем снова...\n",
                      _reconnectCount,
                      RECONNECT_TIMEOUT / 1000);
        _reconnectStarted = now;
        WiFi.reconnect();
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() const {
    return isConnected() ? WiFi.localIP().toString() : "Нет подключения";
}

int WiFiManager::getRSSI() const {
    return isConnected() ? WiFi.RSSI() : 0;
}

int WiFiManager::getChannel() const {
    return isConnected() ? WiFi.channel() : 0;
}

String WiFiManager::getSSID() const {
    return isConnected() ? WiFi.SSID() : "Не подключено";
}

int WiFiManager::getReconnectCount() const {
    return _reconnectCount;
}

unsigned long WiFiManager::getUptime() const {
    return (_connectTime > 0) ? (millis() - _connectTime) / 1000 : 0;
}

String WiFiManager::getConnectionStatus() const {
    switch (WiFi.status()) {
        case WL_CONNECTED:        return "Подключено";
        case WL_NO_SHIELD:        return "Нет WiFi модуля";
        case WL_IDLE_STATUS:      return "Ожидание";
        case WL_NO_SSID_AVAIL:    return "SSID не найден";
        case WL_SCAN_COMPLETED:   return "Сканирование завершено";
        case WL_CONNECT_FAILED:   return "Ошибка подключения";
        case WL_CONNECTION_LOST:  return "Соединение потеряно";
        case WL_DISCONNECTED:     return "Отключено";
        default:                  return "Неизвестно";
    }
}

void WiFiManager::printNetworkInfo() {
    Serial.println("\n--- Информация о подключении ---");
    Serial.printf("SSID:    %s\n",    WiFi.SSID().c_str());
    Serial.printf("IP:      %s\n",    WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n",    WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS:     %s\n",    WiFi.dnsIP().toString().c_str());
    Serial.printf("RSSI:    %d dBm\n", WiFi.RSSI());
    Serial.printf("Канал:   %d\n",    WiFi.channel());
    Serial.printf("MAC:     %s\n",    WiFi.macAddress().c_str());
    Serial.println("--------------------------------\n");
}
