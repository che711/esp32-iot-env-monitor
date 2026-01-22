#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Configuration
inline const char* WIFI_SSID = "SkyNet";
inline const char* WIFI_PASSWORD = "password";

// I2C Configuration
inline constexpr int I2C_SDA = 8;
inline constexpr int I2C_SCL = 9;
inline constexpr uint32_t I2C_FREQ = 100000;

// Timing Configuration
inline constexpr unsigned long SENSOR_INTERVAL = 3000;
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 10000;

// History Configuration
inline constexpr int HISTORY_SIZE = 60;
inline constexpr int HOURLY_HISTORY_SIZE = 1200;  // ← ЭТА СТРОКА!

// Web Server Configuration
inline constexpr int WEB_SERVER_PORT = 80;

// Serial Configuration
inline constexpr unsigned long SERIAL_BAUD = 115200;

// System Limits
inline constexpr float TEMP_INIT_MIN = 999.0;
inline constexpr float TEMP_INIT_MAX = -999.0;
inline constexpr float HUMID_INIT_MIN = 999.0;
inline constexpr float HUMID_INIT_MAX = -999.0;

#endif // CONFIG_H
