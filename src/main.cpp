#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "web_server.h"
#include "calculations.h"

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
// Helper: Dual logging (Serial + WebSocket)
// ============================================
void logBoth(const String& message) {
    Serial.println(message);
    if (wifiManager.isConnected()) {
        webServer.broadcastLog(message);
    }
}

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
            
            if (g_cpuUsage > 100.0) g_cpuUsage = 100.0;
            if (g_cpuUsage < 0.0) g_cpuUsage = 0.0;
        }
        
        busyTime = 0;
        idleTime = 0;
        lastCpuCheck = currentMillis;
    }
}

void printStatus() {
    static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastPrint >= 30000) {
        lastPrint = currentMillis;
        
        // Получение температуры чипа (если доступно)
        #ifdef SOC_TEMP_SENSOR_SUPPORTED
        float chipTemp = temperatureRead();
        #else
        float chipTemp = 0;
        #endif
        
        // Статистика памяти
        uint32_t freeHeap = ESP.getFreeHeap();
        uint32_t totalHeap = ESP.getHeapSize();
        uint32_t usedHeap = totalHeap - freeHeap;
        float heapUsage = (float)usedHeap / totalHeap * 100.0;
        uint32_t minFreeHeap = ESP.getMinFreeHeap();
        
        String status = "\n=== System status ===\n";
        status += "Uptime:         " + String(millis() / 1000) + " sec\n";
        status += "WiFi:           " + String(wifiManager.isConnected() ? "✓ Connected" : "✗ Disconnected");
        status += " (" + String(wifiManager.getRSSI()) + " dBm)\n";
        status += "Temperature:    " + String(sensorManager.getTemperature(), 1) + "°C\n";
        status += "Humidity:       " + String(sensorManager.getHumidity(), 1) + "%\n";
        status += "CPU Usage:      " + String(g_cpuUsage, 1) + "%\n";
        status += "Free Heap:      " + String(ESP.getFreeHeap() / 1024) + " KB\n";
        status += "Web Requests:   " + String(webServer.getRequestCount()) + "\n";
        
        Serial.println(status);
        
        if (wifiManager.isConnected()) {
            // Основная информация
            webServer.broadcastLog("📊 === Status Update ===");
            webServer.broadcastLog("🌡️  AHT10: T=" + String(sensorManager.getTemperature(), 1) + 
                                  "°C, H=" + String(sensorManager.getHumidity(), 1) + "%");
            
            // Температура чипа
            #ifdef SOC_TEMP_SENSOR_SUPPORTED
            if (chipTemp > 0) {
                String tempColor = chipTemp > 70 ? "🔥" : chipTemp > 50 ? "🌡️" : "❄️";
                webServer.broadcastLog(tempColor + " Chip: " + String(chipTemp, 1) + "°C");
            }
            #endif
            
            // CPU и память
            webServer.broadcastLog("⚡ CPU: " + String(g_cpuUsage, 1) + 
                                  "% @ " + String(ESP.getCpuFreqMHz()) + " MHz");
            webServer.broadcastLog("🧠 RAM: " + String(freeHeap / 1024) + " KB free / " + 
                                  String(totalHeap / 1024) + " KB total (" + 
                                  String(heapUsage, 1) + "% used)");
            webServer.broadcastLog("📉 Min Free: " + String(minFreeHeap / 1024) + " KB");
            
            // WiFi детали
            webServer.broadcastLog("📶 WiFi: " + wifiManager.getSSID() + 
                                  " (Ch" + String(wifiManager.getChannel()) + 
                                  ", " + String(wifiManager.getRSSI()) + " dBm)");
            
            // Статистика работы
            webServer.broadcastLog("📈 Requests: " + String(webServer.getRequestCount()) + 
                                  " | Errors: " + String(sensorManager.getReadErrorCount()));
            
            // Uptime красиво
            unsigned long uptimeSec = millis() / 1000;
            int days = uptimeSec / 86400;
            int hours = (uptimeSec % 86400) / 3600;
            int mins = (uptimeSec % 3600) / 60;
            String uptimeStr = "⏱️  Uptime: ";
            if (days > 0) uptimeStr += String(days) + "d ";
            uptimeStr += String(hours) + "h " + String(mins) + "m";
            webServer.broadcastLog(uptimeStr);
            
            webServer.broadcastLog("━━━━━━━━━━━━━━━━━━━━━━━");
        }
    }
}

