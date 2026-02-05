#include "sensor_manager.h"
#include "config.h"
#include <Wire.h>

SensorManager::SensorManager() 
    : _temperature(0.0), _humidity(0.0),
      _minTemp(TEMP_INIT_MIN), _maxTemp(TEMP_INIT_MAX),
      _minHumid(HUMID_INIT_MIN), _maxHumid(HUMID_INIT_MAX),
      _historyIndex(0), _historyCount(0), _hourlyHistoryIndex(0), _hourlyHistoryCount(0),
      _readErrorCount(0), _lastSuccessfulRead(0) {
    
    // Initializing history for a graph
    for(int i = 0; i < HISTORY_SIZE; i++) {
        _tempHistory[i] = 0;
        _humidHistory[i] = 0;
    }
    
    // Allocating memory for an hour-long story
    _hourlyTempHistory = new float[HOURLY_HISTORY_SIZE]();
    _hourlyHumidHistory = new float[HOURLY_HISTORY_SIZE]();
}

SensorManager::~SensorManager() {
    delete[] _hourlyTempHistory;
    delete[] _hourlyHumidHistory;
}

bool SensorManager::begin() {
    Serial.println("Initialization I2C...");
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    delay(100);
    
    Serial.println("Initialization AHT10...");
    if (!_aht.begin()) {
        Serial.println("✗ ERROR: AHT10 not found!");
        Serial.println("Check connection:");
        Serial.printf("  SDA -> GPIO%d\n", I2C_SDA);
        Serial.printf("  SCL -> GPIO%d\n", I2C_SCL);
        return false;
    }
    
    Serial.println("✓ AHT10 initialized successfully");
    
    // The first read is for initialization
    sensors_event_t humid, temp;
    if (_aht.getEvent(&humid, &temp)) {
        _temperature = temp.temperature;
        _humidity = humid.relative_humidity;
        
        if (validateReading(_temperature, _humidity)) {
            _minTemp = _maxTemp = _temperature;
            _minHumid = _maxHumid = _humidity;
            Serial.printf("Initial values: T=%.1f°C, H=%.1f%%\n", 
                         _temperature, _humidity);
        }
    }
    
    return true;
}

bool SensorManager::update() {
    sensors_event_t humid, temp;
    
    if (!_aht.getEvent(&humid, &temp)) {
        _readErrorCount++;
        Serial.println("✗ Sensor reading error");
        return false;
    }
    
    float newTemp = temp.temperature;
    float newHumid = humid.relative_humidity;
    
    // Data validation
    if (!validateReading(newTemp, newHumid)) {
        _readErrorCount++;
        Serial.printf("✗ Incorrect data: T=%.1f°C, H=%.1f%%\n", 
                     newTemp, newHumid);
        return false;
    }
    
    // Updating
    _temperature = newTemp;
    _humidity = newHumid;
    _lastSuccessfulRead = millis();
    
    // Updating min/max
    if (_temperature < _minTemp) _minTemp = _temperature;
    if (_temperature > _maxTemp) _maxTemp = _temperature;
    if (_humidity < _minHumid) _minHumid = _humidity;
    if (_humidity > _maxHumid) _maxHumid = _humidity;
    
    updateHistory();
    
    Serial.printf("T: %.1f°C | H: %.1f%% | Avg: T=%.1f°C H=%.1f%%\n", 
                 _temperature, _humidity, getAvgTemp(), getAvgHumid());
    
    return true;
}

void SensorManager::updateHistory() {
    // Updating the history for the chart (3 minutes)
    _tempHistory[_historyIndex] = _temperature;
    _humidHistory[_historyIndex] = _humidity;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
    
    if (_historyCount < HISTORY_SIZE) {
        _historyCount++;
    }
    
    // Updating the hourly history
    _hourlyTempHistory[_hourlyHistoryIndex] = _temperature;
    _hourlyHumidHistory[_hourlyHistoryIndex] = _humidity;
    _hourlyHistoryIndex = (_hourlyHistoryIndex + 1) % HOURLY_HISTORY_SIZE;
    
    if (_hourlyHistoryCount < HOURLY_HISTORY_SIZE) {
        _hourlyHistoryCount++;
    }
}

bool SensorManager::validateReading(float temp, float humid) {
    if (isnan(temp) || isnan(humid)) {
        return false;
    }
    
    if (temp < TEMP_MIN_VALID || temp > TEMP_MAX_VALID) {
        return false;
    }
    
    if (humid < HUMID_MIN_VALID || humid > HUMID_MAX_VALID) {
        return false;
    }
    
    return true;
}

void SensorManager::resetMinMax() {
    _minTemp = _temperature;
    _maxTemp = _temperature;
    _minHumid = _humidity;
    _maxHumid = _humidity;
    Serial.println("✓ Min/Max have been reset");
}

float SensorManager::getTemperature() const {
    return _temperature;
}

float SensorManager::getHumidity() const {
    return _humidity;
}

float SensorManager::getMinTemp() const {
    return _minTemp;
}

float SensorManager::getMaxTemp() const {
    return _maxTemp;
}

float SensorManager::getMinHumid() const {
    return _minHumid;
}

float SensorManager::getMaxHumid() const {
    return _maxHumid;
}

float SensorManager::getAvgTemp() const {
    if (_hourlyHistoryCount == 0) return _temperature;
    
    float sum = 0;
    for(int i = 0; i < _hourlyHistoryCount; i++) {
        sum += _hourlyTempHistory[i];
    }
    return sum / _hourlyHistoryCount;
}

float SensorManager::getAvgHumid() const {
    if (_hourlyHistoryCount == 0) return _humidity;
    
    float sum = 0;
    for(int i = 0; i < _hourlyHistoryCount; i++) {
        sum += _hourlyHumidHistory[i];
    }
    return sum / _hourlyHistoryCount;
}

void SensorManager::getHistory(float* tempHist, float* humidHist, int size) const {
    int count = min(size, _historyCount);
    
    // If the buffer is not full yet, the data simply lies from 0 to _historyCount.
    if (_historyCount < HISTORY_SIZE) {
        for (int i = 0; i < count; i++) {
            tempHist[i]  = _tempHistory[i];
            humidHist[i] = _humidHistory[i];
        }
    } else {
        // The buffer is full and has already rotated — the oldest element is on _historyIndex
        for (int i = 0; i < count; i++) {
            int idx = (_historyIndex + i) % HISTORY_SIZE;
            tempHist[i]  = _tempHistory[idx];
            humidHist[i] = _humidHistory[idx];
        }
    }
}

int SensorManager::getHistoryIndex() const {
    return _historyIndex;
}

int SensorManager::getHistoryCount() const {
    return _historyCount;
}

bool SensorManager::isValid() const {
    // We consider it valid if the last successful reading was no later than 1 minute ago.
    return (millis() - _lastSuccessfulRead) < 60000;
}

int SensorManager::getReadErrorCount() const {
    return _readErrorCount;
}
