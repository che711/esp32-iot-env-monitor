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
// ⚠️ ВНИМАНИЕ: GPIO2 — strapping-пин ESP32-C3 (должен быть HIGH при сбросе).
// CHRG у TP4056 — открытый коллектор, во время зарядки тянет линию в LOW.
// Если девайс перезагрузится ВО ВРЕМЯ зарядки, он может не стартовать
// (уйдёт в неверный boot mode). Рекомендация: перенести CHRG на GPIO4 или
// GPIO5, либо поставить развязку (например, диод + отдельный пуллап).
inline constexpr int BATTERY_CHRG_PIN = 2;  // GPIO2 - CHRG pin от TP4056
inline constexpr int BATTERY_STDBY_PIN = 3; // GPIO3 - STDBY pin от TP4056

// ============================================
// Timing Configuration
// ============================================
inline constexpr unsigned long SENSOR_INTERVAL        = 30000;  // Опрос датчика каждую 30 сек
inline constexpr unsigned long WIFI_CHECK_INTERVAL    = 300000; // Проверка WiFi каждые 5 мин (было 50 мин!)
inline constexpr unsigned long STATS_UPDATE_INTERVAL  = 5000;  // Обновление статистики каждые 5 сек
inline constexpr unsigned long BATTERY_CHECK_INTERVAL = 10000;  // Проверка батареи каждые 10 сек

// ============================================
// History Configuration
// ============================================
inline constexpr int HISTORY_SIZE        = 60;   // 60 точек × 30с (SENSOR_INTERVAL) = окно 30 минут
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
// Количество усреднений ADC (должно быть >= 8, кратно 4 для trimmed mean)
inline constexpr int BATTERY_ADC_SAMPLES = 20;

// Делитель 100k / 100k → коэффициент 2.0
inline constexpr float BATTERY_DIVIDER_RATIO = 2.0f;

// Калибровочный коэффициент. Теперь измерение идёт через
// analogReadMilliVolts() с заводской eFuse-калибровкой чипа, поэтому
// старое значение 0.901 (подобранное под сырой analogRead) НЕ актуально.
// Калибровка: измерьте мультиметром реальное напряжение батареи,
// сравните с показаниями в логе [BAT] и задайте:
//   CORRECTION = V_мультиметр / V_показанное
// Обычно остаётся в пределах 0.97–1.03 (разброс резисторов делителя).
inline constexpr float BATTERY_ADC_CORRECTION = 1.0f;

// Порог предупреждения (3.5V для Li-Ion)
inline constexpr float BATTERY_WARN_VOLTAGE = 3.5f;

// Критический порог (3.3V для Li-Ion)
inline constexpr float BATTERY_CRITICAL_VOLTAGE = 3.3f;

// Как часто писать в лог предупреждение о низком заряде (5 минут),
// чтобы не спамить каждые BATTERY_CHECK_INTERVAL
inline constexpr unsigned long LOW_BATTERY_LOG_INTERVAL = 300000;

// WiFi modem sleep при питании от батареи (экономит ~40-60 мА в среднем
// ценой чуть большей задержки ответа веб-интерфейса). На USB — отключается.
inline constexpr bool WIFI_POWER_SAVE_ON_BATTERY = true;

// ============================================
// Deep Sleep Configuration
// ============================================
inline constexpr bool  DEEP_SLEEP_ENABLED = true;  // Включить deep sleep при критическом заряде

// Время deep sleep при критическом заряде (5 минут)
inline constexpr uint64_t DEEP_SLEEP_CRITICAL_DURATION = 5ULL * 60 * 1000000; // в микросекундах

// Максимальное время deep sleep при прогрессивном увеличении (60 минут).
// Каждый цикл "проснулся → батарея всё ещё критическая → уснул"
// удваивает длительность сна: 5 → 10 → 20 → 40 → 60 → 60...
// Это спасает батарею от глубокого переразряда, когда её никто не заряжает.
inline constexpr uint64_t DEEP_SLEEP_MAX_DURATION = 60ULL * 60 * 1000000;

// Минимальное время работы перед deep sleep (30 секунд)
inline constexpr unsigned long MIN_UPTIME_BEFORE_SLEEP = 30000;

#endif // CONFIG_H
