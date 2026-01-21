#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "web_server.h"

// Глобальные объекты
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
SensorManager sensorManager;
WeatherWebServer webServer(&sensorManager, &wifiManager);

// Таймеры
unsigned long lastSensorRead = 0;
unsigned long lastWiFiCheck = 0;

// Переменные для расчёта CPU
unsigned long lastCpuCheck = 0;
unsigned long idleTime = 0;
unsigned long busyTime = 0;
float cpuUsage = 0.0;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    Serial.println("\n\n=== ESP32 Super Mini + AHT10 v2.0 ===");
    Serial.println("Версия: 2.0");
    Serial.println("================================\n");
    
    // Инициализация датчика
    if (!sensorManager.begin()) {
        Serial.println("КРИТИЧЕСКАЯ ОШИБКА: Датчик не найден!");
        Serial.println("Проверьте подключение и перезагрузите устройство");
        while (1) {
            delay(1000);
            Serial.print(".");
        }
    }
    
    // Подключение к WiFi
    wifiManager.begin();
    
    // Запуск веб-сервера
    webServer.begin();
    
    Serial.println("\n================================");
    Serial.println("✓ Система готова к работе!");
    Serial.println("================================\n");
}

void loop() {
    unsigned long loopStart = micros(); // Начало цикла
    unsigned long currentMillis = millis();
    
    // Проверка WiFi подключения
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = currentMillis;
        wifiManager.checkConnection();
    }
    
    // Обработка веб-запросов
    webServer.handleClient();
    
    // Чтение данных с датчика
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;
        sensorManager.update();
    }
    
    unsigned long loopEnd = micros(); // Конец цикла
    busyTime += (loopEnd - loopStart); // Время работы
    
    // Небольшая задержка для экономии CPU
    delay(2);
    idleTime += 2000; // 2ms в микросекундах
    
    // Расчёт использования CPU каждую секунду
    if (currentMillis - lastCpuCheck >= 1000) {
        unsigned long totalTime = busyTime + idleTime;
        if (totalTime > 0) {
            cpuUsage = (float)busyTime / totalTime * 100.0;
        }
        // Сброс счётчиков
        busyTime = 0;
        idleTime = 0;
        lastCpuCheck = currentMillis;
    }
}

// Функция для получения CPU usage (может быть вызвана из web_server)
float getCPUUsage() {
    return cpuUsage;
}
