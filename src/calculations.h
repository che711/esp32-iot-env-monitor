#ifndef CALCULATIONS_H
#define CALCULATIONS_H

class WeatherCalculations {
public:
    // Вычисление точки росы (Dew Point)
    static float calculateDewPoint(float temp, float humid);
    
    // Вычисление индекса теплоощущения (Heat Index)
    static float calculateHeatIndex(float temp, float humid);
};

#endif // CALCULATIONS_H
