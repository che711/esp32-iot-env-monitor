#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "battery_manager.h"
#include "web_server.h"
#include "calculations.h"

// ============================================
// Global variables
// ============================================
float g_cpuUsage = 0.0;

// Переменные в RTC-памяти переживают deep sleep (но не полный сброс питания).
// g_sleepCycles — сколько раз подряд мы уходили в сон из-за критической батареи.
// Используется для прогрессивного увеличения длительности сна: 5→10→20→40→60 мин.
RTC_DATA_ATTR uint32_t g_sleepCycles = 0;
RTC_DATA_ATTR uint32_t g_bootCount   = 0;

// Objects
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
SensorManager sensorManager;
BatteryManager batteryManager(BATTERY_ADC_PIN, BATTERY_CHRG_PIN, BATTERY_STDBY_PIN);
WeatherWebServer webServer(&sensorManager, &wifiManager, &batteryManager);

// Timers
unsigned long lastSensorRead = 0;
// lastWiFiCheck удалён — checkConnection() теперь вызывается каждый тик (не блокирует)
unsigned long lastStatsUpdate = 0;
unsigned long lastBatteryCheck = 0;
unsigned long systemStartTime = 0;

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
// Deep Sleep Management
// ============================================
void enterDeepSleep(uint64_t duration_us, const char* reason) {
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║         ENTERING DEEP SLEEP MODE         ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.printf("Reason: %s\n", reason);
    // БАГФИКС: было %lu с 64-битным аргументом (duration/1000000ULL) — UB,
    // печатался мусор. Теперь %llu.
    Serial.printf("Duration: %llu seconds (sleep cycle #%lu)\n",
                  (unsigned long long)(duration_us / 1000000ULL),
                  (unsigned long)g_sleepCycles);
    Serial.println();

    // Отправляем уведомление через WebSocket.
    // БАГФИКС: раньше был просто delay(500) — но без вызова wsServer.loop()
    // сообщение могло не уйти. Прокачиваем обработчик клиентов ~500 мс.
    if (wifiManager.isConnected()) {
        String msg = "⚠️ DEEP SLEEP: " + String(reason);
        webServer.broadcastLog(msg);
        webServer.broadcastLog("Duration: " + String((unsigned long)(duration_us / 1000000ULL)) + "s");
        unsigned long t0 = millis();
        while (millis() - t0 < 500) {
            webServer.handleClient();
            delay(10);
        }
    }

    Serial.println("Shutting down peripherals...");
    WiFi.disconnect(true);   // Корректно закрываем соединение
    WiFi.mode(WIFI_OFF);

    // Конфигурируем таймер пробуждения
    esp_sleep_enable_timer_wakeup(duration_us);

    Serial.println("Going to sleep now...");
    Serial.flush();
    delay(100);

    // Уходим в deep sleep (после пробуждения выполнение начнётся с setup())
    esp_deep_sleep_start();
}

// Длительность сна с прогрессивным удвоением: 5 → 10 → 20 → 40 → 60 (макс) мин
uint64_t nextSleepDuration() {
    uint64_t d = DEEP_SLEEP_CRITICAL_DURATION;
    for (uint32_t i = 0; i < g_sleepCycles && d < DEEP_SLEEP_MAX_DURATION; i++) {
        d *= 2;
    }
    if (d > DEEP_SLEEP_MAX_DURATION) d = DEEP_SLEEP_MAX_DURATION;
    return d;
}

void checkBatteryAndSleep() {
    // Проверяем только если прошло достаточно времени с момента загрузки
    unsigned long uptime = millis() - systemStartTime;
    if (uptime < MIN_UPTIME_BEFORE_SLEEP) {
        return;
    }

    // Не проверяем, если подключено USB
    if (batteryManager.isUsbConnected()) {
        // USB вернулся — сбрасываем счётчик циклов сна
        if (g_sleepCycles > 0) g_sleepCycles = 0;
        return;
    }

    // Критический уровень батареи
    if (batteryManager.isCriticalBattery() && DEEP_SLEEP_ENABLED) {
        String reason = "Critical battery: " + String(batteryManager.getVoltage(), 2) + "V";
        uint64_t duration = nextSleepDuration();  // 1-й сон = 5 мин, дальше ×2
        g_sleepCycles++;
        enterDeepSleep(duration, reason.c_str());
    } else if (g_sleepCycles > 0) {
        // Батарея восстановилась выше критического порога
        g_sleepCycles = 0;
    }
}

// ============================================
// Functions
// ============================================

