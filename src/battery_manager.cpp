#include "battery_manager.h"
#include "config.h"

// ============================================
// Таблица разряда Li-Ion 18650 (напряжение → %)
// ============================================
static const float DISCHARGE_CURVE[][2] = {
    {4.20f, 100},
    {4.18f,  99},
    {4.16f,  98},
    {4.14f,  97},
    {4.12f,  96},
    {4.10f,  95},
    {4.08f,  94},
    {4.06f,  92},
    {4.04f,  90},
    {4.00f,  88},
    {3.90f,  78},
    {3.80f,  65},
    {3.70f,  50},
    {3.60f,  35},
    {3.50f,  20},
    {3.40f,  10},
    {3.30f,   5},
    {3.00f,   0},
};

static const int CURVE_POINTS =
    sizeof(DISCHARGE_CURVE) / sizeof(DISCHARGE_CURVE[0]);

static const float ADC_MAX_VALUE = 4095.0f;   // 12-bit

// ============================================
// Конструктор
// ============================================
BatteryManager::BatteryManager(int adcPin, int chrgPin, int stdbyPin)
    : _adcPin(adcPin),
      _chrgPin(chrgPin),
      _stdbyPin(stdbyPin),
      _voltage(0.0f),
      _percent(0),
      _chargeStatus(ChargeStatus::DISCHARGING),
      _powerSource(PowerSource::BATTERY)
{}

// ============================================
// Инициализация
// ============================================
void BatteryManager::begin() {
    analogReadResolution(12);

    pinMode(_chrgPin, INPUT_PULLUP);
    pinMode(_stdbyPin, INPUT_PULLUP);

    update();

    Serial.println("✓ BatteryManager initialized");
    Serial.println("  " + getSummaryString());
    Serial.printf("  ADC GPIO%d | %d samples | ref %.2fV | divider %.2f | correction %.3f\n",
                  _adcPin, BATTERY_ADC_SAMPLES,
                  BATTERY_ADC_REF_VOLTAGE, BATTERY_DIVIDER_RATIO, BATTERY_ADC_CORRECTION);
    if (_voltage > 4.5f) {
        Serial.println("  ⚠️ Voltage > 4.5V — check BATTERY_DIVIDER_RATIO/BATTERY_ADC_CORRECTION");
    }
    Serial.println();
}

// ============================================
// Обновление
// ============================================
void BatteryManager::update() {
    float newVoltage = readVoltage();

    // Простая фильтрация резких скачков
    if (_voltage > 0.0f && fabs(newVoltage - _voltage) > 0.5f) {
        // Игнорируем нереалистичный скачок
        newVoltage = _voltage;
    }

    _voltage = newVoltage;
    _percent = voltageToPercent(_voltage);
    readChargeStatus();
}

// ============================================
// Чтение напряжения (с отладкой)
// ============================================
float BatteryManager::readVoltage() {

    long sum = 0;
    for (int i = 0; i < BATTERY_ADC_SAMPLES; i++) {
        sum += analogRead(_adcPin);
        delayMicroseconds(100);
    }

    float raw = (float)sum / BATTERY_ADC_SAMPLES;
    float vAdc = (raw / ADC_MAX_VALUE) * BATTERY_ADC_REF_VOLTAGE;
    float vBat = vAdc * BATTERY_DIVIDER_RATIO * BATTERY_ADC_CORRECTION;

    // Periodic debug log once per minute
    static unsigned long lastDebug = 0;
    unsigned long now = millis();
    if (now - lastDebug >= 60000) {
        lastDebug = now;
        Serial.printf("[BAT] raw:%.0f gpio:%.3fV bat:%.3fV\n", raw, vAdc, vBat);
    }

    if (vBat < 0.8f)  return 0.0f;
    if (vBat > 4.5f)  vBat = 4.5f;

    return vBat;
}

