"""
ESP32 Weather Station API Tests
Python-based API testing with pytest

На плате:        ESP32_IP=192.168.1.65 pytest api
Без платы (CI):  pytest api --mock

⚠️ TestResetEndpoint вызывает /reset — min/max на плате будут сброшены.
"""

import concurrent.futures
import os
import re
import time

import pytest
import requests

pytestmark = pytest.mark.api

TIMEOUT = int(os.getenv("TEST_TIMEOUT", "5"))
HISTORY_SIZE = 60  # HISTORY_SIZE в config.h

# ═══════════════════════════════════════════════════════════════
# Fixtures
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session")
def session():
    """Requests session with default timeout"""
    s = requests.Session()
    s.request = lambda method, url, **kwargs: requests.Session.request(
        s, method, url, **{"timeout": TIMEOUT, **kwargs}
    )
    yield s
    s.close()

@pytest.fixture(scope="module", autouse=True)
def check_connectivity(session, base_url):
    """Verify ESP32 is reachable before running tests"""
    try:
        response = session.get(base_url)
        assert response.status_code == 200
        print(f"\n✓ ESP32 is reachable at {base_url}")
    except requests.exceptions.RequestException as e:
        pytest.fail(f"Cannot connect to ESP32 at {base_url}: {e}")

@pytest.fixture
def data(session, base_url):
    return session.get(f"{base_url}/data").json()

@pytest.fixture
def stats(session, base_url):
    return session.get(f"{base_url}/stats").json()

@pytest.fixture
def history(session, base_url):
    return session.get(f"{base_url}/history").json()

# ═══════════════════════════════════════════════════════════════
# Root Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestRootEndpoint:
    """Tests for root endpoint (/)"""

    def test_root_returns_html(self, session, base_url):
        """GET / should return HTML page"""
        response = session.get(f"{base_url}/")

        assert response.status_code == 200
        assert "text/html" in response.headers.get("Content-Type", "")
        assert "<!DOCTYPE html>" in response.text
        assert "<html" in response.text

    def test_root_contains_title(self, session, base_url):
        """HTML should contain page title"""
        response = session.get(f"{base_url}/")

        assert "<title>ENV Station</title>" in response.text

    def test_root_contains_sensor_elements(self, session, base_url):
        """HTML should contain sensor data elements"""
        response = session.get(f"{base_url}/")

        for element_id in ("temperature", "humidity", "dewPoint", "heatIndex", "fwVersion"):
            assert f'id="{element_id}"' in response.text, f"Missing element #{element_id}"

    def test_root_contains_javascript(self, session, base_url):
        """HTML should contain JavaScript that polls the API"""
        response = session.get(f"{base_url}/")

        assert "<script" in response.text
        for endpoint in ("/data", "/stats", "/history"):
            assert f"fetch('{endpoint}')" in response.text

# ═══════════════════════════════════════════════════════════════
# Data Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestDataEndpoint:
    """Tests for /data endpoint"""

    def test_data_endpoint_accessible(self, session, base_url):
        """GET /data should be accessible and never cached"""
        response = session.get(f"{base_url}/data")

        assert response.status_code == 200
        assert "application/json" in response.headers.get("Content-Type", "")
        assert "no-cache" in response.headers.get("Cache-Control", "")

    def test_data_returns_valid_json(self, session, base_url):
        """GET /data should return valid JSON"""
        response = session.get(f"{base_url}/data")

        try:
            assert isinstance(response.json(), dict)
        except requests.exceptions.JSONDecodeError:
            pytest.fail("Response is not valid JSON")

    def test_data_required_fields(self, data):
        """GET /data should contain all required fields"""
        required_fields = [
            "temperature", "humidity",
            "minTemp", "maxTemp",
            "minHumid", "maxHumid",
            "avgTemp", "avgHumid",
            "dewPoint", "heatIndex",
            "timestamp"
        ]

        for field in required_fields:
            assert field in data, f"Missing field: {field}"

    def test_temperature_range(self, data):
        """Temperature should be in AHT10 range"""
        temp = data["temperature"]
        assert -40 <= temp <= 85, f"Temperature out of range: {temp}"

    def test_humidity_range(self, data):
        """Humidity should be in valid range"""
        humid = data["humidity"]
        assert 0 <= humid <= 100, f"Humidity out of range: {humid}"

    def test_minmax_consistency(self, data):
        """Min should be <= current <= Max"""
        assert data["minTemp"] <= data["temperature"] <= data["maxTemp"]
        assert data["minHumid"] <= data["humidity"] <= data["maxHumid"]

    def test_dew_point_not_above_temperature(self, data):
        """Dew point can't exceed air temperature"""
        assert data["dewPoint"] <= data["temperature"] + 0.01

    def test_timestamp_is_integer(self, data):
        """Timestamp (millis) should be a valid integer"""
        assert isinstance(data["timestamp"], int)
        assert data["timestamp"] > 0

