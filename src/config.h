#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
const char* WIFI_SSID = "Network";        // Измените на имя вашей сети
const char* WIFI_PASSWORD = "Password";   // Измените на пароль вашей сети

// ============================================
// I2C Configuration
// ============================================
const int I2C_SDA = 8;           // GPIO для SDA
const int I2C_SCL = 9;           // GPIO для SCL
const uint32_t I2C_FREQ = 100000; // Частота I2C (100 кГц)

// ============================================
// Timing Configuration
// ============================================
const unsigned long SENSOR_INTERVAL = 3000;      // Интервал чтения датчика (мс)
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Интервал проверки WiFi (мс)

// ============================================
// History Configuration
// ============================================
const int HISTORY_SIZE = 60;  // Размер истории (60 точек = 3 минуты при интервале 3 сек)

// ============================================
// Web Server Configuration
// ============================================
const int WEB_SERVER_PORT = 80;

// ============================================
// Serial Configuration
// ============================================
const unsigned long SERIAL_BAUD = 115200;

// ============================================
// System Limits
// ============================================
const float TEMP_INIT_MIN = 999.0;   // Начальное значение для min температуры
const float TEMP_INIT_MAX = -999.0;  // Начальное значение для max температуры
const float HUMID_INIT_MIN = 999.0;  // Начальное значение для min влажности
const float HUMID_INIT_MAX = -999.0; // Начальное значение для max влажности

#endif // CONFIG_H
