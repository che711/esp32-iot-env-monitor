#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include "config.h"
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "battery_manager.h"

// ============================================
// Экраны дисплея
// ============================================
enum class DisplayScreen : uint8_t {
    MAIN = 0,   // Крупно температура + влажность
    STATS,      // Min / Max / Avg + точка росы и heat index
    SYSTEM,     // IP, WiFi, память, CPU, uptime
    BATTERY,    // Напряжение, заряд, статус TP4056
    COUNT       // Служебное — количество экранов
};

// ============================================
// Класс управления OLED-дисплеем SSD1306 128x64
// ============================================
// Живёт на общей с AHT10 шине I2C. Wire.begin() ВЫЗЫВАЕТСЯ СНАРУЖИ (в setup()),
// поэтому begin() дисплея шину не трогает — иначе двойная инициализация.
class DisplayManager {
public:
    DisplayManager(SensorManager* sensor, WiFiManager* wifi, BatteryManager* battery);

    // Возвращает false, если дисплей не найден на шине — это не фатально,
    // устройство продолжает работать как раньше (веб + Serial)
    bool begin();

    // Вызывать в loop() — неблокирующая, сама выдерживает DISPLAY_UPDATE_INTERVAL
    void update();

    // Навигация
    void nextScreen();
    void setScreen(DisplayScreen screen);
    DisplayScreen getScreen() const { return _screen; }

    // Питание
    void setPower(bool on);
    void togglePower();
    void wake();          // Включить и сбросить таймер автогашения
    void powerOff();      // Жёстко погасить (перед deep sleep)
    bool isOn() const { return _isOn; }
    bool isPresent() const { return _present; }

    // Сообщения вне обычного цикла отрисовки
    void showSplash(const char* version);
    void showMessage(const char* title, const char* line1 = nullptr,
                     const char* line2 = nullptr);

private:
    Adafruit_SSD1306 _display;

    SensorManager*  _sensor;
    WiFiManager*    _wifi;
    BatteryManager* _battery;

    bool          _present;        // Дисплей реально найден на шине
    bool          _isOn;           // Матрица включена
    bool          _autoOff;        // Погашен политикой энергосбережения, а не вручную
    DisplayScreen _screen;

    unsigned long _lastUpdate;     // Последняя перерисовка
    unsigned long _lastActivity;   // Последнее взаимодействие (для автогашения)
    unsigned long _lastRotate;     // Последняя автосмена экрана

    // Отрисовка экранов
    void drawMain();
    void drawStats();
    void drawSystem();
    void drawBattery();

    // Общие элементы
    void drawHeader(const char* title);
    void drawTopBar();
    void drawBatteryIcon(int x, int y, int percent, bool charging);
    void drawLabeledRow(int y, const char* label, const String& value);

    // Утилиты
    bool probeAddress(uint8_t addr) const;
    void applyPowerPolicy();
    String formatUptime() const;
};

#endif // DISPLAY_MANAGER_H