// ============================================
// Напряжение → %
// ============================================
int BatteryManager::voltageToPercent(float v) {

    if (v <= 0.0f)
        return 0;

    if (v >= DISCHARGE_CURVE[0][0])
        return 100;

    if (v <= DISCHARGE_CURVE[CURVE_POINTS - 1][0])
        return 0;

    for (int i = 0; i < CURVE_POINTS - 1; i++) {

        float vHigh = DISCHARGE_CURVE[i][0];
        float vLow  = DISCHARGE_CURVE[i + 1][0];

        if (v <= vHigh && v > vLow) {

            float pHigh = DISCHARGE_CURVE[i][1];
            float pLow  = DISCHARGE_CURVE[i + 1][1];

            float range = (vHigh - vLow);
            if (range <= 0.0001f)
                return (int)pLow;

            float t = (v - vLow) / range;
            float percent = pLow + t * (pHigh - pLow);

            if (percent < 0)   percent = 0;
            if (percent > 100) percent = 100;

            return (int)(percent + 0.5f); // округление
        }
    }

    return 0;
}

// ============================================
// TP4056 статус
// ============================================
void BatteryManager::readChargeStatus() {

    bool chrgLow  = (digitalRead(_chrgPin)  == LOW);
    bool stdbyLow = (digitalRead(_stdbyPin) == LOW);

    if (chrgLow && !stdbyLow) {
        _chargeStatus = ChargeStatus::CHARGING;
        _powerSource  = PowerSource::USB;
    }
    else if (!chrgLow && stdbyLow) {
        _chargeStatus = ChargeStatus::CHARGED;
        _powerSource  = PowerSource::USB;
    }
    else if (!chrgLow && !stdbyLow) {
        _chargeStatus = ChargeStatus::DISCHARGING;
        _powerSource  = PowerSource::BATTERY;
    }
    else {
        // Оба LOW — неопределённое состояние
        _chargeStatus = ChargeStatus::UNKNOWN;
        _powerSource  = PowerSource::USB;
    }
}

// ============================================
// Геттеры
// ============================================
float BatteryManager::getVoltage() const { return _voltage; }
int   BatteryManager::getPercent() const { return _percent; }
ChargeStatus BatteryManager::getChargeStatus() const { return _chargeStatus; }
PowerSource  BatteryManager::getPowerSource()  const { return _powerSource; }

bool BatteryManager::isCharging() const {
    return _chargeStatus == ChargeStatus::CHARGING;
}

bool BatteryManager::isUsbConnected() const {
    return _powerSource == PowerSource::USB;
}

bool BatteryManager::isLowBattery() const {
    return (!isUsbConnected()) &&
           (_voltage > 0.0f) &&
           (_voltage < BATTERY_WARN_VOLTAGE);
}

bool BatteryManager::isCriticalBattery() const {
    return (!isUsbConnected()) &&
           (_voltage > 0.0f) &&
           (_voltage < BATTERY_CRITICAL_VOLTAGE);
}

// ============================================
// Строковые представления
// ============================================
String BatteryManager::getStatusString() const {
    switch (_chargeStatus) {
        case ChargeStatus::CHARGING:    return "Charging";
        case ChargeStatus::CHARGED:     return "Fully charged";
        case ChargeStatus::DISCHARGING: return "Discharging";
        case ChargeStatus::NO_BATTERY:  return "No battery";
        case ChargeStatus::UNKNOWN:     return "Unknown";
        default:                        return "Unknown";
    }
}

String BatteryManager::getPowerSourceString() const {
    return (_powerSource == PowerSource::USB) ? "USB" : "Battery";
}

String BatteryManager::getSummaryString() const {

    String s;
    s.reserve(64);

    s += String(_voltage, 2) + "V | ";
    s += String(_percent) + "% | ";
    s += getStatusString();
    s += " (" + getPowerSourceString() + ")";

    if (isCriticalBattery())
        s += " ‼ CRITICAL";
    else if (isLowBattery())
        s += " ⚠ LOW";

    return s;
}
