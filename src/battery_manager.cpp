#include "battery_manager.h"
#include "config.h"
#include <algorithm>  // std::sort

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

static const float ADC_MAX_VALUE = 4095.0f;  // 12-bit

// ============================================
// Таблица компенсации нелинейности ADC ESP32
// (vAdc_raw → коэффициент коррекции)
//
// ADC ESP32-C3 при 11dB аттенюации нелинеен
// в верхней части диапазона. Эта таблица
// компенсирует систематическую ошибку
// без замены оборудования.
//
// Калибровка: измерьте мультиметром реальное
// напряжение на ADC-пине и сравните с vAdc.
// Если vAdc стабильно ниже — увеличьте
// коэффициент для этого диапазона.
// ============================================
static const float ADC_NONLIN_TABLE[][2] = {
    // { vAdc_raw,  correction_factor }
    {0.00f, 1.000f},
    {0.50f, 1.000f},
    {1.00f, 1.005f},
    {1.50f, 1.012f},
    {1.80f, 1.020f},  // ~3.6V bat — начало нелинейности
    {2.00f, 1.030f},  // ~4.0V bat — основная погрешность
    {2.10f, 1.040f},  // ~4.2V bat — максимальная погрешность
    {2.50f, 1.050f},
};
static const int NONLIN_POINTS =
    sizeof(ADC_NONLIN_TABLE) / sizeof(ADC_NONLIN_TABLE[0]);

// Коэффициент EMA-фильтра (0..1)
// 0.25 = плавное сглаживание ~4 замера для выхода на 63%
// При BATTERY_CHECK_INTERVAL=30с и изменении батареи ~0.001V/мин — этого достаточно
static constexpr float EMA_ALPHA = 0.25f;

// Максимальный "реалистичный" прыжок напряжения за один цикл (В)
// Батарея не может измениться на 1V за 30 секунд — это точно помеха
static constexpr float MAX_REALISTIC_DELTA = 1.0f;

// ============================================
// Конструктор
// ============================================
BatteryManager::BatteryManager(int adcPin, int chrgPin, int stdbyPin)
    : _adcPin(adcPin),
      _chrgPin(chrgPin),
      _stdbyPin(stdbyPin),
      _voltage(0.0f),
      _emaVoltage(0.0f),
      _percent(0),
      _chargeStatus(ChargeStatus::DISCHARGING),
      _powerSource(PowerSource::BATTERY),
      _initialized(false)
{}

// ============================================
// Инициализация
// ============================================
void BatteryManager::begin() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);  // Явно устанавливаем аттенюацию

    pinMode(_chrgPin, INPUT_PULLUP);
    pinMode(_stdbyPin, INPUT_PULLUP);

    // Первый замер: делаем 3 независимых чтения и берём медиану.
    // Это надёжнее чем единственный замер при старте, когда ADC
    // ещё не успел стабилизироваться.
    delay(10);
    float v1 = readVoltageRaw();
    delay(5);
    float v2 = readVoltageRaw();
    delay(5);
    float v3 = readVoltageRaw();

    // Медиана из трёх
    float initVoltage;
    if ((v1 <= v2 && v2 <= v3) || (v3 <= v2 && v2 <= v1))
        initVoltage = v2;
    else if ((v2 <= v1 && v1 <= v3) || (v3 <= v1 && v1 <= v2))
        initVoltage = v1;
    else
        initVoltage = v3;

    _voltage    = initVoltage;
    _emaVoltage = initVoltage;  // Seed EMA с реальным значением
    _initialized = true;

    _percent = voltageToPercent(_voltage);
    readChargeStatus();

    Serial.println("✓ BatteryManager initialized");
    Serial.println("  " + getSummaryString());
    Serial.printf("  ADC GPIO%d | %d samples | ref %.2fV | divider %.2f | correction %.3f\n",
                  _adcPin, BATTERY_ADC_SAMPLES,
                  BATTERY_ADC_REF_VOLTAGE, BATTERY_DIVIDER_RATIO, BATTERY_ADC_CORRECTION);
    Serial.printf("  Init median: v1=%.3f v2=%.3f v3=%.3f → %.3fV\n",
                  v1, v2, v3, initVoltage);
    if (_voltage > 4.5f)
        Serial.println("  ⚠️ Voltage > 4.5V — check BATTERY_DIVIDER_RATIO/BATTERY_ADC_CORRECTION");
    Serial.println();
}

