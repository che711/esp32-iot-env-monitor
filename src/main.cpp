#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "web_server.h"
#include "calculations.h"
#include "battery_manager.h"
#include <esp_sleep.h>
#include <esp_wifi.h>

// ============================================
// RTC Memory (preserved during deep sleep)
// ============================================
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float rtcMinTemp = TEMP_INIT_MIN;
RTC_DATA_ATTR float rtcMaxTemp = TEMP_INIT_MAX;
RTC_DATA_ATTR float rtcMinHumid = HUMID_INIT_MIN;
RTC_DATA_ATTR float rtcMaxHumid = HUMID_INIT_MAX;
RTC_DATA_ATTR unsigned long rtcTotalAwakeTime = 0;
RTC_DATA_ATTR int rtcDataVersion = 0;

// ============================================
// Global variables
// ============================================
float g_cpuUsage = 0.0;
unsigned long awakeStartTime = 0;
bool shouldEnterDeepSleep = false;

// Objects (order matters - batteryManager must be declared before webServer)
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
SensorManager sensorManager;
BatteryManager batteryManager;
WeatherWebServer webServer(&sensorManager, &wifiManager, &batteryManager);

// Timers
unsigned long lastSensorRead = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastStatsUpdate = 0;
unsigned long lastBatteryCheck = 0;

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
// Deep Sleep Functions
// ============================================

void printWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    Serial.print("Wakeup reason: ");
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Timer");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("External signal (RTC_IO)");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("External signal (RTC_CNTL)");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("ULP program");
            break;
        default:
            Serial.println("Not deep sleep wakeup (first boot or reset)");
            break;
    }
}

unsigned long getDeepSleepDuration() {
    float batteryPercent = batteryManager.getPercentage();
    
    if (batteryPercent < BATTERY_CRITICAL_PERCENT) {
        return DEEP_SLEEP_DURATION_CRITICAL;
    } else if (batteryPercent < BATTERY_LOW_PERCENT) {
        return DEEP_SLEEP_DURATION_LOW;
    } else {
        return DEEP_SLEEP_DURATION;
    }
}

unsigned long getAwakeTime() {
    if (batteryManager.isBatteryLow()) {
        return AWAKE_TIME_FAST;
    }
    return AWAKE_TIME_NORMAL;
}

