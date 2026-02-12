"""
ESP32 Weather Station API Tests
Python-based API testing with pytest
"""

import pytest
import requests
import json
import time
from typing import Dict, Any
import os

# ═══════════════════════════════════════════════════════════════
# Configuration
# ═══════════════════════════════════════════════════════════════

ESP32_IP = os.getenv("ESP32_IP", "192.168.1.100")
BASE_URL = f"http://{ESP32_IP}"
TIMEOUT = 5

# ═══════════════════════════════════════════════════════════════
# Fixtures
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session")
def base_url():
    """Base URL for API requests"""
    return BASE_URL

@pytest.fixture(scope="session")
def session():
    """Requests session with default timeout"""
    s = requests.Session()
    s.request = lambda *args, **kwargs: requests.Session.request(
        s, *args, timeout=TIMEOUT, **kwargs
    )
    return s

@pytest.fixture(scope="session", autouse=True)
def check_connectivity(session):
    """Verify ESP32 is reachable before running tests"""
    try:
        response = session.get(BASE_URL)
        assert response.status_code == 200
        print(f"\n✓ ESP32 is reachable at {BASE_URL}")
    except requests.exceptions.RequestException as e:
        pytest.fail(f"Cannot connect to ESP32 at {BASE_URL}: {e}")

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
        
        assert "Environmental Statistics" in response.text or \
               "Env_Stats" in response.text
    
    def test_root_contains_sensor_elements(self, session, base_url):
        """HTML should contain sensor data elements"""
        response = session.get(f"{base_url}/")
        
        assert "temperature" in response.text.lower()
        assert "humidity" in response.text.lower()
        
    def test_root_contains_javascript(self, session, base_url):
        """HTML should contain JavaScript"""
        response = session.get(f"{base_url}/")
        
        assert "<script" in response.text
        assert "updateData" in response.text or \
               "fetch" in response.text

