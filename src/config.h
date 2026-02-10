#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
inline const char* WIFI_SSID = "SkyNet";
inline const char* WIFI_PASSWORD = "password";
inline constexpr int WIFI_MAX_RETRY = 30;
inline constexpr unsigned long WIFI_TIMEOUT = 15000;

// ============================================
// I2C Configuration (ESP32 Super Mini)
// ============================================
inline constexpr int I2C_SDA = 8;
inline constexpr int I2C_SCL = 9;
inline constexpr uint32_t I2C_FREQ = 100000;

// ============================================
// Battery Configuration (18650 + TP4056)
// ============================================
inline constexpr int BATTERY_ADC_PIN = 0;           // GPIO0 - ADC for battery voltage
inline constexpr int BATTERY_CHRG_PIN = 1;          // GPIO1 - TP4056 CHRG pin (LOW when charging)
inline constexpr int BATTERY_STDBY_PIN = 2;         // GPIO2 - TP4056 STDBY pin (LOW when charged)
inline constexpr int BATTERY_USB_DETECT_PIN = -1;   // Optional: GPIO for USB detection (-1 if not used)

// Battery voltage parameters
inline constexpr float BATTERY_VOLTAGE_DIVIDER = 2.0;  // Voltage divider ratio (e.g., 100K+100K = 2.0)
inline constexpr float BATTERY_VOLTAGE_FULL = 4.2;     // Fully charged voltage
inline constexpr float BATTERY_VOLTAGE_NOMINAL = 3.7;  // Nominal voltage
inline constexpr float BATTERY_VOLTAGE_EMPTY = 3.0;    // Empty voltage (cutoff)

// Battery percentage thresholds
inline constexpr float BATTERY_LOW_PERCENT = 20.0;      // Low battery warning
inline constexpr float BATTERY_CRITICAL_PERCENT = 10.0; // Critical battery (force deep sleep)

// ============================================
// Deep Sleep Configuration
// ============================================
inline constexpr bool DEEP_SLEEP_ENABLED = true;             // Enable/disable deep sleep
inline constexpr unsigned long DEEP_SLEEP_DURATION = 300;    // Sleep duration in seconds (5 min)
inline constexpr unsigned long DEEP_SLEEP_DURATION_LOW = 600; // Sleep when battery low (10 min)
inline constexpr unsigned long DEEP_SLEEP_DURATION_CRITICAL = 1800; // Sleep when critical (30 min)

// Deep sleep triggers
inline constexpr bool DEEP_SLEEP_ON_BATTERY = true;   // Sleep when on battery power
inline constexpr bool STAY_AWAKE_WHEN_CHARGING = true; // Stay awake when USB connected

// Awake time settings
inline constexpr unsigned long AWAKE_TIME_NORMAL = 60000;    // Stay awake for 60 sec (read sensor, send data)
inline constexpr unsigned long AWAKE_TIME_FAST = 30000;      // Fast mode: 30 sec (low battery)

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL = 5000;       // Read sensor every 5 sec
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 30000;  // Check WiFi every 30 sec (reduced for battery)
inline constexpr unsigned long STATS_UPDATE_INTERVAL = 10000; // Statistics update every 10 sec
inline constexpr unsigned long BATTERY_CHECK_INTERVAL = 30000; // Check battery every 30 sec

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE = 60;              // 60 points = 5 min at 5 sec interval
inline constexpr int HOURLY_HISTORY_SIZE = 720;      // 720 points = 1 hour at 5 sec interval

// ============================================
// Web Server Configuration
// ============================================
inline constexpr int WEB_SERVER_PORT = 80;
inline constexpr int WEB_RESPONSE_TIMEOUT = 1000;
inline constexpr int WEBSOCKET_PORT = 81;

// ============================================
// Serial Configuration
// ============================================
inline constexpr unsigned long SERIAL_BAUD = 115200;

// ============================================
// Memory Configuration
// ============================================
inline constexpr size_t JSON_BUFFER_SIZE = 2048;
inline constexpr size_t HISTORY_BUFFER_SIZE = 4096;

// ============================================
// System Limits
// ============================================
inline constexpr float TEMP_INIT_MIN = 999.0;
inline constexpr float TEMP_INIT_MAX = -999.0;
inline constexpr float HUMID_INIT_MIN = 999.0;
inline constexpr float HUMID_INIT_MAX = -999.0;

// ============================================
// Sensor Validation
// ============================================
inline constexpr float TEMP_MIN_VALID = -40.0;
inline constexpr float TEMP_MAX_VALID = 85.0;
inline constexpr float HUMID_MIN_VALID = 0.0;
inline constexpr float HUMID_MAX_VALID = 100.0;

// ============================================
// LED Configuration
// ============================================
inline constexpr int LED_BUILTIN_PIN = 8;  // GPIO8 on ESP32 Super Mini
inline constexpr bool LED_ENABLED = false;  // Can be disabled to save power

// ============================================
// Power Saving Features
// ============================================
inline constexpr bool WIFI_POWER_SAVE = true;        // Enable WiFi power save mode
inline constexpr bool REDUCE_CPU_FREQ = true;        // Reduce CPU frequency on battery
inline constexpr int CPU_FREQ_BATTERY = 80;          // CPU frequency on battery (MHz)
inline constexpr int CPU_FREQ_CHARGING = 160;        // CPU frequency when charging (MHz)

// ============================================
// RTC Memory (for deep sleep data persistence)
// ============================================
inline constexpr int RTC_DATA_VERSION = 1;  // Version for RTC data structure

#endif // CONFIG_H
