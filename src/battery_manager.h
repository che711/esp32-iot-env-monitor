#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

// ============================================
// Статус зарядки TP4056
// ============================================
enum class ChargeStatus {
    UNKNOWN,         //
    CHARGING,        // CHRG = LOW  — идёт зарядка
    CHARGED,         // STDBY = LOW — заряжен полностью
    DISCHARGING,     // Оба HIGH    — работа от батареи
    NO_BATTERY       // Оба LOW     — нет батареи (не подключена)
};

// ============================================
// Источник питания
// ============================================
enum class PowerSource {
    USB,             // Питание от USB (зарядка или полный заряд)
    BATTERY          // Питание от батареи
};

// ============================================
// Класс управления батареей
// ============================================
class BatteryManager {
public:
    BatteryManager(int adcPin, int chrgPin, int stdbyPin);

    void begin();
    void update();  // Вызывать периодически для обновления данных

    // Геттеры
    float        getVoltage()      const;  // Напряжение батареи, В (EMA-сглаженное)
    int          getPercent()      const;  // Уровень заряда, %
    ChargeStatus getChargeStatus() const;
    PowerSource  getPowerSource()  const;

    // Удобные предикаты
    bool isCharging()      const;
    bool isUsbConnected()  const;       // USB подключён (зарядка или полный заряд)
    bool isLowBattery()    const;       // Ниже порога предупреждения
    bool isCriticalBattery() const;     // Критически мало — нужно срочно спать

    // Строковые представления для логов
    String getStatusString()     const;
    String getPowerSourceString() const;
    String getSummaryString()    const;  // "3.85V | 72% | Charging"

private:
    int _adcPin;
    int _chrgPin;
    int _stdbyPin;

    float        _voltage;       // Текущее (EMA-сглаженное) напряжение
    float        _emaVoltage;    // Внутренний аккумулятор EMA
    int          _percent;
    ChargeStatus _chargeStatus;
    PowerSource  _powerSource;
    bool         _initialized;   // true после первого корректного замера

    // Внутренние методы
    float readVoltageRaw();               // Замер с отбрасыванием выбросов
    float adcNonlinCorrection(float vAdc) const; // Компенсация нелинейности ADC
    int   voltageToPercent(float v);
    void  readChargeStatus();
};

#endif // BATTERY_MANAGER_H
