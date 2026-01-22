#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_AHTX0.h>
#include "config.h"

class SensorManager {
public:
    SensorManager();
    ~SensorManager();  // Добавляем деструктор
    
    bool begin();
    void update();
    void resetMinMax();
    
    float getTemperature();
    float getHumidity();
    float getMinTemp();
    float getMaxTemp();
    float getMinHumid();
    float getMaxHumid();
    float getAvgTemp();      // Новый метод
    float getAvgHumid();     // Новый метод
    
    void getHistory(float* tempHist, float* humidHist, int size);
    
private:
    Adafruit_AHTX0 _aht;
    
    float _temperature;
    float _humidity;
    float _minTemp;
    float _maxTemp;
    float _minHumid;
    float _maxHumid;
    
    float _tempHistory[HISTORY_SIZE];
    float _humidHistory[HISTORY_SIZE];
    int _historyIndex;
    
    // История на час для расчёта средних (используем динамическую память)
    float* _hourlyTempHistory;
    float* _hourlyHumidHistory;
    int _hourlyHistoryIndex;
    int _hourlyHistoryCount;  // Счётчик заполненных элементов
    
    void updateHistory();
};

#endif // SENSOR_MANAGER_H