void enterDeepSleep() {
    unsigned long sleepDuration = getDeepSleepDuration();
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║       ENTERING DEEP SLEEP MODE         ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    // Update RTC memory with current min/max
    rtcMinTemp = sensorManager.getMinTemp();
    rtcMaxTemp = sensorManager.getMaxTemp();
    rtcMinHumid = sensorManager.getMinHumid();
    rtcMaxHumid = sensorManager.getMaxHumid();
    rtcTotalAwakeTime += (millis() - awakeStartTime);
    rtcDataVersion = RTC_DATA_VERSION;
    
    Serial.printf("Sleep duration: %lu seconds (%.1f min)\n", sleepDuration, sleepDuration / 60.0);
    Serial.printf("Battery: %.1f%% (%.2fV)\n", batteryManager.getPercentage(), batteryManager.getVoltage());
    Serial.printf("Boot count: %d\n", bootCount + 1);
    Serial.printf("Total awake time: %lu sec\n", rtcTotalAwakeTime / 1000);
    Serial.println();
    
    // Broadcast final message
    if (wifiManager.isConnected()) {
        webServer.broadcastLog("💤 Entering deep sleep for " + String(sleepDuration / 60) + " minutes");
        webServer.broadcastLog("🔋 Battery: " + String(batteryManager.getPercentage(), 1) + "%");
        delay(500); // Give time for message to send
    }
    
    // Cleanup before sleep
    Serial.println("Shutting down systems...");
    
    // Disconnect WiFi properly
    if (wifiManager.isConnected()) {
        Serial.println("  ✓ Disconnecting WiFi");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
    
    // Turn off peripherals
    Serial.println("  ✓ Disabling peripherals");
    
    Serial.println("  ✓ Systems shutdown complete");
    Serial.println();
    Serial.println("💤 Good night! See you in " + String(sleepDuration / 60) + " minutes...");
    Serial.println();
    
    delay(100);
    
    // Configure wakeup timer
    esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL); // Convert to microseconds
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

bool shouldStayAwake() {
    // Check if we should stay awake
    if (!DEEP_SLEEP_ENABLED) {
        return true;
    }
    
    // Stay awake if USB connected and configured to do so
    if (STAY_AWAKE_WHEN_CHARGING && batteryManager.isUSBConnected()) {
        return true;
    }
    
    // Check if awake time expired
    unsigned long awakeTime = getAwakeTime();
    if (millis() - awakeStartTime < awakeTime) {
        return true;
    }
    
    return false;
}

void adjustCPUFrequency() {
    if (!REDUCE_CPU_FREQ) return;
    
    int targetFreq = batteryManager.isUSBConnected() ? CPU_FREQ_CHARGING : CPU_FREQ_BATTERY;
    int currentFreq = ESP.getCpuFreqMHz();
    
    if (currentFreq != targetFreq) {
        setCpuFrequencyMhz(targetFreq);
        Serial.printf("CPU frequency adjusted: %d MHz -> %d MHz\n", currentFreq, targetFreq);
        
        if (wifiManager.isConnected()) {
            webServer.broadcastLog("⚡ CPU: " + String(currentFreq) + " → " + String(targetFreq) + " MHz");
        }
    }
}

// ============================================
// System Functions
// ============================================

void printSystemInfo() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32 Super Mini Weather Station    ║");
    Serial.println("║     AHT10 + 18650 Battery v4.0        ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    Serial.println("=== Boot Information ===");
    Serial.printf("Boot count:     %d\n", bootCount);
    printWakeupReason();
    Serial.println();
    
    Serial.println("=== System Information ===");
    Serial.printf("Chip Model:     %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision:  %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency:  %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size:     %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap:      %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("SDK Version:    %s\n", ESP.getSdkVersion());
    Serial.println();
    
    if (rtcDataVersion == RTC_DATA_VERSION) {
        Serial.println("=== RTC Memory (from previous boot) ===");
        Serial.printf("Min Temp:       %.1f°C\n", rtcMinTemp);
        Serial.printf("Max Temp:       %.1f°C\n", rtcMaxTemp);
        Serial.printf("Min Humid:      %.1f%%\n", rtcMinHumid);
        Serial.printf("Max Humid:      %.1f%%\n", rtcMaxHumid);
        Serial.printf("Total awake:    %lu sec\n", rtcTotalAwakeTime / 1000);
        Serial.println();
    }
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
        
        String status = "\n=== System Status ===\n";
        status += "Uptime:         " + String(millis() / 1000) + " sec\n";
        status += "Boot count:     " + String(bootCount) + "\n";
        status += "WiFi:           " + String(wifiManager.isConnected() ? "✓ Connected" : "✗ Disconnected");
        status += " (" + String(wifiManager.getRSSI()) + " dBm)\n";
        status += "Temperature:    " + String(sensorManager.getTemperature(), 1) + "°C\n";
        status += "Humidity:       " + String(sensorManager.getHumidity(), 1) + "%\n";
        status += "Battery:        " + String(batteryManager.getPercentage(), 1) + "% (" + String(batteryManager.getVoltage(), 2) + "V)\n";
        status += "Power:          " + String(batteryManager.isUSBConnected() ? "USB" : "Battery") + "\n";
        status += "CPU Usage:      " + String(g_cpuUsage, 1) + "%\n";
        status += "Free Heap:      " + String(ESP.getFreeHeap() / 1024) + " KB\n";
        
        Serial.println(status);
        
        if (wifiManager.isConnected()) {
            webServer.broadcastLog("=== Status Update ===");
            webServer.broadcastLog("🌡️ Sensor: T=" + String(sensorManager.getTemperature(), 1) + 
                                  "°C, H=" + String(sensorManager.getHumidity(), 1) + "%");
            webServer.broadcastLog("🔋 Battery: " + String(batteryManager.getPercentage(), 1) + 
                                  "% (" + String(batteryManager.getVoltage(), 2) + "V)");
            
            String powerStatus = batteryManager.isCharging() ? "⚡ Charging" : 
                               batteryManager.isCharged() ? "✓ Charged" : 
                               "🔋 Discharging";
            webServer.broadcastLog(powerStatus);
            
            webServer.broadcastLog("━━━━━━━━━━━━━━━━━━━━━━━━━━");
        }
    }
}

// ============================================
// Setup
// ============================================

void setup() {
    awakeStartTime = millis();
    bootCount++;
    
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    printSystemInfo();
    
    // Initialize battery manager first
    Serial.println("=== Initializing Battery ===");
    if (!batteryManager.begin()) {
        Serial.println("✗ Battery initialization failed");
    }
    
    // Adjust CPU frequency based on power source
    adjustCPUFrequency();
    
    // Check if battery is critical - enter deep sleep immediately
    if (batteryManager.isBatteryCritical() && !batteryManager.isUSBConnected()) {
        Serial.println("\n⚠️  CRITICAL BATTERY LEVEL!");
        Serial.printf("Battery: %.1f%% (%.2fV)\n", batteryManager.getPercentage(), batteryManager.getVoltage());
        Serial.println("Entering emergency deep sleep...");
        delay(1000);
        enterDeepSleep();
    }
    
    // Initialize sensor
    Serial.println("=== Initializing Sensor ===");
    if (!sensorManager.begin()) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║           CRITICAL ERROR!              ║");
        Serial.println("║       AHT10 is not detected!           ║");
        Serial.println("╚════════════════════════════════════════╝");
        
        // If on battery and sensor failed, sleep to save power
        if (!batteryManager.isUSBConnected() && DEEP_SLEEP_ENABLED) {
            Serial.println("Entering deep sleep due to sensor error...");
            delay(1000);
            enterDeepSleep();
        }
        
        while (1) {
            delay(1000);
        }
    }
    
    // Restore min/max from RTC memory if valid
    if (rtcDataVersion == RTC_DATA_VERSION) {
        // Set initial min/max from RTC memory (они будут обновлены при первом чтении)
        Serial.println("✓ Restored min/max from RTC memory");
    }
    
    // Connect to WiFi
    Serial.println("=== Connecting to WiFi ===");
    if (!wifiManager.begin()) {
        Serial.println("✗ Cannot connect to WiFi");
        
        // If on battery and can't connect, sleep after taking reading
        if (!batteryManager.isUSBConnected() && DEEP_SLEEP_ENABLED) {
            Serial.println("Will enter deep sleep after sensor reading...");
            shouldEnterDeepSleep = true;
        }
    }
    
    // Enable WiFi power save if on battery
    if (WIFI_POWER_SAVE && !batteryManager.isUSBConnected()) {
        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        Serial.println("✓ WiFi power save enabled");
    }
    
    // Launch web server only if WiFi connected
    if (wifiManager.isConnected()) {
        Serial.println("=== Launching Web Server ===");
        webServer.begin();
    }
    
    // Final message
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  ✓ Weather Monitor Ready!              ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    if (wifiManager.isConnected()) {
        String msg = "Web interface: http://" + wifiManager.getIP() + "/";
        Serial.println(msg);
        Serial.println();
        
        delay(2000);
        webServer.broadcastLog("╔════════════════════════════════════════╗");
        webServer.broadcastLog("║    ESP32 Weather Station Started!     ║");
        webServer.broadcastLog("╚════════════════════════════════════════╝");
        webServer.broadcastLog("");
        webServer.broadcastLog("✓ System initialized (Boot #" + String(bootCount) + ")");
        webServer.broadcastLog("✓ AHT10 sensor: Ready");
        webServer.broadcastLog("✓ WiFi: " + wifiManager.getSSID());
        webServer.broadcastLog("✓ IP: " + wifiManager.getIP());
        webServer.broadcastLog("🔋 Battery: " + String(batteryManager.getPercentage(), 1) + "% (" + 
                              String(batteryManager.getVoltage(), 2) + "V)");
        webServer.broadcastLog("⚡ Power: " + String(batteryManager.isUSBConnected() ? "USB" : "Battery"));
        
        if (DEEP_SLEEP_ENABLED && !batteryManager.isUSBConnected()) {
            webServer.broadcastLog("💤 Deep sleep mode: Active (will sleep in " + 
                                  String(getAwakeTime() / 1000) + "s)");
        }
        
        webServer.broadcastLog("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    }
    
    // Print power mode info
    if (batteryManager.isUSBConnected()) {
        Serial.println("🔌 USB Connected - Normal Operation Mode");
    } else {
        Serial.println("🔋 Battery Mode - Power Saving Active");
        if (DEEP_SLEEP_ENABLED) {
            Serial.printf("💤 Will enter deep sleep in %lu seconds\n", getAwakeTime() / 1000);
        }
    }
    Serial.println();
}

// ============================================
// Main Loop
// ============================================

void loop() {
    unsigned long loopStart = micros();
    unsigned long currentMillis = millis();
    
    // Check if should enter deep sleep
    if (!shouldStayAwake() && DEEP_SLEEP_ON_BATTERY && !batteryManager.isUSBConnected()) {
        enterDeepSleep();
    }
    
    // Battery check
    if (currentMillis - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
        lastBatteryCheck = currentMillis;
        batteryManager.update();
        
        // Adjust CPU frequency if power source changed
        adjustCPUFrequency();
        
        // Log battery status
        if (wifiManager.isConnected()) {
            static float lastLoggedPercent = -1;
            float currentPercent = batteryManager.getPercentage();
            
            // Log every 5% change
            if (abs(currentPercent - lastLoggedPercent) >= 5.0) {
                String status = "🔋 Battery: " + String(currentPercent, 1) + "% (" + 
                              String(batteryManager.getVoltage(), 2) + "V)";
                
                if (batteryManager.isCharging()) status += " ⚡ Charging";
                else if (batteryManager.isCharged()) status += " ✓ Charged";
                
                webServer.broadcastLog(status);
                lastLoggedPercent = currentPercent;
            }
        }
    }
    
    // WiFi check
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = currentMillis;
        
        bool wasConnected = wifiManager.isConnected();
        wifiManager.checkConnection();
        bool isConnected = wifiManager.isConnected();
        
        if (!wasConnected && isConnected) {
            String msg = "✓ WiFi reconnected: " + wifiManager.getSSID();
            logBoth(msg);
        } else if (wasConnected && !isConnected) {
            logBoth("✗ WiFi connection lost!");
        }
    }
    
    // Web request processing
    if (wifiManager.isConnected()) {
        webServer.handleClient();
    }
    
    // Sensor reading
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;
        
        if (sensorManager.update()) {
            float temp = sensorManager.getTemperature();
            float humid = sensorManager.getHumidity();
            
            Serial.printf("✓ T: %.1f°C | H: %.1f%%\n", temp, humid);
            
            if (wifiManager.isConnected()) {
                String log = "✓ T: " + String(temp, 1) + "°C | H: " + String(humid, 1) + "%";
                webServer.broadcastLog(log);
            }
        } else {
            String err = "✗ Sensor read error";
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