#ifndef CALCULATIONS_H
#define CALCULATIONS_H

class WeatherCalculations {
public:
    // Calculate Dew Point
    static float calculateDewPoint(float temp, float humid);
    
    // Calculate Heat Index
    static float calculateHeatIndex(float temp, float humid);
};

#endif // CALCULATIONS_H
