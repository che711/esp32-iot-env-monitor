// Юнит-тесты WeatherCalculations — запускаются на хосте: pio test -e native
// Эталоны взяты из таблиц NWS (heat index) и стандартной формулы Магнуса
// (точка росы), а не пересчитаны из тестируемого кода.
#include <math.h>
#include <unity.h>

#include "calculations.h"

void setUp() {}
void tearDown() {}

// ---------- Dew point ----------

void test_dew_point_reference_values() {
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 16.7f, WeatherCalculations::calculateDewPoint(25.0f, 60.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f,  9.3f, WeatherCalculations::calculateDewPoint(20.0f, 50.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -3.0f, WeatherCalculations::calculateDewPoint( 0.0f, 80.0f));
}

void test_dew_point_equals_temperature_at_saturation() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, WeatherCalculations::calculateDewPoint(25.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -5.0f, WeatherCalculations::calculateDewPoint(-5.0f, 100.0f));
}

void test_dew_point_not_above_temperature() {
    for (float rh = 5.0f; rh <= 100.0f; rh += 5.0f) {
        TEST_ASSERT_TRUE(WeatherCalculations::calculateDewPoint(22.0f, rh) <= 22.0f + 0.01f);
    }
}

void test_dew_point_zero_humidity_is_finite() {
    // log(0) не должен протечь в JSON как -inf/nan
    float dp = WeatherCalculations::calculateDewPoint(25.0f, 0.0f);
    TEST_ASSERT_FALSE(isnan(dp));
    TEST_ASSERT_FALSE(isinf(dp));
    TEST_ASSERT_TRUE(dp < 0.0f);
}

void test_dew_point_humidity_above_100_is_clamped() {
    TEST_ASSERT_EQUAL_FLOAT(WeatherCalculations::calculateDewPoint(25.0f, 100.0f),
                            WeatherCalculations::calculateDewPoint(25.0f, 120.0f));
}

// ---------- Heat index ----------

void test_heat_index_below_threshold_returns_temperature() {
    TEST_ASSERT_EQUAL_FLOAT(20.0f, WeatherCalculations::calculateHeatIndex(20.0f, 90.0f));
    TEST_ASSERT_EQUAL_FLOAT(26.9f, WeatherCalculations::calculateHeatIndex(26.9f, 50.0f));
}

void test_heat_index_reference_values() {
    // NWS: 86°F/70% → 95°F, 90°F/50% → 95°F, 95°F/80% → ~133°F
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 35.0f, WeatherCalculations::calculateHeatIndex(30.0f, 70.0f));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 35.0f, WeatherCalculations::calculateHeatIndex(32.0f, 50.0f));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 56.1f, WeatherCalculations::calculateHeatIndex(35.0f, 80.0f));
}

void test_heat_index_grows_with_humidity() {
    float prev = WeatherCalculations::calculateHeatIndex(32.0f, 40.0f);
    for (float rh = 45.0f; rh <= 90.0f; rh += 5.0f) {
        float hi = WeatherCalculations::calculateHeatIndex(32.0f, rh);
        TEST_ASSERT_TRUE(hi > prev);
        prev = hi;
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_dew_point_reference_values);
    RUN_TEST(test_dew_point_equals_temperature_at_saturation);
    RUN_TEST(test_dew_point_not_above_temperature);
    RUN_TEST(test_dew_point_zero_humidity_is_finite);
    RUN_TEST(test_dew_point_humidity_above_100_is_clamped);
    RUN_TEST(test_heat_index_below_threshold_returns_temperature);
    RUN_TEST(test_heat_index_reference_values);
    RUN_TEST(test_heat_index_grows_with_humidity);
    return UNITY_END();
}
