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
    
    // Getters of current values
    float getTemperature() const;
    float getHumidity() const;
    
    // Getters min/max
    float getMinTemp() const;
    float getMaxTemp() const;
    float getMinHumid() const;
    float getMaxHumid() const;
    
    // Getters of average values
    float getAvgTemp() const;
    float getAvgHumid() const;
    
    // Graph history
    void getHistory(float* tempHist, float* humidHist, int size) const;
    int  getHistoryIndex() const;
    int  getHistoryCount() const;
    
    // Sensor
    bool isValid() const;
    int getReadErrorCount() const;
    
private:
    Adafruit_AHTX0 _aht;
    
    // Current values
    float _temperature;
    float _humidity;
    
    // Min/Max values
    float _minTemp;
    float _maxTemp;
    float _minHumid;
    float _maxHumid;
    
    // History for the graph (3 minutes)
    float _tempHistory[HISTORY_SIZE];
    float _humidHistory[HISTORY_SIZE];
    int _historyIndex;
    int _historyCount;
    
    // Running average accumulators (O(1) instead of O(n) per call)
    double _avgTempAccum;
    double _avgHumidAccum;
    int    _avgCount;
    
    // Internal state for isValid
    bool _hadFirstRead;

    // Error statistics
    int _readErrorCount;
    unsigned long _lastSuccessfulRead;
    
    // Internal methods
    void updateHistory();
    bool validateReading(float temp, float humid);
};

#endif // SENSOR_MANAGER_H