void printSystemInfo() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║   ESP32-C3 Weather Station v3.1          ║");
    Serial.println("║   AHT10 + Battery Management             ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
    
    // Проверка причины пробуждения
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("⏰ Wakeup from DEEP SLEEP (Timer)");
        Serial.println();
    }
    
    Serial.println("=== System Information ===");
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
            if (g_cpuUsage < 0.0)   g_cpuUsage = 0.0;
        }
        busyTime = 0;
        idleTime = 0;
        lastCpuCheck = currentMillis;
    }
}

void printStatus() {
    static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastPrint < 30000) return;
    lastPrint = currentMillis;

    #ifdef SOC_TEMP_SENSOR_SUPPORTED
    float chipTemp = temperatureRead();
    #else
    float chipTemp = 0;
    #endif

    uint32_t freeHeap    = ESP.getFreeHeap();
    uint32_t totalHeap   = ESP.getHeapSize();
    float    heapUsage   = (float)(totalHeap - freeHeap) / totalHeap * 100.0;
    uint32_t minFreeHeap = ESP.getMinFreeHeap();

    Serial.println();
    Serial.println("=== System Status ===");
    Serial.printf("Uptime:       %lu sec\n", millis() / 1000);
    Serial.printf("WiFi:         %s (%d dBm)\n",
                  wifiManager.isConnected() ? "Connected" : "Disconnected",
                  wifiManager.getRSSI());
    Serial.printf("Temperature:  %.1f C\n", sensorManager.getTemperature());
    Serial.printf("Humidity:     %.1f%%\n",  sensorManager.getHumidity());
    Serial.printf("Battery:      %s\n", batteryManager.getSummaryString().c_str());
    Serial.printf("CPU:          %.1f%%\n",  g_cpuUsage);
    Serial.printf("Free Heap:    %u KB\n",   freeHeap / 1024);
    Serial.printf("Requests:     %lu\n",     webServer.getRequestCount());
    Serial.println();

    if (wifiManager.isConnected()) {
        webServer.broadcastLog("=== Status Update ===");
        webServer.broadcastLog("T=" + String(sensorManager.getTemperature(), 1) +
                               "C  H=" + String(sensorManager.getHumidity(), 1) + "%");
        webServer.broadcastLog("Battery: " + batteryManager.getSummaryString());
        #ifdef SOC_TEMP_SENSOR_SUPPORTED
        if (chipTemp > 0)
            webServer.broadcastLog("Chip: " + String(chipTemp, 1) + "C");
        #endif
        webServer.broadcastLog("CPU: " + String(g_cpuUsage, 1) +
                               "%  @" + String(ESP.getCpuFreqMHz()) + " MHz");
        webServer.broadcastLog("RAM: " + String(freeHeap / 1024) + " KB free / " +
                               String(totalHeap / 1024) + " KB (" +
                               String(heapUsage, 1) + "% used)");
        webServer.broadcastLog("Min Free: " + String(minFreeHeap / 1024) + " KB");
        webServer.broadcastLog("WiFi: " + wifiManager.getSSID() +
                               " (Ch" + String(wifiManager.getChannel()) +
                               "  " + String(wifiManager.getRSSI()) + " dBm)");
        webServer.broadcastLog("Requests: " + String(webServer.getRequestCount()) +
                               "  Errors: " + String(sensorManager.getReadErrorCount()));

        unsigned long up = millis() / 1000;
        int d = up / 86400, h = (up % 86400) / 3600, m = (up % 3600) / 60;
        String uStr = "Uptime: ";
        if (d > 0) uStr += String(d) + "d ";
        uStr += String(h) + "h " + String(m) + "m";
        webServer.broadcastLog(uStr);
        webServer.broadcastLog("-------------------------");
    }
}