// ============================================
// Обновление
// ============================================
void BatteryManager::update() {
    float newVoltage = readVoltageRaw();

    if (!_initialized) {
        // На случай если update() вызван до begin() — инициализируем напрямую
        _voltage     = newVoltage;
        _emaVoltage  = newVoltage;
        _initialized = true;
    } else {
        // ── Шаг 1: Отклоняем физически невозможные значения ──────────────
        // Батарея не может измениться более чем на MAX_REALISTIC_DELTA
        // за один цикл проверки. Если разница больше — это помеха ADC.
        if (fabs(newVoltage - _emaVoltage) > MAX_REALISTIC_DELTA) {
            Serial.printf("[BAT] Spike rejected: new=%.3fV ema=%.3fV (delta=%.3fV)\n",
                          newVoltage, _emaVoltage,
                          fabs(newVoltage - _emaVoltage));
            // Не обновляем EMA, оставляем старое значение
            newVoltage = _emaVoltage;
        }

        // ── Шаг 2: EMA-фильтр ────────────────────────────────────────────
        // Плавно движемся к новому значению, не залипая на первом замере.
        // Формула: ema = alpha * new + (1-alpha) * old
        _emaVoltage = EMA_ALPHA * newVoltage + (1.0f - EMA_ALPHA) * _emaVoltage;
        _voltage    = _emaVoltage;
    }

    _percent = voltageToPercent(_voltage);
    readChargeStatus();
}

// ============================================
// Компенсация нелинейности ADC
// Интерполяция по таблице ADC_NONLIN_TABLE
// ============================================
float BatteryManager::adcNonlinCorrection(float vAdc) const {
    if (vAdc <= ADC_NONLIN_TABLE[0][0])
        return ADC_NONLIN_TABLE[0][1];

    for (int i = 0; i < NONLIN_POINTS - 1; i++) {
        float vLow  = ADC_NONLIN_TABLE[i][0];
        float vHigh = ADC_NONLIN_TABLE[i + 1][0];
        if (vAdc >= vLow && vAdc < vHigh) {
            float t = (vAdc - vLow) / (vHigh - vLow);
            return ADC_NONLIN_TABLE[i][1] +
                   t * (ADC_NONLIN_TABLE[i + 1][1] - ADC_NONLIN_TABLE[i][1]);
        }
    }

    return ADC_NONLIN_TABLE[NONLIN_POINTS - 1][1];
}

// ============================================
// Чтение сырого напряжения
// Улучшено: отбрасываем выбросы (25% снизу и сверху)
// перед усреднением
// ============================================
float BatteryManager::readVoltageRaw() {
    const int N = BATTERY_ADC_SAMPLES;  // должно быть >= 8
    int samples[N];

    for (int i = 0; i < N; i++) {
        samples[i] = analogRead(_adcPin);
        delayMicroseconds(200);  // Чуть больше паузы для стабилизации зарядки входа
    }

    // Сортируем и отбрасываем нижние и верхние 25%
    std::sort(samples, samples + N);
    int trimLow  = N / 4;       // отбрасываем нижний квартиль
    int trimHigh = N - N / 4;   // отбрасываем верхний квартиль

    long sum = 0;
    for (int i = trimLow; i < trimHigh; i++) {
        sum += samples[i];
    }
    int validCount = trimHigh - trimLow;
    float raw = (float)sum / validCount;

    // ADC → напряжение на пине
    float vAdc = (raw / ADC_MAX_VALUE) * BATTERY_ADC_REF_VOLTAGE;

    // Компенсация нелинейности ADC
    float nonlinFactor = adcNonlinCorrection(vAdc);
    vAdc *= nonlinFactor;

    // Делитель напряжения + калибровочный коэффициент
    float vBat = vAdc * BATTERY_DIVIDER_RATIO * BATTERY_ADC_CORRECTION;

    // Отладочный лог раз в минуту
    static unsigned long lastDebug = 0;
    unsigned long now = millis();
    if (now - lastDebug >= 60000) {
        lastDebug = now;
        Serial.printf("[BAT] raw:%.0f(trim) gpio:%.3fV nonlin:%.3f bat:%.3fV ema:%.3fV\n",
                      raw, vAdc / nonlinFactor, nonlinFactor, vBat, _emaVoltage);
    }

    if (vBat < 0.8f) return 0.0f;
    if (vBat > 4.5f) vBat = 4.5f;

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

            float t       = (v - vLow) / range;
            float percent = pLow + t * (pHigh - pLow);

            if (percent < 0)   percent = 0;
            if (percent > 100) percent = 100;

            return (int)(percent + 0.5f);
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
float        BatteryManager::getVoltage()      const { return _voltage; }
int          BatteryManager::getPercent()      const { return _percent; }
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
