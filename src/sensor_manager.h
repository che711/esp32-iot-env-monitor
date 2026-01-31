#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_AHTX0.h>
#include "config.h"

class SensorManager {
public:
    SensorManager();
    ~SensorManager();
    
    bool begin();
    bool update();
    void resetMinMax();
    
    // Геттеры текущих значений
    float getTemperature() const;
    float getHumidity() const;
    
    // Геттеры min/max
    float getMinTemp() const;
    float getMaxTemp() const;
    float getMinHumid() const;
    float getMaxHumid() const;
    
    // Геттеры средних значений
    float getAvgTemp() const;
    float getAvgHumid() const;
    
    // История для графиков
    void getHistory(float* tempHist, float* humidHist, int size) const;
    
    // Статус датчика
    bool isValid() const;
    int getReadErrorCount() const;
    
private:
    Adafruit_AHTX0 _aht;
    
    // Текущие значения
    float _temperature;
    float _humidity;
    
    // Min/Max значения
    float _minTemp;
    float _maxTemp;
    float _minHumid;
    float _maxHumid;
    
    // История для графика (3 минуты)
    float _tempHistory[HISTORY_SIZE];
    float _humidHistory[HISTORY_SIZE];
    int _historyIndex;
    
    // Часовая история для средних значений
    float* _hourlyTempHistory;
    float* _hourlyHumidHistory;
    int _hourlyHistoryIndex;
    int _hourlyHistoryCount;
    
    // Статистика ошибок
    int _readErrorCount;
    unsigned long _lastSuccessfulRead;
    
    // Внутренние методы
    void updateHistory();
    bool validateReading(float temp, float humid);
};

#endif // SENSOR_MANAGER_H