// ============================================
// Setup
// ============================================
void setup() {
    systemStartTime = millis();
    g_bootCount++;

    bool wokeFromSleep =
        (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER);

    // USB CDC Serial init (ESP32-C3 specific)
    Serial.begin(SERIAL_BAUD);
    if (!wokeFromSleep) {
        // Ждём хост только при обычном старте. При пробуждении из deep sleep
        // на батарее USB-хоста нет — ждать 5 секунд значит впустую жечь заряд
        // в КАЖДОМ цикле сна.
        unsigned long t0 = millis();
        while (!Serial && (millis() - t0 < 5000)) {
            delay(10);
        }
        delay(300);
    }

    printSystemInfo();
    Serial.printf("Boot #%lu | Sleep cycles: %lu\n\n",
                  (unsigned long)g_bootCount, (unsigned long)g_sleepCycles);

    // Initialize Battery Manager
    Serial.println("=== Initializing Battery Manager ===");
    batteryManager.begin();
    Serial.println();

    // Check battery before starting everything else.
    // При пробуждении из сна с критической батареей мы окажемся здесь через
    // ~1 секунду после старта — WiFi и сенсор даже не начнут инициализироваться.
    if (batteryManager.isCriticalBattery() && !batteryManager.isUsbConnected()) {
        Serial.println("⚠️ CRITICAL: Battery too low to start. Entering deep sleep immediately.");
        if (DEEP_SLEEP_ENABLED) {
            uint64_t duration = nextSleepDuration();
            g_sleepCycles++;
            enterDeepSleep(duration, "Critical battery on boot");
        }
        // DEEP_SLEEP_ENABLED=false: print warning and continue (developer/debug mode)
        Serial.println("⚠️ DEEP_SLEEP_ENABLED=false — continuing despite critical battery.");
    } else if (wokeFromSleep && !batteryManager.isCriticalBattery()) {
        // Проснулись, а батарея уже в норме (зарядили) — сбрасываем счётчик
        g_sleepCycles = 0;
    }

    // Initialize sensor
    Serial.println("=== Initializing Sensor ===");
    if (!sensorManager.begin()) {
        Serial.println();
        Serial.println("╔══════════════════════════════════════════╗");
        Serial.println("║           CRITICAL ERROR!                ║");
        Serial.println("║       AHT10 is not detected!             ║");
        Serial.println("╚══════════════════════════════════════════╝");
        Serial.println("Possible reasons:");
        Serial.println("  1. Incorrect I2C connection");
        Serial.println("  2. Damaged sensor");
        Serial.println("  3. Incorrect GPIO pins in config.h");
        Serial.println("Check the connection and restart the device");
        while (1) { delay(250); }
    }

    // Connect to WiFi
    Serial.println("=== Connecting to WiFi ===");
    if (!wifiManager.begin()) {
        Serial.println("Cannot connect to WiFi");
        Serial.println("  Working offline — web interface unavailable");
    } else if (WIFI_POWER_SAVE_ON_BATTERY && !batteryManager.isUsbConnected()) {
        // Стартуем на батарее — сразу включаем modem sleep
        wifiManager.setPowerSave(true);
        Serial.println("🔋 Battery power detected — WiFi power save ON");
    }

    // Launch web-server
    Serial.println("=== Launching Web Server ===");
    webServer.begin();

    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║    Weather Monitor is ready to work!     ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();

    if (wifiManager.isConnected()) {
        Serial.println("Web: http://" + wifiManager.getIP() + "/");
        Serial.println();

        delay(2000);
        webServer.broadcastLog("╔══════════════════════════════════════════╗");
        webServer.broadcastLog("║     ESP32 Weather Station Started!       ║");
        webServer.broadcastLog("╚══════════════════════════════════════════╝");
        webServer.broadcastLog("");
        webServer.broadcastLog("AHT10 sensor: Ready");
        webServer.broadcastLog("Battery: " + batteryManager.getSummaryString());
        webServer.broadcastLog("WiFi: " + wifiManager.getSSID());
        webServer.broadcastLog("IP: "   + wifiManager.getIP());
        webServer.broadcastLog("Signal: " + String(wifiManager.getRSSI()) +
                               " dBm (Ch" + String(wifiManager.getChannel()) + ")");
        webServer.broadcastLog("");
        webServer.broadcastLog("Hardware:");
        webServer.broadcastLog("  Chip:  " + String(ESP.getChipModel()) +
                               " rev" + String(ESP.getChipRevision()));
        webServer.broadcastLog("  CPU:   " + String(ESP.getCpuFreqMHz()) + " MHz");
        webServer.broadcastLog("  Flash: " + String(ESP.getFlashChipSize() / (1024*1024)) + " MB");
        webServer.broadcastLog("  RAM:   " + String(ESP.getHeapSize() / 1024) + " KB");
        webServer.broadcastLog("  Free:  " + String(ESP.getFreeHeap() / 1024) + " KB");
        #ifdef SOC_TEMP_SENSOR_SUPPORTED
        float ct = temperatureRead();
        if (ct > 0) webServer.broadcastLog("  Chip Temp: " + String(ct, 1) + "C");
        #endif
        webServer.broadcastLog("");
        webServer.broadcastLog("Initial reading:");
        webServer.broadcastLog("  Temperature: " +
                               String(sensorManager.getTemperature(), 1) + "C");
        webServer.broadcastLog("  Humidity:    " +
                               String(sensorManager.getHumidity(), 1) + "%");
        webServer.broadcastLog("");
        webServer.broadcastLog("Monitoring interval: " +
                               String(SENSOR_INTERVAL / 1000) + "s");
        webServer.broadcastLog("Deep Sleep: " + String(DEEP_SLEEP_ENABLED ? "Enabled" : "Disabled"));
        webServer.broadcastLog("-------------------------");
    }
}

