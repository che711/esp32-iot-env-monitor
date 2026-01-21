#include "sensor_manager.h"
#include <Wire.h>

SensorManager::SensorManager() 
    : _temperature(0.0), _humidity(0.0),
      _minTemp(TEMP_INIT_MIN), _maxTemp(TEMP_INIT_MAX),
      _minHumid(HUMID_INIT_MIN), _maxHumid(HUMID_INIT_MAX),
      _historyIndex(0) {
    
    // Инициализация истории
    for(int i = 0; i < HISTORY_SIZE; i++) {
        _tempHistory[i] = 0;
        _humidHistory[i] = 0;
    }
}

bool SensorManager::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    delay(100);
    
    Serial.println("Инициализация AHT10...");
    if (!_aht.begin()) {
        Serial.println("ОШИБКА: AHT10 не найден!");
        return false;
    }
    Serial.println("✓ AHT10 инициализирован");
    return true;
}

void SensorManager::update() {
    sensors_event_t humid, temp;
    
    if (_aht.getEvent(&humid, &temp)) {
        _temperature = temp.temperature;
        _humidity = humid.relative_humidity;
        
        if (!isnan(_temperature) && !isnan(_humidity)) {
            // Обновление min/max
            if (_temperature < _minTemp) _minTemp = _temperature;
            if (_temperature > _maxTemp) _maxTemp = _temperature;
            if (_humidity < _minHumid) _minHumid = _humidity;
            if (_humidity > _maxHumid) _maxHumid = _humidity;
            
            // Обновление истории
            updateHistory();
            
            Serial.printf("T: %.1f°C | H: %.1f%%\n", _temperature, _humidity);
        }
    }
}

void SensorManager::updateHistory() {
    _tempHistory[_historyIndex] = _temperature;
    _humidHistory[_historyIndex] = _humidity;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
}

void SensorManager::resetMinMax() {
    _minTemp = _temperature;
    _maxTemp = _temperature;
    _minHumid = _humidity;
    _maxHumid = _humidity;
    Serial.println("Min/Max сброшены");
}

float SensorManager::getTemperature() {
    return _temperature;
}

float SensorManager::getHumidity() {
    return _humidity;
}

float SensorManager::getMinTemp() {
    return _minTemp;
}

float SensorManager::getMaxTemp() {
    return _maxTemp;
}

float SensorManager::getMinHumid() {
    return _minHumid;
}

float SensorManager::getMaxHumid() {
    return _maxHumid;
}

void SensorManager::getHistory(float* tempHist, float* humidHist, int size) {
    for(int i = 0; i < size && i < HISTORY_SIZE; i++) {
        tempHist[i] = _tempHistory[i];
        humidHist[i] = _humidHistory[i];
    }
}
