#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
// Используйте inline для constexpr в C++17 или определите в .cpp файле
inline const char* WIFI_SSID = "SkyNet";        // Измените на имя вашей сети
inline const char* WIFI_PASSWORD = "password";   // Измените на пароль вашей сети

// ============================================
// I2C Configuration
// ============================================
inline constexpr int I2C_SDA = 8;           // GPIO для SDA
inline constexpr int I2C_SCL = 9;           // GPIO для SCL
inline constexpr uint32_t I2C_FREQ = 100000; // Частота I2C (100 кГц)

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL = 3000;      // Интервал чтения датчика (мс)
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 10000; // Интервал проверки WiFi (мс)

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE = 60;  // Размер истории (60 точек = 3 минуты при интервале 3 сек)

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