# ═══════════════════════════════════════════════════════════════
# Stats Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestStatsEndpoint:
    """Tests for /stats endpoint"""

    def test_stats_endpoint_accessible(self, session, base_url):
        """GET /stats should be accessible"""
        response = session.get(f"{base_url}/stats")

        assert response.status_code == 200
        assert "application/json" in response.headers.get("Content-Type", "")

    def test_stats_required_fields(self, stats):
        """GET /stats should contain system information"""
        required_fields = [
            "uptime", "firmware", "buildTime", "millisOverflows",
            "freeHeap", "freeHeapRaw", "totalHeapRaw", "heapUsagePct", "heapUsage",
            "cpuUsage", "ssid", "rssi", "ip", "requests", "errors", "battery"
        ]

        for field in required_fields:
            assert field in stats, f"Missing field: {field}"

    def test_firmware_version_format(self, stats):
        """Firmware version looks like 3.1 / 3.1.2"""
        assert re.fullmatch(r"\d+\.\d+(\.\d+)?", stats["firmware"]), stats["firmware"]

    def test_build_time_format(self, stats):
        """Build time is 'YYYY-MM-DD HH:MM' (scripts/build_time.py)"""
        assert re.fullmatch(r"\d{4}-\d{2}-\d{2} \d{2}:\d{2}", stats["buildTime"]), stats["buildTime"]

    def test_uptime_format(self, stats):
        """Uptime is 'HH:MM:SS' or 'Nd HH:MM:SS'"""
        assert re.fullmatch(r"(\d+d )?\d{2}:\d{2}:\d{2}", stats["uptime"]), stats["uptime"]

    def test_millis_overflows_is_counter(self, stats):
        """millis() overflow counter is a non-negative integer"""
        assert isinstance(stats["millisOverflows"], int)
        assert stats["millisOverflows"] >= 0

    def test_heap_consistency(self, stats):
        """Free heap fits in total heap, usage is a percentage"""
        assert 0 < stats["freeHeapRaw"] <= stats["totalHeapRaw"]
        assert 0 <= stats["heapUsagePct"] <= 100

    def test_battery_fields(self, stats):
        """Battery object should contain required fields"""
        assert "battery" in stats
        battery = stats["battery"]

        required_battery_fields = [
            "voltage", "percent", "status", "source",
            "isCharging", "isUsb", "isLow", "isCritical"
        ]

        for field in required_battery_fields:
            assert field in battery, f"Missing battery field: {field}"

    def test_battery_voltage_range(self, stats):
        """Battery voltage should be in valid range"""
        voltage = stats["battery"]["voltage"]
        assert 0 <= voltage <= 5.0, f"Battery voltage out of range: {voltage}"

    def test_battery_percent_range(self, stats):
        """Battery percent should be 0-100"""
        percent = stats["battery"]["percent"]
        assert 0 <= percent <= 100, f"Battery percent out of range: {percent}"

    def test_cpu_usage_range(self, stats):
        """CPU usage is a numeric string without '%'"""
        cpu_value = float(stats["cpuUsage"])
        assert 0 <= cpu_value <= 100

    def test_rssi_range(self, stats):
        """WiFi RSSI is a numeric string in dBm"""
        rssi_value = int(stats["rssi"])
        assert -100 <= rssi_value <= 0, f"RSSI out of range: {rssi_value}"

# ═══════════════════════════════════════════════════════════════
# History Endpoint Tests
# ═══════════════════════════════════════════════════════════════

HISTORY_SERIES = ["labels", "temp", "humid", "dew", "heat"]

class TestHistoryEndpoint:
    """Tests for /history endpoint"""

    def test_history_endpoint_accessible(self, session, base_url):
        """GET /history should be accessible"""
        response = session.get(f"{base_url}/history")

        assert response.status_code == 200
        assert "application/json" in response.headers.get("Content-Type", "")

    def test_history_structure(self, history):
        """History should have all series as lists"""
        for series in HISTORY_SERIES:
            assert isinstance(history.get(series), list), f"Missing series: {series}"

    def test_history_arrays_same_length(self, history):
        """All history arrays should have same length"""
        lengths = {series: len(history[series]) for series in HISTORY_SERIES}
        assert len(set(lengths.values())) == 1, lengths

    def test_history_max_size(self, history):
        """History should not exceed HISTORY_SIZE"""
        assert len(history["labels"]) <= HISTORY_SIZE

    def test_history_last_label_is_now(self, history):
        """Labels are relative to now; the newest point is 'now'"""
        if not history["labels"]:
            pytest.skip("History is empty (device just booted)")
        assert history["labels"][-1] == "now"

    def test_history_dew_not_above_temp(self, history):
        """Dew point series never exceeds temperature series"""
        for t, dew in zip(history["temp"], history["dew"]):
            assert dew <= t + 0.1

