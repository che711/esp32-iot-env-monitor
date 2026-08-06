#include "display_manager.h"
#include "calculations.h"
#include <Wire.h>

// CPU usage считается в main.cpp — тот же приём, что в web_server.cpp
extern float g_cpuUsage;

// Геометрия шрифта Adafruit GFX по умолчанию: 6x8 пикселей при size=1
static constexpr int CHAR_W = 6;
static constexpr int CHAR_H = 8;

DisplayManager::DisplayManager(SensorManager* sensor, WiFiManager* wifi,
                               BatteryManager* battery)
    // clkDuring и clkAfter задаём одинаковыми: библиотека по умолчанию
    // роняет шину обратно на 100 кГц после каждой передачи, а нам нужно,
    // чтобы AHT10 тоже читался на 400 кГц
    : _display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1, I2C_FREQ, I2C_FREQ),
      _sensor(sensor), _wifi(wifi), _battery(battery),
      _present(false), _isOn(false), _autoOff(false),
      _screen(DisplayScreen::MAIN),
      _lastUpdate(0), _lastActivity(0), _lastRotate(0) {
}

// --------------------------------------------
// Инициализация
// --------------------------------------------
bool DisplayManager::probeAddress(uint8_t addr) const {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool DisplayManager::begin() {
    if (!DISPLAY_ENABLED) {
        Serial.println("Display disabled in config (DISPLAY_ENABLED=false)");
        return false;
    }

    Serial.println("Initialization SSD1306...");

    // Сначала пингуем адрес: Adafruit_SSD1306::begin() не проверяет ACK
    // надёжно и при отсутствии дисплея может вернуть true
    if (!probeAddress(DISPLAY_I2C_ADDR)) {
        Serial.printf("✗ SSD1306 not found at 0x%02X\n", DISPLAY_I2C_ADDR);
        Serial.println("  Display features disabled — device continues normally");
        return false;
    }

    // reset=false, periphBegin=false — Wire.begin() уже сделан в setup(),
    // повторная инициализация шины сломала бы уже работающий AHT10
    if (!_display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR, false, false)) {
        Serial.println("✗ ERROR: SSD1306 allocation failed");
        return false;
    }

    _present      = true;
    _isOn         = true;
    _lastActivity = millis();

    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.cp437(true);   // Нужно для символа градуса (0xF8)
    _display.display();

    Serial.printf("✓ SSD1306 initialized at 0x%02X (%dx%d)\n",
                  DISPLAY_I2C_ADDR, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    return true;
}

// --------------------------------------------
// Питание
// --------------------------------------------
void DisplayManager::setPower(bool on) {
    if (!_present || _isOn == on) return;

    _display.ssd1306_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
    _isOn = on;
}

void DisplayManager::togglePower() {
    if (!_present) return;

    setPower(!_isOn);
    _autoOff      = false;   // Ручное управление отменяет автополитику
    _lastActivity = millis();
}

void DisplayManager::wake() {
    if (!_present) return;

    _lastActivity = millis();
    if (!_isOn) {
        setPower(true);
        _autoOff = false;
    }
}

void DisplayManager::powerOff() {
    setPower(false);
}

// Автогашение на батарее: экран ест 10-20 мА, что сопоставимо с остальным
// потреблением платы. На USB — гасить незачем.
void DisplayManager::applyPowerPolicy() {
    if (!DISPLAY_AUTO_OFF_ON_BATTERY) return;

    bool onBattery = !_battery->isUsbConnected();
    unsigned long now = millis();

    if (onBattery) {
        if (_isOn && (now - _lastActivity) >= DISPLAY_BATTERY_TIMEOUT) {
            setPower(false);
            _autoOff = true;
        }
    } else if (!_isOn && _autoOff) {
        // Воткнули USB — возвращаем экран, но только если гасили его мы,
        // а не пользователь долгим нажатием
        setPower(true);
        _autoOff = false;
    }
}

// --------------------------------------------
// Навигация
// --------------------------------------------
void DisplayManager::nextScreen() {
    uint8_t next = (static_cast<uint8_t>(_screen) + 1) %
                   static_cast<uint8_t>(DisplayScreen::COUNT);
    setScreen(static_cast<DisplayScreen>(next));
}

void DisplayManager::setScreen(DisplayScreen screen) {
    _screen     = screen;
    _lastRotate = millis();
    _lastUpdate = 0;   // Перерисовать немедленно, не дожидаясь интервала
}

// --------------------------------------------
// Главный цикл
// --------------------------------------------
void DisplayManager::update() {
    if (!_present) return;

    applyPowerPolicy();

    unsigned long now = millis();

    // Погашенный экран не перерисовываем — это просто трата времени на I2C
    if (!_isOn) return;

    if (DISPLAY_AUTO_ROTATE && (now - _lastRotate) >= DISPLAY_AUTO_ROTATE_INTERVAL) {
        nextScreen();
        now = millis();
    }

    if (_lastUpdate != 0 && (now - _lastUpdate) < DISPLAY_UPDATE_INTERVAL) return;
    _lastUpdate = now;

    _display.clearDisplay();

    switch (_screen) {
        case DisplayScreen::MAIN:    drawMain();    break;
        case DisplayScreen::STATS:   drawStats();   break;
        case DisplayScreen::SYSTEM:  drawSystem();  break;
        case DisplayScreen::BATTERY: drawBattery(); break;
        default:                     drawMain();    break;
    }

    _display.display();
}

// --------------------------------------------
// Общие элементы
// --------------------------------------------
void DisplayManager::drawBatteryIcon(int x, int y, int percent, bool charging) {
    // Корпус 14x8 + носик 2x4
    _display.drawRect(x, y, 14, 8, SSD1306_WHITE);
    _display.fillRect(x + 14, y + 2, 2, 4, SSD1306_WHITE);

    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;

    int fill = (percent * 10) / 100;   // Внутренняя область — 10 пикселей
    if (fill > 0) {
        _display.fillRect(x + 2, y + 2, fill, 4, SSD1306_WHITE);
    }

    if (charging) {
        // Молния поверх заливки — инверсией, чтобы читалась на любом уровне
        _display.drawLine(x + 8, y + 1, x + 5, y + 4, SSD1306_INVERSE);
        _display.drawLine(x + 5, y + 4, x + 8, y + 4, SSD1306_INVERSE);
        _display.drawLine(x + 8, y + 4, x + 5, y + 7, SSD1306_INVERSE);
    }
}

void DisplayManager::drawTopBar() {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    // Слева — состояние WiFi
    _display.setCursor(0, 1);
    if (_wifi->isConnected()) {
        _display.print(_wifi->getRSSI());
        _display.print("dBm");
    } else {
        _display.print("no wifi");
    }

    // Справа — батарея. Если её нет физически, TP4056 отдаёт NO_BATTERY —
    // рисовать шкалу заряда в этом случае бессмысленно
    if (_battery->getChargeStatus() == ChargeStatus::NO_BATTERY) {
        const char* label = "USB";
        int w = static_cast<int>(strlen(label)) * CHAR_W;
        _display.setCursor(DISPLAY_WIDTH - w, 1);
        _display.print(label);
    } else {
        int percent = _battery->getPercent();
        drawBatteryIcon(DISPLAY_WIDTH - 16, 1, percent, _battery->isCharging());

        String pct = String(percent) + "%";
        int w = static_cast<int>(pct.length()) * CHAR_W;
        _display.setCursor(DISPLAY_WIDTH - 20 - w, 1);
        _display.print(pct);
    }

    _display.drawFastHLine(0, 10, DISPLAY_WIDTH, SSD1306_WHITE);
}

void DisplayManager::drawHeader(const char* title) {
    _display.fillRect(0, 0, DISPLAY_WIDTH, 10, SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_BLACK);
    _display.setCursor(2, 1);
    _display.print(title);
    _display.setTextColor(SSD1306_WHITE);
}

void DisplayManager::drawLabeledRow(int y, const char* label, const String& value) {
    _display.setCursor(0, y);
    _display.print(label);
    _display.setCursor(30, y);
    _display.print(value);
}

// --------------------------------------------
// Экран 1: температура и влажность крупно
// --------------------------------------------
void DisplayManager::drawMain() {
    drawTopBar();

    _display.setTextColor(SSD1306_WHITE);

    // Температура — крупно (18x24 на символ)
    _display.setTextSize(3);
    _display.setCursor(2, 16);
    _display.print(_sensor->getTemperature(), 1);

    // Градус и C — вплотную к числу, размером поменьше
    int x = _display.getCursorX() + 2;
    _display.setTextSize(2);
    _display.setCursor(x, 16);
    _display.write(0xF8);   // '°' в кодировке CP437
    _display.print("C");

    // Влажность
    _display.setTextSize(2);
    _display.setCursor(2, 46);
    _display.print(_sensor->getHumidity(), 1);
    _display.print(" %");

    // Метка RH справа, чтобы не путать с процентами заряда
    _display.setTextSize(1);
    _display.setCursor(DISPLAY_WIDTH - 2 * CHAR_W, 54);
    _display.print("RH");
}

// --------------------------------------------
// Экран 2: min / max / avg + производные величины
// --------------------------------------------
void DisplayManager::drawStats() {
    drawHeader("MIN / MAX / AVG");

    _display.setTextSize(1);

    // Шапка колонок
    _display.setCursor(30, 14);
    _display.print("T");
    _display.setCursor(78, 14);
    _display.print("H");

    struct Row { const char* label; float t; float h; };
    const Row rows[] = {
        { "min", _sensor->getMinTemp(), _sensor->getMinHumid() },
        { "max", _sensor->getMaxTemp(), _sensor->getMaxHumid() },
        { "avg", _sensor->getAvgTemp(), _sensor->getAvgHumid() },
    };

    int y = 24;
    for (const Row& r : rows) {
        _display.setCursor(0, y);
        _display.print(r.label);
        _display.setCursor(24, y);
        _display.print(r.t, 1);
        _display.setCursor(72, y);
        _display.print(r.h, 1);
        y += 10;
    }

    // Точка росы и heat index — те же формулы, что уходят в веб-интерфейс
    float t  = _sensor->getTemperature();
    float h  = _sensor->getHumidity();
    float dp = WeatherCalculations::calculateDewPoint(t, h);
    float hi = WeatherCalculations::calculateHeatIndex(t, h);

    _display.setCursor(0, 54);
    _display.print("dew ");
    _display.print(dp, 1);
    _display.setCursor(66, 54);
    _display.print("hi ");
    _display.print(hi, 1);
}

// --------------------------------------------
// Экран 3: система
// --------------------------------------------
void DisplayManager::drawSystem() {
    drawHeader("SYSTEM");

    _display.setTextSize(1);

    if (_wifi->isConnected()) {
        drawLabeledRow(14, "IP", _wifi->getIP());
        drawLabeledRow(24, "net", _wifi->getSSID() + " " +
                                  String(_wifi->getRSSI()) + "dB");
    } else {
        drawLabeledRow(14, "IP", "—");
        drawLabeledRow(24, "net", "disconnected");
    }

    drawLabeledRow(34, "ram", String(ESP.getFreeHeap() / 1024) + " KB free");
    drawLabeledRow(44, "cpu", String(g_cpuUsage, 1) + " %  " +
                              String(ESP.getCpuFreqMHz()) + "MHz");
    drawLabeledRow(54, "up",  formatUptime());
}

// --------------------------------------------
// Экран 4: батарея
// --------------------------------------------
void DisplayManager::drawBattery() {
    drawHeader("BATTERY");

    int percent = _battery->getPercent();

    // Напряжение крупно
    _display.setTextSize(2);
    _display.setCursor(2, 14);
    _display.print(_battery->getVoltage(), 2);
    _display.print(" V");

    // Процент справа от напряжения
    _display.setTextSize(1);
    String pct = String(percent) + "%";
    _display.setCursor(DISPLAY_WIDTH - static_cast<int>(pct.length()) * CHAR_W - 2, 20);
    _display.print(pct);

    // Шкала заряда
    _display.drawRect(2, 34, 124, 10, SSD1306_WHITE);
    int fill = (percent * 120) / 100;
    if (fill < 0)   fill = 0;
    if (fill > 120) fill = 120;
    if (fill > 0) {
        _display.fillRect(4, 36, fill, 6, SSD1306_WHITE);
    }

    _display.setCursor(0, 48);
    _display.print(_battery->getStatusString());

    _display.setCursor(0, 57);
    _display.print("src: ");
    _display.print(_battery->getPowerSourceString());
}

// --------------------------------------------
// Сообщения вне обычного цикла
// --------------------------------------------
void DisplayManager::showSplash(const char* version) {
    if (!_present) return;

    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    _display.setTextSize(1);
    _display.setCursor(2, 6);
    _display.print("ESP32-C3");

    _display.setTextSize(2);
    _display.setCursor(2, 20);
    _display.print("Weather");

    _display.setTextSize(1);
    _display.setCursor(2, 42);
    _display.print("Station ");
    _display.print(version);

    _display.drawFastHLine(0, 54, DISPLAY_WIDTH, SSD1306_WHITE);
    _display.setCursor(2, 57);
    _display.print("starting...");

    _display.display();
    _lastActivity = millis();
}

void DisplayManager::showMessage(const char* title, const char* line1,
                                 const char* line2) {
    if (!_present) return;

    // Сообщение может прийти, когда экран погашен политикой автогашения —
    // включаем, иначе пользователь его просто не увидит
    wake();

    _display.clearDisplay();
    drawHeader(title);

    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    if (line1) {
        _display.setCursor(0, 18);
        _display.print(line1);
    }
    if (line2) {
        _display.setCursor(0, 30);
        _display.print(line2);
    }

    _display.display();

    // Следующий update() перерисует обычный экран поверх — сбрасываем
    // таймер, чтобы сообщение успело повисеть
    _lastUpdate = millis();
}

// --------------------------------------------
// Утилиты
// --------------------------------------------
String DisplayManager::formatUptime() const {
    unsigned long up = millis() / 1000;
    unsigned long d = up / 86400;
    unsigned long h = (up % 86400) / 3600;
    unsigned long m = (up % 3600) / 60;

    String s;
    if (d > 0) s += String(d) + "d ";
    s += String(h) + "h " + String(m) + "m";
    return s;
}
