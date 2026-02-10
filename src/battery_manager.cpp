#include "battery_manager.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>

BatteryManager::BatteryManager()
    : _voltage(0.0),
      _percentage(0.0),
      _status(BatteryStatus::UNKNOWN),
      _powerSource(PowerSource::UNKNOWN),
      _isCharging(false),
      _isCharged(false),
      _usbConnected(false),
      _lastUpdate(0),
      _readCount(0) {
}

bool BatteryManager::begin() {
    Serial.println("=== Battery Manager Initialization ===");
    
    // Configure ADC for battery voltage reading
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    
    // Configure GPIO pins for TP4056 status
    pinMode(BATTERY_CHRG_PIN, INPUT_PULLUP);  // CHRG pin (LOW when charging)
    pinMode(BATTERY_STDBY_PIN, INPUT_PULLUP); // STDBY pin (LOW when charged)
    
    if (BATTERY_USB_DETECT_PIN >= 0) {
        pinMode(BATTERY_USB_DETECT_PIN, INPUT);
    }
    
    delay(100);
    
    // Initial reading
    update();
    
    Serial.printf("✓ Battery Manager initialized\n");
    Serial.printf("  Initial voltage: %.2fV (%.0f%%)\n", _voltage, _percentage);
    Serial.printf("  Power source: %s\n", 
                 _powerSource == PowerSource::USB_CHARGING ? "USB" : "Battery");
    Serial.printf("  Status: %s\n", 
                 _isCharging ? "Charging" : 
                 _isCharged ? "Charged" : "Discharging");
    
    return true;
}

void BatteryManager::update() {
    _voltage = readVoltage();
    _percentage = calculatePercentage(_voltage);
    updatePowerSource();
    updateStatus();
    
    _lastUpdate = millis();
    _readCount++;
}

float BatteryManager::readVoltage() {
    // Read ADC value
    int adcValue = analogRead(BATTERY_ADC_PIN);
    
    // Average multiple readings for stability
    const int samples = 10;
    long sum = adcValue;
    
    for (int i = 1; i < samples; i++) {
        delay(10);
        sum += analogRead(BATTERY_ADC_PIN);
    }
    
    adcValue = sum / samples;
    
    // Convert ADC to voltage
    // ESP32-C3 ADC: 12-bit (0-4095), with 11db attenuation (0-3.3V)
    float adcVoltage = (adcValue / 4095.0) * 3.3;
    
    // Calculate actual battery voltage using voltage divider
    // TP4056 outputs battery voltage, typically divided by 2 (e.g., 100K + 100K resistors)
    float batteryVoltage = adcVoltage * BATTERY_VOLTAGE_DIVIDER;
    
    return batteryVoltage;
}

float BatteryManager::calculatePercentage(float voltage) {
    // Li-ion battery voltage curve
    // 4.2V = 100%, 3.7V = 50%, 3.0V = 0%
    
    if (voltage >= BATTERY_VOLTAGE_FULL) {
        return 100.0;
    }
    
    if (voltage <= BATTERY_VOLTAGE_EMPTY) {
        return 0.0;
    }
    
    // Linear approximation (можно использовать lookup table для точности)
    float range = BATTERY_VOLTAGE_FULL - BATTERY_VOLTAGE_EMPTY;
    float current = voltage - BATTERY_VOLTAGE_EMPTY;
    float percentage = (current / range) * 100.0;
    
    // Clamp to 0-100%
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;
    
    return percentage;
}

void BatteryManager::updatePowerSource() {
    // Check USB detection pin if available
    if (BATTERY_USB_DETECT_PIN >= 0) {
        _usbConnected = digitalRead(BATTERY_USB_DETECT_PIN) == HIGH;
    } else {
        // Fallback: detect USB by charging status or voltage
        _usbConnected = _isCharging || _isCharged || (_voltage > 4.1);
    }
    
    _powerSource = _usbConnected ? PowerSource::USB_CHARGING : PowerSource::BATTERY;
}

void BatteryManager::updateStatus() {
    // Read TP4056 status pins
    // CHRG: LOW when charging, HIGH when not charging
    // STDBY: LOW when charged (standby), HIGH otherwise
    
    bool chrgPin = digitalRead(BATTERY_CHRG_PIN);
    bool stdbyPin = digitalRead(BATTERY_STDBY_PIN);
    
    _isCharging = (chrgPin == LOW);  // Active LOW
    _isCharged = (stdbyPin == LOW);  // Active LOW
    
    // Determine battery status
    if (_isCharging) {
        _status = BatteryStatus::CHARGING;
    } else if (_isCharged) {
        _status = BatteryStatus::CHARGED;
    } else if (_percentage < BATTERY_CRITICAL_PERCENT) {
        _status = BatteryStatus::CRITICAL;
    } else if (_percentage < BATTERY_LOW_PERCENT) {
        _status = BatteryStatus::BATTERY_LOW;  // Updated name
    } else {
        _status = BatteryStatus::DISCHARGING;
    }
}

float BatteryManager::getVoltage() const {
    return _voltage;
}

float BatteryManager::getPercentage() const {
    return _percentage;
}

BatteryStatus BatteryManager::getStatus() const {
    return _status;
}

PowerSource BatteryManager::getPowerSource() const {
    return _powerSource;
}

bool BatteryManager::isCharging() const {
    return _isCharging;
}

bool BatteryManager::isCharged() const {
    return _isCharged;
}

bool BatteryManager::isUSBConnected() const {
    return _usbConnected;
}

bool BatteryManager::isBatteryLow() const {
    return _percentage < BATTERY_LOW_PERCENT;
}

bool BatteryManager::isBatteryCritical() const {
    return _percentage < BATTERY_CRITICAL_PERCENT;
}

unsigned long BatteryManager::getLastUpdateTime() const {
    return _lastUpdate;
}

int BatteryManager::getReadCount() const {
    return _readCount;
}
