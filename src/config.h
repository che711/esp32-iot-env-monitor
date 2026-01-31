#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
inline const char* WIFI_SSID = "SkyNet";        // Change this param
inline const char* WIFI_PASSWORD = "password";   // Change this param
// ============================================
// Weather API Configuration
// ============================================
inline const char* WEATHER_API_KEY = "YOUR_API";         // Get free api_key from openweathermap.org
inline const char* WEATHER_CITY = "CITY";           // Your city
inline const char* WEATHER_COUNTRY = "COUNTRY_CODE";     // Country code
inline constexpr unsigned long WEATHER_UPDATE_INTERVAL = 1200000; // 30 минут (API лимит)

// ============================================
// I2C Configuration
// ============================================
inline constexpr int I2C_SDA = 8;           // GPIO для SDA
inline constexpr int I2C_SCL = 9;           // GPIO для SCL
inline constexpr uint32_t I2C_FREQ = 100000; // Частота I2C (100 кГц)

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL = 5000;      // Интервал чтения датчика (мс)
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 600000; // Интервал проверки WiFi (мс)

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE = 60;  // Размер истории для графика (60 точек = 3 минуты при интервале 3 сек)
inline constexpr int HOURLY_HISTORY_SIZE = 1200;  // История на час (1200 точек = 60 мин при интервале 3 сек)

// ============================================
// Web Server Configuration
// ============================================
inline constexpr int WEB_SERVER_PORT = 80;

// ============================================
// Serial Configuration
// ============================================
inline constexpr unsigned long SERIAL_BAUD = 115200;

// ============================================
// System Limits
// ============================================
inline constexpr float TEMP_INIT_MIN = 999.0;   // Начальное значение для min температуры
inline constexpr float TEMP_INIT_MAX = -999.0;  // Начальное значение для max температуры
inline constexpr float HUMID_INIT_MIN = 999.0;  // Начальное значение для min влажности
inline constexpr float HUMID_INIT_MAX = -999.0; // Начальное значение для max влажности

#endif // CONFIG_H
