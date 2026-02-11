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
// I2C Configuration (ESP32 Super Mini)
// ============================================
inline constexpr int      I2C_SDA  = 8;
inline constexpr int      I2C_SCL  = 9;
inline constexpr uint32_t I2C_FREQ = 100000;

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL       = 5000;   // Опрос датчика каждые 5 сек
inline constexpr unsigned long WIFI_CHECK_INTERVAL   = 300000; // Проверка WiFi каждые 300 сек
//                                                    ^^^^^^ БЫЛО 3000000 (50 мин!) — исправлено
inline constexpr unsigned long STATS_UPDATE_INTERVAL = 10000;  // Обновление статистики каждые 10 сек

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE        = 60;   // 60 точек = 5 мин при интервале 5 сек
inline constexpr int HOURLY_HISTORY_SIZE = 1200; // 1200 точек = 1 час

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
// LED Configuration
// ============================================
inline constexpr int  LED_BUILTIN_PIN = 8;    // GPIO8 на ESP32 Super Mini
inline constexpr bool LED_ENABLED     = false; // Отключить для экономии энергии

// ===============================
// BATTERY CONFIGURATION
// ===============================

// Количество усреднений ADC
inline constexpr int BATTERY_ADC_SAMPLES = 20;

// Опорное напряжение ADC (ESP32-C3 реальное ≈ 3.3)
inline constexpr float BATTERY_ADC_REF_VOLTAGE = 3.3f;

// Делитель 100k / 100k → коэффициент 2.0
inline constexpr float BATTERY_DIVIDER_RATIO = 2.0f;

// Поправочный коэффициент калибровки
inline constexpr float BATTERY_ADC_CORRECTION = 1.00f;

// Порог предупреждения
inline constexpr float BATTERY_WARN_VOLTAGE = 3.5f;

// Критический порог
inline constexpr float BATTERY_CRITICAL_VOLTAGE = 3.3f;


#endif // CONFIG_H