// ============================================
// Main Loop
// ============================================
void loop() {
    unsigned long loopStart    = micros();
    unsigned long currentMillis = millis();

    // Check battery status
    if (currentMillis - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
        lastBatteryCheck = currentMillis;
        PowerSource prevSource = batteryManager.getPowerSource();
        batteryManager.update();

        // Автопереключение энергосбережения WiFi по источнику питания:
        // батарея → modem sleep включён, USB → выключен (минимальная задержка)
        if (WIFI_POWER_SAVE_ON_BATTERY &&
            batteryManager.getPowerSource() != prevSource) {
            bool onBattery = !batteryManager.isUsbConnected();
            wifiManager.setPowerSave(onBattery);
            logBoth(onBattery ? "🔋 On battery — WiFi power save ON"
                              : "🔌 On USB — WiFi power save OFF");
        }

        // Check if we need to enter deep sleep
        checkBatteryAndSleep();

        // Log battery status if low — не чаще LOW_BATTERY_LOG_INTERVAL,
        // иначе спамим каждые 10 секунд
        if (batteryManager.isLowBattery() && !batteryManager.isUsbConnected()) {
            static unsigned long lastLowLog = 0;
            if (lastLowLog == 0 || currentMillis - lastLowLog >= LOW_BATTERY_LOG_INTERVAL) {
                lastLowLog = currentMillis;
                logBoth("⚠️ Low battery: " + String(batteryManager.getVoltage(), 2) + "V (" +
                       String(batteryManager.getPercent()) + "%)");
            }
        }
    }

    // Check WiFi connection
    // checkConnection() теперь НЕ блокирует — можно вызывать каждый тик.
    // Внутри она сама ограничивает частоту попыток реконнекта.
    {
        bool wasConnected = wifiManager.isConnected();
        wifiManager.checkConnection();
        bool isNowConnected = wifiManager.isConnected();
        if (!wasConnected && isNowConnected)
            logBoth("WiFi reconnected: " + wifiManager.getSSID() +
                    " (" + wifiManager.getIP() + "  " +
                    String(wifiManager.getRSSI()) + " dBm)");
        else if (wasConnected && !isNowConnected)
            Serial.println("WiFi connection lost — reconnecting...");
    }

    // Web request processing and WebSocket.
    // Вызываем ВСЕГДА — веб-сервер должен отвечать даже во время реконнекта WiFi.
    webServer.handleClient();

    // Read sensor data
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;

        if (sensorManager.update()) {
            float temp     = sensorManager.getTemperature();
            float humid    = sensorManager.getHumidity();
            float avgTemp  = sensorManager.getAvgTemp();
            float avgHumid = sensorManager.getAvgHumid();

            String log = "T: " + String(temp, 1) + "C | H: " + String(humid, 1) + "%";
            Serial.println(log);

            if (wifiManager.isConnected()) {
                webServer.broadcastLog(log);

                static int readCount = 0;
                if (++readCount % 3 == 0) {
                    webServer.broadcastLog("Min: T=" + String(sensorManager.getMinTemp(), 1) +
                                          "C  H=" + String(sensorManager.getMinHumid(), 1) + "%");
                    webServer.broadcastLog("Max: T=" + String(sensorManager.getMaxTemp(), 1) +
                                          "C  H=" + String(sensorManager.getMaxHumid(), 1) + "%");
                    webServer.broadcastLog("Avg: T=" + String(avgTemp, 1) +
                                          "C  H=" + String(avgHumid, 1) + "%");
                    float dp = WeatherCalculations::calculateDewPoint(temp, humid);
                    float hi = WeatherCalculations::calculateHeatIndex(temp, humid);
                    webServer.broadcastLog("Dew Point:  " + String(dp, 1) + "C");
                    webServer.broadcastLog("Heat Index: " + String(hi, 1) + "C");
                }
            }
        } else {
            logBoth("Sensor error (count: " +
                    String(sensorManager.getReadErrorCount()) + ")");
        }
    }

    updateCPUUsage();
    printStatus();

    unsigned long loopEnd = micros();
    busyTime += (loopEnd - loopStart);
    // idleTime фиксированно = 10мс на итерацию (время delay в конце loop).
    // g_cpuUsage = busyTime / (busyTime + idleTime) — относительная нагрузка loop
    // на фоне intentional delay, а не реальный % времени CPU.
    delay(10);
    idleTime += 10000;
}
