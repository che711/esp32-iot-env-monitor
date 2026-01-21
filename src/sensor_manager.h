#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_AHTX0.h>
#include "config.h"

class SensorManager {
public:
    SensorManager();
    
    bool begin();
    void update();
    void resetMinMax();
    
    float getTemperature();
    float getHumidity();
    float getMinTemp();
    float getMaxTemp();
    float getMinHumid();
    float getMaxHumid();
    
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
    
    void updateHistory();
};

#endif // SENSOR_MANAGER_H
