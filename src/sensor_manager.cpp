#include "sensor_manager.h"
#include "config.h"
#include <Wire.h>

SensorManager::SensorManager() 
    : _temperature(0.0), _humidity(0.0),
      _minTemp(TEMP_INIT_MIN), _maxTemp(TEMP_INIT_MAX),
      _minHumid(HUMID_INIT_MIN), _maxHumid(HUMID_INIT_MAX),
      _historyIndex(0), _hourlyHistoryIndex(0), _hourlyHistoryCount(0),
      _readErrorCount(0), _lastSuccessfulRead(0) {
    
    // Инициализация истории для графика
    for(int i = 0; i < HISTORY_SIZE; i++) {
        _tempHistory[i] = 0;
        _humidHistory[i] = 0;
    }
    
    // Выделение памяти для часовой истории
    _hourlyTempHistory = new float[HOURLY_HISTORY_SIZE]();
    _hourlyHumidHistory = new float[HOURLY_HISTORY_SIZE]();
}

SensorManager::~SensorManager() {
    delete[] _hourlyTempHistory;
    delete[] _hourlyHumidHistory;
}

bool SensorManager::begin() {
    Serial.println("Инициализация I2C...");
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    delay(100);
    
    Serial.println("Инициализация AHT10...");
    if (!_aht.begin()) {
        Serial.println("✗ ОШИБКА: AHT10 не найден!");
        Serial.println("Проверьте подключение:");
        Serial.printf("  SDA -> GPIO%d\n", I2C_SDA);
        Serial.printf("  SCL -> GPIO%d\n", I2C_SCL);
        return false;
    }
    
    Serial.println("✓ AHT10 инициализирован успешно");
    
    // Первое чтение для инициализации
    sensors_event_t humid, temp;
    if (_aht.getEvent(&humid, &temp)) {
        _temperature = temp.temperature;
        _humidity = humid.relative_humidity;
        
        if (validateReading(_temperature, _humidity)) {
            _minTemp = _maxTemp = _temperature;
            _minHumid = _maxHumid = _humidity;
            Serial.printf("Начальные значения: T=%.1f°C, H=%.1f%%\n", 
                         _temperature, _humidity);
        }
    }
    
    return true;
}

bool SensorManager::update() {
    sensors_event_t humid, temp;
    
    if (!_aht.getEvent(&humid, &temp)) {
        _readErrorCount++;
        Serial.println("✗ Ошибка чтения датчика");
        return false;
    }
    
    float newTemp = temp.temperature;
    float newHumid = humid.relative_humidity;
    
    // Валидация данных
    if (!validateReading(newTemp, newHumid)) {
        _readErrorCount++;
        Serial.printf("✗ Некорректные данные: T=%.1f°C, H=%.1f%%\n", 
                     newTemp, newHumid);
        return false;
    }
    
    // Обновление значений
    _temperature = newTemp;
    _humidity = newHumid;
    _lastSuccessfulRead = millis();
    
    // Обновление min/max
    if (_temperature < _minTemp) _minTemp = _temperature;
    if (_temperature > _maxTemp) _maxTemp = _temperature;
    if (_humidity < _minHumid) _minHumid = _humidity;
    if (_humidity > _maxHumid) _maxHumid = _humidity;
    
    // Обновление истории
    updateHistory();
    
    Serial.printf("T: %.1f°C | H: %.1f%% | Avg: T=%.1f°C H=%.1f%%\n", 
                 _temperature, _humidity, getAvgTemp(), getAvgHumid());
    
    return true;
}

void SensorManager::updateHistory() {
    // Обновление истории для графика (3 минуты)
    _tempHistory[_historyIndex] = _temperature;
    _humidHistory[_historyIndex] = _humidity;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
    
    // Обновление часовой истории
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
    Serial.println("✓ Min/Max сброшены");
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
    int count = min(size, HISTORY_SIZE);
    for(int i = 0; i < count; i++) {
        tempHist[i] = _tempHistory[i];
        humidHist[i] = _humidHistory[i];
    }
}

bool SensorManager::isValid() const {
    // Считаем валидным если последнее успешное чтение было не позже 1 минуты назад
    return (millis() - _lastSuccessfulRead) < 60000;
}

int SensorManager::getReadErrorCount() const {
    return _readErrorCount;
}
