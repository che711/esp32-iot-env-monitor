#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include "config.h"

enum class PowerSource {
    BATTERY,
    USB_CHARGING,
    UNKNOWN
};

enum class BatteryStatus {
    CHARGING,
    CHARGED,
    DISCHARGING,
    BATTERY_LOW,      // Renamed to avoid conflict with Arduino LOW macro
    CRITICAL,
    UNKNOWN
};

class BatteryManager {
public:
    BatteryManager();
    
    bool begin();
    void update();
    
    // Battery status
    float getVoltage() const;
    float getPercentage() const;
    BatteryStatus getStatus() const;
    PowerSource getPowerSource() const;
    
    // Charging status (from TP4056)
    bool isCharging() const;
    bool isCharged() const;
    bool isUSBConnected() const;
    
    // Battery health
    bool isBatteryLow() const;
    bool isBatteryCritical() const;
    
    // Statistics
    unsigned long getLastUpdateTime() const;
    int getReadCount() const;
    
private:
    float _voltage;
    float _percentage;
    BatteryStatus _status;
    PowerSource _powerSource;
    
    bool _isCharging;
    bool _isCharged;
    bool _usbConnected;
    
    unsigned long _lastUpdate;
    int _readCount;
    
    // Internal methods
    float readVoltage();
    float calculatePercentage(float voltage);
    void updateStatus();
    void updatePowerSource();
};

#endif // BATTERY_MANAGER_H
