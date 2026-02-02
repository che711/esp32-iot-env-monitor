#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "web_server.h"

// ============================================
// Global variables
// ============================================
float g_cpuUsage = 0.0;

// Objects
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
SensorManager sensorManager;
WeatherWebServer webServer(&sensorManager, &wifiManager);

// Timers
unsigned long lastSensorRead = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastStatsUpdate = 0;

// CPU monitoring
unsigned long lastCpuCheck = 0;
unsigned long idleTime = 0;
unsigned long busyTime = 0;

// ============================================
// Functions
// ============================================

void printSystemInfo() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32 Super Mini Weather Station    ║");
    Serial.println("║        AHT10 Sensor v3.0              ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    Serial.println("=== System information ===");
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
            
            // Limiting value
            if (g_cpuUsage > 100.0) g_cpuUsage = 100.0;
            if (g_cpuUsage < 0.0) g_cpuUsage = 0.0;
        }
        
        // Resetting counters
        busyTime = 0;
        idleTime = 0;
        lastCpuCheck = currentMillis;
    }
}

void printStatus() {
    static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();
    
    // Print the status once in 30 sec
    if (currentMillis - lastPrint >= 30000) {
        lastPrint = currentMillis;
        
        Serial.println("\n=== System status ===");
        Serial.printf("Uptime:         %lu sec\n", millis() / 1000);
        Serial.printf("WiFi:           %s (%d dBm)\n", 
                     wifiManager.isConnected() ? "✓ Connected" : "✗ Disconnected",
                     wifiManager.getRSSI());
        Serial.printf("Temperature:    %.1f°C\n", sensorManager.getTemperature());
        Serial.printf("Humidity:      %.1f%%\n", sensorManager.getHumidity());
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
    
    // Output of information about the system
    printSystemInfo();
    
    // Инициализация датчика
    Serial.println("=== Инициализация датчика ===");
    if (!sensorManager.begin()) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║           CRITICAL ERROR!                ║");
        Serial.println("║          AHT10  is not detected!         ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.println();
        Serial.println("Possible reasons:");
        Serial.println("  1. Incorrect I2C connection");
        Serial.println("  2. Damaged sens");
        Serial.println("  3. Incorrect GPIO pins in config.h");
        Serial.println();
        Serial.println("Check the connection and restart the device");
        Serial.println();
        
        // Blink the built-in LED to indicate an error
        while (1) {
            delay(250);
        }
    }
    
    // Подключение к WiFi
    Serial.println("=== Connecting to WiFi ===");
    if (!wifiManager.begin()) {
        Serial.println("✗ Cannot connect to WiFi");
        Serial.println("  Keep working offline.");
        Serial.println("  The web interface is unavailable");
    }
    
    // Запуск веб-сервера
    Serial.println("=== Launching the web server ===");
    webServer.begin();
    
    // Финальное сообщение
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║ ✓Atmospheric Monitor is ready to work! ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    if (wifiManager.isConnected()) {
        Serial.println("Access to the web interface:");
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
            Serial.println("✗ Sensor reading error");
        }
    }
    
    // Update CPU stats
    updateCPUUsage();
    
    // Print stats
    printStatus();
    
    // Calculation of the cycle execution time
    unsigned long loopEnd = micros();
    busyTime += (loopEnd - loopStart);
    
    // Simple delay
    delay(10);
    idleTime += 10000; // 10ms
}