// ============================================
// Setup
// ============================================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    printSystemInfo();
    
    // Инициализация датчика
    Serial.println("=== Инициализация датчика ===");
    if (!sensorManager.begin()) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║           CRITICAL ERROR!              ║");
        Serial.println("║       AHT10  is not detected!          ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.println();
        Serial.println("Possible reasons:");
        Serial.println("  1. Incorrect I2C connection");
        Serial.println("  2. Damaged sensor");
        Serial.println("  3. Incorrect GPIO pins in config.h");
        Serial.println();
        Serial.println("Check the connection and restart the device");
        
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
    Serial.println("║ ✓ Weather Monitor is ready to work!   ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    if (wifiManager.isConnected()) {
        String msg = "Access to the web interface: http://" + wifiManager.getIP() + "/";
        Serial.println(msg);
        Serial.println();
        
        // Отправка расширенной информации в WebSocket (с задержкой для установки соединения)
        delay(2000);
        webServer.broadcastLog("╔════════════════════════════════════════╗");
        webServer.broadcastLog("║   ESP32 Weather Station Started!      ║");
        webServer.broadcastLog("╚════════════════════════════════════════╝");
        webServer.broadcastLog("");
        webServer.broadcastLog("✓ System initialized successfully");
        webServer.broadcastLog("✓ AHT10 sensor: Ready");
        webServer.broadcastLog("✓ WiFi: " + wifiManager.getSSID());
        webServer.broadcastLog("✓ IP: " + wifiManager.getIP());
        webServer.broadcastLog("✓ Signal: " + String(wifiManager.getRSSI()) + " dBm (Ch" + 
                              String(wifiManager.getChannel()) + ")");
        webServer.broadcastLog("");
        webServer.broadcastLog("📟 Hardware Info:");
        webServer.broadcastLog("  Chip: " + String(ESP.getChipModel()) + 
                              " rev" + String(ESP.getChipRevision()));
        webServer.broadcastLog("  CPU: " + String(ESP.getCpuFreqMHz()) + " MHz");
        webServer.broadcastLog("  Flash: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB");
        webServer.broadcastLog("  RAM: " + String(ESP.getHeapSize() / 1024) + " KB");
        webServer.broadcastLog("  Free: " + String(ESP.getFreeHeap() / 1024) + " KB");
        
        #ifdef SOC_TEMP_SENSOR_SUPPORTED
        float chipTemp = temperatureRead();
        if (chipTemp > 0) {
            webServer.broadcastLog("  Chip Temp: " + String(chipTemp, 1) + "°C");
        }
        #endif
        
        webServer.broadcastLog("");
        webServer.broadcastLog("🌡️  Initial reading:");
        webServer.broadcastLog("  Temperature: " + String(sensorManager.getTemperature(), 1) + "°C");
        webServer.broadcastLog("  Humidity: " + String(sensorManager.getHumidity(), 1) + "%");
        webServer.broadcastLog("");
        webServer.broadcastLog("📊 Monitoring started (interval: " + 
                              String(SENSOR_INTERVAL / 1000) + "s)");
        webServer.broadcastLog("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
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
        
        bool wasConnected = wifiManager.isConnected();
        wifiManager.checkConnection();
        bool isConnected = wifiManager.isConnected();
        
        // Логирование изменения состояния WiFi
        if (!wasConnected && isConnected) {
            String msg = "✓ WiFi reconnected: " + wifiManager.getSSID() + 
                        " (" + wifiManager.getIP() + ", " + 
                        String(wifiManager.getRSSI()) + " dBm)";
            logBoth(msg);
        } else if (wasConnected && !isConnected) {
            logBoth("✗ WiFi connection lost!");
        }
    }
    
    // Обработка веб-запросов и WebSocket
    if (wifiManager.isConnected()) {
        webServer.handleClient();
    }
    
    // Чтение данных с датчика
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;
        
        if (sensorManager.update()) {
            float temp = sensorManager.getTemperature();
            float humid = sensorManager.getHumidity();
            float avgTemp = sensorManager.getAvgTemp();
            float avgHumid = sensorManager.getAvgHumid();
            
            // Базовый лог
            String log = "✓ T: " + String(temp, 1) + "°C | H: " + String(humid, 1) + "%";
            Serial.println(log);
            
            if (wifiManager.isConnected()) {
                webServer.broadcastLog(log);
                
                // Детальная статистика (каждое 3-е чтение = 30 сек)
                static int readCount = 0;
                readCount++;
                
                if (readCount % 3 == 0) {
                    // Min/Max info
                    webServer.broadcastLog("  📊 Min: T=" + String(sensorManager.getMinTemp(), 1) + 
                                          "°C, H=" + String(sensorManager.getMinHumid(), 1) + "%");
                    webServer.broadcastLog("  📊 Max: T=" + String(sensorManager.getMaxTemp(), 1) + 
                                          "°C, H=" + String(sensorManager.getMaxHumid(), 1) + "%");
                    webServer.broadcastLog("  📊 Avg: T=" + String(avgTemp, 1) + 
                                          "°C, H=" + String(avgHumid, 1) + "%");
                    
                    // Вычисляемые параметры
                    float dewPoint = WeatherCalculations::calculateDewPoint(temp, humid);
                    float heatIndex = WeatherCalculations::calculateHeatIndex(temp, humid);
                    webServer.broadcastLog("  💧 Dew Point: " + String(dewPoint, 1) + "°C");
                    webServer.broadcastLog("  🌡️  Heat Index: " + String(heatIndex, 1) + "°C");
                }
            }
        } else {
            String err = "✗ Sensor reading error (count: " + String(sensorManager.getReadErrorCount()) + ")";
            logBoth(err);
        }
    }
    
    updateCPUUsage();
    printStatus();
    
    unsigned long loopEnd = micros();
    busyTime += (loopEnd - loopStart);
    
    delay(10);
    idleTime += 10000;
}
