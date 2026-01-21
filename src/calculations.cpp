#include "calculations.h"
#include <math.h>

float WeatherCalculations::calculateDewPoint(float temp, float humid) {
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * temp) / (b + temp)) + log(humid / 100.0);
    return (b * alpha) / (a - alpha);
}

float WeatherCalculations::calculateHeatIndex(float temp, float humid) {
    // Heat Index применяется только при температуре выше 27°C
    if (temp < 27) return temp;
    
    // Коэффициенты для формулы Rothfusz
    float c1 = -8.78469475556;
    float c2 = 1.61139411;
    float c3 = 2.33854883889;
    float c4 = -0.14611605;
    float c5 = -0.012308094;
    float c6 = -0.0164248277778;
    float c7 = 0.002211732;
    float c8 = 0.00072546;
    float c9 = -0.000003582;
    
    return c1 + (c2 * temp) + (c3 * humid) + (c4 * temp * humid) + 
           (c5 * temp * temp) + (c6 * humid * humid) + 
           (c7 * temp * temp * humid) + (c8 * temp * humid * humid) + 
           (c9 * temp * temp * humid * humid);
}
