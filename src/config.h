#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
inline const char* WIFI_SSID     = "SkyNet";
inline const char* WIFI_PASSWORD = "password";
inline constexpr int           WIFI_MAX_RETRY = 30;
inline constexpr unsigned long WIFI_TIMEOUT   = 15000;

// ============================================
// I2C Configuration (ESP32-C3 Super Mini)
// ============================================
inline constexpr int      I2C_SDA  = 8;
inline constexpr int      I2C_SCL  = 9;
inline constexpr uint32_t I2C_FREQ = 100000;

// ============================================
// GPIO Pin Configuration
// ============================================
// LED отключен, чтобы не конфликтовать с I2C_SDA
inline constexpr int  LED_BUILTIN_PIN = -1;    // Отключен (был конфликт с GPIO8)
inline constexpr bool LED_ENABLED     = false; // Отключить для экономии энергии

// Battery Manager Pins (TP4056 + делитель напряжения)
inline constexpr int BATTERY_ADC_PIN  = 0;  // GPIO0 - ADC для измерения напряжения
inline constexpr int BATTERY_CHRG_PIN = 2;  // GPIO2 - CHRG pin от TP4056
inline constexpr int BATTERY_STDBY_PIN = 3; // GPIO3 - STDBY pin от TP4056

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL        = 10000;  // Опрос датчика каждые 10 сек
inline constexpr unsigned long WIFI_CHECK_INTERVAL    = 300000; // Проверка WiFi каждые 5 мин (было 50 мин!)
inline constexpr unsigned long STATS_UPDATE_INTERVAL  = 10000;  // Обновление статистики каждые 10 сек
inline constexpr unsigned long BATTERY_CHECK_INTERVAL = 30000;  // Проверка батареи каждые 30 сек

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE        = 60;   // 60 points = 10 min at SENSOR_INTERVAL=10s
// HOURLY_HISTORY_SIZE removed — replaced with O(1) running average in SensorManager

// ============================================
// Web Server Configuration
// ============================================
inline constexpr int WEB_SERVER_PORT      = 80;
inline constexpr int WEB_RESPONSE_TIMEOUT = 1000;

// ============================================
// Serial Configuration
// ============================================
inline constexpr unsigned long SERIAL_BAUD = 115200;

// ============================================
// Memory Configuration
// ============================================
inline constexpr size_t JSON_BUFFER_SIZE    = 2048;
inline constexpr size_t HISTORY_BUFFER_SIZE = 4096;

// ============================================
// System Limits
// ============================================
inline constexpr float TEMP_INIT_MIN  = 999.0;
inline constexpr float TEMP_INIT_MAX  = -999.0;
inline constexpr float HUMID_INIT_MIN = 999.0;
inline constexpr float HUMID_INIT_MAX = -999.0;

// ============================================
// Sensor Validation
// ============================================
inline constexpr float TEMP_MIN_VALID  = -40.0;
inline constexpr float TEMP_MAX_VALID  = 85.0;
inline constexpr float HUMID_MIN_VALID = 0.0;
inline constexpr float HUMID_MAX_VALID = 100.0;

// ============================================
// Battery Configuration
// ============================================
// Количество усреднений ADC
inline constexpr int BATTERY_ADC_SAMPLES = 20;

// Опорное напряжение ADC (ESP32-C3 реальное ≈ 3.3)
inline constexpr float BATTERY_ADC_REF_VOLTAGE = 3.3f;

// Делитель 100k / 100k → коэффициент 2.0
inline constexpr float BATTERY_DIVIDER_RATIO = 2.0f;

inline constexpr float BATTERY_ADC_CORRECTION = 0.944f;

// Порог предупреждения (3.5V для Li-Ion)
inline constexpr float BATTERY_WARN_VOLTAGE = 3.5f;

// Критический порог (3.3V для Li-Ion)
inline constexpr float BATTERY_CRITICAL_VOLTAGE = 3.3f;

// ============================================
// Deep Sleep Configuration
// ============================================
inline constexpr bool  DEEP_SLEEP_ENABLED = true;  // Включить deep sleep при критическом заряде

// Время deep sleep при критическом заряде (5 минут)
inline constexpr unsigned long DEEP_SLEEP_CRITICAL_DURATION = 5 * 60 * 1000000ULL; // в микросекундах

// Время deep sleep при низком заряде (1 минута)
inline constexpr unsigned long DEEP_SLEEP_LOW_DURATION = 1 * 60 * 1000000ULL; // в микросекундах

// Минимальное время работы перед deep sleep (30 секунд)
inline constexpr unsigned long MIN_UPTIME_BEFORE_SLEEP = 30000;

#endif // CONFIG_H
