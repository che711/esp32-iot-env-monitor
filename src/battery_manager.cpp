#include "battery_manager.h"
#include "config.h"

// ============================================
// Таблица разряда Li-Ion 18650 (напряжение → %)
// ============================================
static const float DISCHARGE_CURVE[][2] = {
    {4.20f, 100},
    {4.10f,  95},
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
    
    // ═══════════════════════════════════════════════════════════
    // ОТЛАДКА: Вывод параметров конфигурации
    // ═══════════════════════════════════════════════════════════
    Serial.println("\n╔════════════════════════════════════════════╗");
    Serial.println("║  Battery ADC Configuration & Diagnostics   ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.printf("ADC Pin:           GPIO%d\n", _adcPin);
    Serial.printf("ADC Samples:       %d\n", BATTERY_ADC_SAMPLES);
    Serial.printf("ADC Ref Voltage:   %.2fV\n", BATTERY_ADC_REF_VOLTAGE);
    Serial.printf("Divider Ratio:     %.2f\n", BATTERY_DIVIDER_RATIO);
    Serial.printf("Correction:        %.2f\n", BATTERY_ADC_CORRECTION);
    Serial.println("─────────────────────────────────────────────");
    
    // Одиночное измерение для отладки
    long sum = 0;
    for (int i = 0; i < BATTERY_ADC_SAMPLES; i++) {
        sum += analogRead(_adcPin);
        delayMicroseconds(100);
    }
    float raw = (float)sum / BATTERY_ADC_SAMPLES;
    float vAdc = (raw / ADC_MAX_VALUE) * BATTERY_ADC_REF_VOLTAGE;
    float vBat_calc = vAdc * BATTERY_DIVIDER_RATIO * BATTERY_ADC_CORRECTION;
    
    Serial.printf("ADC Raw Value:     %.0f / 4095\n", raw);
    Serial.printf("Voltage on GPIO:   %.3fV\n", vAdc);
    Serial.printf("Calculated vBat:   %.3fV\n", vBat_calc);
    Serial.printf("Reported vBat:     %.2fV\n", _voltage);
    
    if (vBat_calc > 4.5f) {
        Serial.println("\n⚠️  WARNING: Calculated voltage > 4.5V!");
        Serial.println("   This indicates calibration problem.");
        Serial.printf("   Voltage capped at 4.5V (real: %.2fV)\n", vBat_calc);
    }
    
    // Расчет ожидаемого коэффициента
    if (vAdc > 0.1f) {
        float expected_ratio = _voltage / (vAdc * BATTERY_DIVIDER_RATIO);
        Serial.printf("\nExpected correction: %.2f (current: %.2f)\n", 
                     expected_ratio, BATTERY_ADC_CORRECTION);
        
        if (fabs(expected_ratio - BATTERY_ADC_CORRECTION) > 0.1f) {
            Serial.println("⚠️  Correction mismatch detected!");
        }
    }
    
    Serial.println("╚════════════════════════════════════════════╝\n");
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

    // ═══ ОТЛАДКА ═══
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 60000) {  // Каждую минуту
        lastDebug = millis();
        Serial.printf("[BAT DEBUG] Raw:%.0f vGPIO:%.3fV Calc:%.3fV ", 
                     raw, vAdc, vBat);
    }
    // ═══════════════

    // Защита от мусора
    if (vBat < 0.8f) {
        Serial.println("Final:0.00V (too low)");
        return 0.0f;
    }
    
    if (vBat > 4.5f) {
        // ═══ ОТЛАДКА ═══
        if (millis() - lastDebug < 100) {
            Serial.printf("Final:4.50V (capped from %.2fV)\n", vBat);
        }
        // ═══════════════
        vBat = 4.5f;
    } else {
        // ═══ ОТЛАДКА ═══
        if (millis() - lastDebug < 100) {
            Serial.printf("Final:%.2fV\n", vBat);
        }
        // ═══════════════
    }

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