# ═══════════════════════════════════════════════════════════════
# Data Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestDataEndpoint:
    """Tests for /data endpoint"""
    
    def test_data_endpoint_accessible(self, session, base_url):
        """GET /data should be accessible"""
        response = session.get(f"{base_url}/data")
        
        assert response.status_code == 200
        assert "application/json" in response.headers.get("Content-Type", "")
    
    def test_data_returns_valid_json(self, session, base_url):
        """GET /data should return valid JSON"""
        response = session.get(f"{base_url}/data")
        
        try:
            data = response.json()
            assert isinstance(data, dict)
        except json.JSONDecodeError:
            pytest.fail("Response is not valid JSON")
    
    def test_data_required_fields(self, session, base_url):
        """GET /data should contain all required fields"""
        response = session.get(f"{base_url}/data")
        data = response.json()
        
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
    
    def test_temperature_range(self, session, base_url):
        """Temperature should be in valid range"""
        response = session.get(f"{base_url}/data")
        data = response.json()
        
        temp = data["temperature"]
        assert -40 <= temp <= 85, f"Temperature out of range: {temp}"
    
    def test_humidity_range(self, session, base_url):
        """Humidity should be in valid range"""
        response = session.get(f"{base_url}/data")
        data = response.json()
        
        humid = data["humidity"]
        assert 0 <= humid <= 100, f"Humidity out of range: {humid}"
    
    def test_minmax_consistency(self, session, base_url):
        """Min should be <= current <= Max"""
        response = session.get(f"{base_url}/data")
        data = response.json()
        
        # Temperature
        assert data["minTemp"] <= data["temperature"] <= data["maxTemp"]
        
        # Humidity
        assert data["minHumid"] <= data["humidity"] <= data["maxHumid"]
    
    def test_timestamp_is_integer(self, session, base_url):
        """Timestamp should be a valid integer"""
        response = session.get(f"{base_url}/data")
        data = response.json()
        
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
    
    def test_stats_required_fields(self, session, base_url):
        """GET /stats should contain system information"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        required_fields = [
            "uptime", "freeHeap", "heapUsage", "cpuUsage",
            "ssid", "rssi", "ip", "requests", "errors", "battery"
        ]
        
        for field in required_fields:
            assert field in data, f"Missing field: {field}"
    
    def test_battery_fields(self, session, base_url):
        """Battery object should contain required fields"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        assert "battery" in data
        battery = data["battery"]
        
        required_battery_fields = [
            "voltage", "percent", "status", "source",
            "isCharging", "isUsb", "isLow", "isCritical"
        ]
        
        for field in required_battery_fields:
            assert field in battery, f"Missing battery field: {field}"
    
    def test_battery_voltage_range(self, session, base_url):
        """Battery voltage should be in valid range"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        voltage = data["battery"]["voltage"]
        assert 0 <= voltage <= 5.0, f"Battery voltage out of range: {voltage}"
    
    def test_battery_percent_range(self, session, base_url):
        """Battery percent should be 0-100"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        percent = data["battery"]["percent"]
        assert 0 <= percent <= 100, f"Battery percent out of range: {percent}"
    
    def test_cpu_usage_range(self, session, base_url):
        """CPU usage should be valid percentage string"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        cpu_str = data["cpuUsage"]
        # Удаляем % если есть
        cpu_value = float(cpu_str.replace("%", ""))
        assert 0 <= cpu_value <= 100
    
    def test_rssi_range(self, session, base_url):
        """WiFi RSSI should be in valid range"""
        response = session.get(f"{base_url}/stats")
        data = response.json()
        
        rssi_str = data["rssi"]
        rssi_value = int(rssi_str.split()[0])  # "XX dBm" -> XX
        assert -100 <= rssi_value <= 0, f"RSSI out of range: {rssi_value}"

# ═══════════════════════════════════════════════════════════════
# History Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestHistoryEndpoint:
    """Tests for /history endpoint"""
    
    def test_history_endpoint_accessible(self, session, base_url):
        """GET /history should be accessible"""
        response = session.get(f"{base_url}/history")
        
        assert response.status_code == 200
        assert "application/json" in response.headers.get("Content-Type", "")
    
    def test_history_structure(self, session, base_url):
        """History should have correct structure"""
        response = session.get(f"{base_url}/history")
        data = response.json()
        
        assert "labels" in data
        assert "temp" in data
        assert "humid" in data
        
        assert isinstance(data["labels"], list)
        assert isinstance(data["temp"], list)
        assert isinstance(data["humid"], list)
    
    def test_history_arrays_same_length(self, session, base_url):
        """All history arrays should have same length"""
        response = session.get(f"{base_url}/history")
        data = response.json()
        
        labels_len = len(data["labels"])
        temp_len = len(data["temp"])
        humid_len = len(data["humid"])
        
        assert labels_len == temp_len == humid_len
    
    def test_history_max_size(self, session, base_url):
        """History should not exceed maximum size"""
        response = session.get(f"{base_url}/history")
        data = response.json()
        
        # По умолчанию HISTORY_SIZE = 60
        assert len(data["labels"]) <= 60

# ═══════════════════════════════════════════════════════════════
# Reset Endpoint Tests
# ═══════════════════════════════════════════════════════════════

class TestResetEndpoint:
    """Tests for /reset endpoint"""
    
    def test_reset_endpoint_accessible(self, session, base_url):
        """GET /reset should be accessible"""
        response = session.get(f"{base_url}/reset")
        
        assert response.status_code == 200
    
    def test_reset_returns_success(self, session, base_url):
        """Reset should return success message"""
        response = session.get(f"{base_url}/reset")
        data = response.json()
        
        assert "success" in data
        assert data["success"] is True
    
    def test_reset_actually_resets(self, session, base_url):
        """Reset should actually reset min/max values"""
        # Получаем текущие данные
        response1 = session.get(f"{base_url}/data")
        data1 = response1.json()
        
        # Сбрасываем
        session.get(f"{base_url}/reset")
        
        # Ждем немного
        time.sleep(1)
        
        # Получаем новые данные
        response2 = session.get(f"{base_url}/data")
        data2 = response2.json()
        
        # После сброса min/max должны быть близки к текущим значениям
        temp_diff = abs(data2["maxTemp"] - data2["temperature"])
        assert temp_diff < 2.0  # В пределах 2°C

# ═══════════════════════════════════════════════════════════════
# Error Handling Tests
# ═══════════════════════════════════════════════════════════════

class TestErrorHandling:
    """Tests for error handling"""
    
    def test_404_on_invalid_path(self, session, base_url):
        """Invalid path should return 404"""
        response = session.get(f"{base_url}/nonexistent", allow_redirects=False)
        
        assert response.status_code == 404
    
    def test_method_not_allowed(self, session, base_url):
        """POST to GET-only endpoint should fail gracefully"""
        try:
            response = session.post(f"{base_url}/data")
            # ESP32 может вернуть 404 или 405
            assert response.status_code in [404, 405, 501]
        except requests.exceptions.RequestException:
            # Некоторые серверы могут просто разорвать соединение
            pass

# ═══════════════════════════════════════════════════════════════
# Performance Tests
# ═══════════════════════════════════════════════════════════════

class TestPerformance:
    """Performance and reliability tests"""
    
    def test_response_time_data(self, session, base_url):
        """Data endpoint should respond quickly"""
        start = time.time()
        response = session.get(f"{base_url}/data")
        elapsed = time.time() - start
        
        assert response.status_code == 200
        assert elapsed < 1.0, f"Response too slow: {elapsed:.2f}s"
    
    def test_response_time_stats(self, session, base_url):
        """Stats endpoint should respond quickly"""
        start = time.time()
        response = session.get(f"{base_url}/stats")
        elapsed = time.time() - start
        
        assert response.status_code == 200
        assert elapsed < 1.0, f"Response too slow: {elapsed:.2f}s"
    
    def test_concurrent_requests(self, session, base_url):
        """Server should handle concurrent requests"""
        import concurrent.futures
        
        def make_request():
            return session.get(f"{base_url}/data")
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(make_request) for _ in range(5)]
            results = [f.result() for f in concurrent.futures.as_completed(futures)]
        
        # Все запросы должны быть успешными
        for result in results:
            assert result.status_code == 200
    
    def test_data_consistency_over_time(self, session, base_url):
        """Data should be consistent across multiple requests"""
        responses = []
        for _ in range(3):
            response = session.get(f"{base_url}/data")
            responses.append(response.json())
            time.sleep(0.5)
        
        # Температура не должна прыгать более чем на 5°C между запросами
        temps = [r["temperature"] for r in responses]
        max_diff = max(temps) - min(temps)
        assert max_diff < 5.0, f"Temperature jumped by {max_diff}°C"

# ═══════════════════════════════════════════════════════════════
# CORS Tests
# ═══════════════════════════════════════════════════════════════

class TestCORS:
    """CORS headers tests"""
    
    def test_cors_headers_present(self, session, base_url):
        """API should include CORS headers"""
        response = session.get(f"{base_url}/data")
        
        # CORS headers могут присутствовать
        headers = response.headers
        if "Access-Control-Allow-Origin" in headers:
            assert headers["Access-Control-Allow-Origin"] == "*" or \
                   headers["Access-Control-Allow-Origin"] != ""

# ═══════════════════════════════════════════════════════════════
# Integration Tests
# ═══════════════════════════════════════════════════════════════

class TestIntegration:
    """Integration tests for complete workflows"""
    
    def test_full_data_flow(self, session, base_url):
        """Complete data retrieval workflow"""
        # 1. Получить данные датчиков
        data_response = session.get(f"{base_url}/data")
        assert data_response.status_code == 200
        data = data_response.json()
        
        # 2. Получить системную статистику
        stats_response = session.get(f"{base_url}/stats")
        assert stats_response.status_code == 200
        stats = stats_response.json()
        
        # 3. Получить историю
        history_response = session.get(f"{base_url}/history")
        assert history_response.status_code == 200
        history = history_response.json()
        
        # Все данные должны быть согласованы
        assert len(history["temp"]) > 0
        assert data["temperature"] is not None
        assert stats["battery"]["voltage"] > 0


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])
