#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "web_server.h"

// ============================================
// Глобальные переменные
// ============================================
float g_cpuUsage = 0.0;

// Объекты
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
SensorManager sensorManager;
WeatherWebServer webServer(&sensorManager, &wifiManager);

// Таймеры
unsigned long lastSensorRead = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastStatsUpdate = 0;

// CPU мониторинг
unsigned long lastCpuCheck = 0;
unsigned long idleTime = 0;
unsigned long busyTime = 0;

// ============================================
// Функции
// ============================================

void printSystemInfo() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32 Super Mini Weather Station    ║");
    Serial.println("║        AHT10 Sensor v3.0              ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    Serial.println("=== Информация о системе ===");
    Serial.printf("Chip Model:     %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision:  %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency:  %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size:     %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap:      %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("SDK Version:    %s\n", ESP.getSdkVersion());
    Serial.println();
}

void updateCPUUsage() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastCpuCheck >= STATS_UPDATE_INTERVAL) {
        unsigned long totalTime = busyTime + idleTime;
        
        if (totalTime > 0) {
            g_cpuUsage = (float)busyTime / totalTime * 100.0;
            
            // Ограничиваем значение
            if (g_cpuUsage > 100.0) g_cpuUsage = 100.0;
            if (g_cpuUsage < 0.0) g_cpuUsage = 0.0;
        }
        
        // Сброс счётчиков
        busyTime = 0;
        idleTime = 0;
        lastCpuCheck = currentMillis;
    }
}

void printStatus() {
    static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();
    
    // Печатаем статус раз в 30 секунд
    if (currentMillis - lastPrint >= 30000) {
        lastPrint = currentMillis;
        
        Serial.println("\n=== Статус системы ===");
        Serial.printf("Uptime:         %lu сек\n", millis() / 1000);
        Serial.printf("WiFi:           %s (%d dBm)\n", 
                     wifiManager.isConnected() ? "✓ Подключен" : "✗ Отключен",
                     wifiManager.getRSSI());
        Serial.printf("Температура:    %.1f°C\n", sensorManager.getTemperature());
        Serial.printf("Влажность:      %.1f%%\n", sensorManager.getHumidity());
        Serial.printf("CPU Usage:      %.1f%%\n", g_cpuUsage);
        Serial.printf("Free Heap:      %d KB\n", ESP.getFreeHeap() / 1024);
        Serial.printf("Web Requests:   %lu\n", webServer.getRequestCount());
        Serial.println();
    }
}

// ============================================
// Setup
// ============================================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    // Вывод информации о системе
    printSystemInfo();
    
    // Инициализация датчика
    Serial.println("=== Инициализация датчика ===");
    if (!sensorManager.begin()) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║      КРИТИЧЕСКАЯ ОШИБКА!              ║");
        Serial.println("║   Датчик AHT10 не обнаружен!         ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.println();
        Serial.println("Возможные причины:");
        Serial.println("  1. Неправильное подключение I2C");
        Serial.println("  2. Поврежденный датчик");
        Serial.println("  3. Неправильные GPIO пины в config.h");
        Serial.println();
        Serial.println("Проверьте подключение и перезагрузите устройство");
        Serial.println();
        
        // Мигаем встроенным светодиодом для индикации ошибки
        while (1) {
            delay(250);
        }
    }
    
    // Подключение к WiFi
    Serial.println("=== Подключение к WiFi ===");
    if (!wifiManager.begin()) {
        Serial.println("✗ Не удалось подключиться к WiFi");
        Serial.println("  Продолжаем работу в автономном режиме");
        Serial.println("  Веб-интерфейс недоступен");
    }
    
    // Запуск веб-сервера
    Serial.println("=== Запуск веб-сервера ===");
    webServer.begin();
    
    // Финальное сообщение
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║    ✓ СИСТЕМА ГОТОВА К РАБОТЕ!        ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    if (wifiManager.isConnected()) {
        Serial.println("Доступ к веб-интерфейсу:");
        Serial.printf("  http://%s/\n", wifiManager.getIP().c_str());
        Serial.println();
    }
}

// ============================================
// Main Loop
// ============================================

void loop() {
    unsigned long loopStart = micros();
    unsigned long currentMillis = millis();
    
    // Проверка WiFi соединения
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = currentMillis;
        wifiManager.checkConnection();
    }
    
    // Обработка веб-запросов
    if (wifiManager.isConnected()) {
        webServer.handleClient();
    }
    
    // Чтение данных с датчика
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;
        
        if (!sensorManager.update()) {
            Serial.println("✗ Ошибка чтения датчика");
        }
    }
    
    // Обновление статистики CPU
    updateCPUUsage();
    
    // Печать статуса
    printStatus();
    
    // Расчет времени выполнения цикла
    unsigned long loopEnd = micros();
    busyTime += (loopEnd - loopStart);
    
    // Небольшая задержка для экономии CPU
    delay(10);
    idleTime += 10000; // 10ms в микросекундах
}