# ═══════════════════════════════════════════════════════════════
# Reset Endpoint Tests (сбрасывает min/max на плате!)
# ═══════════════════════════════════════════════════════════════

class TestResetEndpoint:
    """Tests for /reset endpoint"""

    def test_reset_returns_success(self, session, base_url):
        """Reset should return success JSON"""
        response = session.get(f"{base_url}/reset")

        assert response.status_code == 200
        body = response.json()
        assert body.get("success") is True

    def test_reset_actually_resets(self, session, base_url):
        """After reset min == max == current (resetMinMax)"""
        session.get(f"{base_url}/reset")
        data = session.get(f"{base_url}/data").json()

        # Между /reset и /data мог пройти опрос датчика — поэтому не строгое ==
        assert data["minTemp"] <= data["temperature"] <= data["maxTemp"]
        assert data["maxTemp"] - data["minTemp"] < 0.5
        assert data["maxHumid"] - data["minHumid"] < 2.0

# ═══════════════════════════════════════════════════════════════
# Error Handling Tests
# ═══════════════════════════════════════════════════════════════

class TestErrorHandling:
    """Tests for error handling"""

    def test_404_on_invalid_path(self, session, base_url):
        """Invalid path should return 404"""
        response = session.get(f"{base_url}/nonexistent", allow_redirects=False)

        assert response.status_code == 404
        assert "/nonexistent" in response.text

    def test_post_to_get_only_endpoint(self, session, base_url):
        """Routes are GET-only; POST falls through to onNotFound"""
        response = session.post(f"{base_url}/data")
        assert response.status_code == 404

# ═══════════════════════════════════════════════════════════════
# Performance Tests
# ═══════════════════════════════════════════════════════════════

@pytest.mark.performance
class TestPerformance:
    """Performance and reliability tests"""

    @pytest.mark.parametrize("endpoint", ["/data", "/stats"])
    def test_response_time(self, session, base_url, endpoint):
        """JSON endpoints should respond quickly"""
        start = time.time()
        response = session.get(f"{base_url}{endpoint}")
        elapsed = time.time() - start

        assert response.status_code == 200
        assert elapsed < 1.0, f"Response too slow: {elapsed:.2f}s"

    def test_concurrent_requests(self, base_url):
        """Server should handle concurrent requests"""
        def make_request():
            return requests.get(f"{base_url}/data", timeout=TIMEOUT)

        with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(make_request) for _ in range(5)]
            results = [f.result() for f in concurrent.futures.as_completed(futures)]

        # Все запросы должны быть успешными
        for result in results:
            assert result.status_code == 200

    def test_data_consistency_over_time(self, session, base_url):
        """Data should be consistent across multiple requests"""
        temps = []
        for _ in range(3):
            temps.append(session.get(f"{base_url}/data").json()["temperature"])
            time.sleep(0.5)

        # Температура не должна прыгать более чем на 5°C между запросами
        max_diff = max(temps) - min(temps)
        assert max_diff < 5.0, f"Temperature jumped by {max_diff}°C"

# ═══════════════════════════════════════════════════════════════
# CORS Tests
# ═══════════════════════════════════════════════════════════════

class TestCORS:
    """CORS headers tests"""

    @pytest.mark.parametrize("endpoint", ["/data", "/stats", "/history"])
    def test_cors_headers_present(self, session, base_url, endpoint):
        """API endpoints allow cross-origin reads (setCORSHeaders)"""
        response = session.get(f"{base_url}{endpoint}")
        assert response.headers.get("Access-Control-Allow-Origin") == "*"

# ═══════════════════════════════════════════════════════════════
# Integration Tests
# ═══════════════════════════════════════════════════════════════

@pytest.mark.integration
class TestIntegration:
    """Integration tests for complete workflows"""

    def test_full_data_flow(self, session, base_url):
        """Complete data retrieval workflow"""
        data = session.get(f"{base_url}/data").json()
        stats = session.get(f"{base_url}/stats").json()
        history = session.get(f"{base_url}/history").json()

        # Все данные должны быть согласованы
        assert isinstance(data["temperature"], (int, float))
        assert isinstance(stats["battery"]["voltage"], (int, float))
        if history["temp"]:
            # Последняя точка истории — недавний замер, не дальше 5°C от текущего
            assert abs(history["temp"][-1] - data["temperature"]) < 5.0

    def test_request_counter_grows(self, session, base_url):
        """Every HTTP request increments the 'requests' counter"""
        first = session.get(f"{base_url}/stats").json()["requests"]
        session.get(f"{base_url}/data")
        second = session.get(f"{base_url}/stats").json()["requests"]
        assert second >= first + 2


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])
