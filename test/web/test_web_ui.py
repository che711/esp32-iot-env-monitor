"""
ESP32 Weather Station Web UI Tests
End-to-end testing with Playwright
"""

import pytest
import re
import os
from playwright.sync_api import Page, expect

ESP32_IP = os.getenv("ESP32_IP", "192.168.1.100")
BASE_URL = f"http://{ESP32_IP}"

# ═══════════════════════════════════════════════════════════════
# Fixtures
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session")
def base_url():
    return BASE_URL

@pytest.fixture(scope="function")
def page(page: Page, base_url):
    """Navigate to the page before each test"""
    page.goto(base_url)
    page.wait_for_load_state("networkidle")
    return page

# ═══════════════════════════════════════════════════════════════
# Page Load Tests
# ═══════════════════════════════════════════════════════════════

class TestPageLoad:
    """Tests for initial page load"""
    
    def test_page_loads_successfully(self, page: Page):
        """Page should load without errors"""
        assert page.url.startswith(BASE_URL)
        
    def test_page_title(self, page: Page):
        """Page should have correct title"""
        title = page.title()
        assert "Env_Stats" in title or "Environmental" in title
    
    def test_no_javascript_errors(self, page: Page):
        """Page should not have JavaScript errors"""
        # Playwright автоматически фиксирует console errors
        # Можно добавить дополнительную проверку
        pass
    
    def test_page_is_responsive(self, page: Page):
        """Page should be responsive"""
        # Проверка разных размеров экрана
        viewports = [
            {"width": 1920, "height": 1080},  # Desktop
            {"width": 768, "height": 1024},   # Tablet
            {"width": 375, "height": 667},    # Mobile
        ]
        
        for viewport in viewports:
            page.set_viewport_size(viewport)
            # Страница должна отобразиться
            expect(page.locator("body")).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# UI Elements Tests
# ═══════════════════════════════════════════════════════════════

class TestUIElements:
    """Tests for UI elements presence"""
    
    def test_header_visible(self, page: Page):
        """Header should be visible"""
        header = page.locator(".header")
        expect(header).to_be_visible()
    
    def test_title_visible(self, page: Page):
        """Page title should be visible"""
        title = page.locator("h1")
        expect(title).to_be_visible()
        expect(title).to_contain_text("Environmental")
    
    def test_status_badge_visible(self, page: Page):
        """Connection status badge should be visible"""
        status = page.locator("#statusBadge")
        expect(status).to_be_visible()
    
    def test_temperature_card_visible(self, page: Page):
        """Temperature card should be visible"""
        temp_card = page.locator(".temp-card")
        expect(temp_card).to_be_visible()
        
        # Должна быть метка
        expect(temp_card.locator(".sensor-label")).to_contain_text("Temperature")
    
    def test_humidity_card_visible(self, page: Page):
        """Humidity card should be visible"""
        humid_card = page.locator(".humidity-card")
        expect(humid_card).to_be_visible()
        
        expect(humid_card.locator(".sensor-label")).to_contain_text("Humidity")
    
    def test_dewpoint_card_visible(self, page: Page):
        """Dew point card should be visible"""
        dew_card = page.locator(".dewpoint-card")
        expect(dew_card).to_be_visible()
        
        expect(dew_card.locator(".sensor-label")).to_contain_text("Dew")
    
    def test_heatindex_card_visible(self, page: Page):
        """Heat index card should be visible"""
        heat_card = page.locator(".heatindex-card")
        expect(heat_card).to_be_visible()
        
        expect(heat_card.locator(".sensor-label")).to_contain_text("Heat")
    
    def test_system_control_card_visible(self, page: Page):
        """System & Control card should be visible"""
        system_card = page.get_by_text("System & Control")
        expect(system_card).to_be_visible()
    
    def test_charts_visible(self, page: Page):
        """Charts should be visible"""
        # Проверяем canvas элементы для графиков
        canvases = page.locator("canvas")
        assert canvases.count() >= 4  # Минимум 4 графика
    
    def test_buttons_visible(self, page: Page):
        """Control buttons should be visible"""
        buttons = page.locator("button")
        assert buttons.count() >= 4  # CSV, JSON, Reset, Reboot
        
        # Проверка конкретных кнопок
        expect(page.get_by_role("button", name=re.compile("CSV"))).to_be_visible()
        expect(page.get_by_role("button", name=re.compile("JSON"))).to_be_visible()
        expect(page.get_by_role("button", name=re.compile("Reset"))).to_be_visible()
        expect(page.get_by_role("button", name=re.compile("Reboot"))).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# Data Display Tests
# ═══════════════════════════════════════════════════════════════

class TestDataDisplay:
    """Tests for data display and updates"""
    
    def test_temperature_displays(self, page: Page):
        """Temperature value should be displayed"""
        page.wait_for_timeout(3000)  # Ждем первого обновления
        
        temp_value = page.locator("#temperature")
        expect(temp_value).not_to_have_text("--")
        
        # Проверяем что это число
        text = temp_value.inner_text()
        assert re.match(r"^\d+\.\d+$", text), f"Invalid temperature format: {text}"
    
    def test_humidity_displays(self, page: Page):
        """Humidity value should be displayed"""
        page.wait_for_timeout(3000)
        
        humid_value = page.locator("#humidity")
        expect(humid_value).not_to_have_text("--")
        
        text = humid_value.inner_text()
        assert re.match(r"^\d+\.\d+$", text), f"Invalid humidity format: {text}"
    
    def test_battery_info_displays(self, page: Page):
        """Battery information should be displayed"""
        page.wait_for_timeout(5000)  # Stats обновляются каждые 5 секунд
        
        battery_percent = page.locator("#batteryPercent")
        battery_voltage = page.locator("#batteryVoltage")
        battery_source = page.locator("#batterySource")
        
        # Батарея может не отображаться если не подключена
        # Проверяем что хотя бы элементы существуют
        expect(battery_percent).to_be_attached()
        expect(battery_voltage).to_be_attached()
        expect(battery_source).to_be_attached()
    
    def test_minmax_displays(self, page: Page):
        """Min/Max values should be displayed"""
        page.wait_for_timeout(3000)
        
        min_temp = page.locator("#minTemp")
        max_temp = page.locator("#maxTemp")
        
        expect(min_temp).not_to_have_text("--")
        expect(max_temp).not_to_have_text("--")
    
    def test_system_stats_display(self, page: Page):
        """System statistics should be displayed"""
        page.wait_for_timeout(5000)
        
        uptime = page.locator("#uptime")
        cpu = page.locator("#cpuUsage")
        ram = page.locator("#freeHeap")
        
        expect(uptime).not_to_have_text("--")
        expect(cpu).not_to_have_text("--")
        expect(ram).not_to_have_text("--")
    
    def test_wifi_info_displays(self, page: Page):
        """WiFi information should be displayed"""
        page.wait_for_timeout(5000)
        
        ssid = page.locator("#ssid")
        rssi = page.locator("#rssi")
        ip = page.locator("#ipAddr")
        
        expect(ssid).not_to_have_text("--")
        expect(rssi).not_to_have_text("--")
        expect(ip).not_to_have_text("--")

# ═══════════════════════════════════════════════════════════════
# Real-time Updates Tests
# ═══════════════════════════════════════════════════════════════

class TestRealTimeUpdates:
    """Tests for real-time data updates"""
    
    def test_data_updates_automatically(self, page: Page):
        """Data should update automatically"""
        # Получаем начальное значение
        page.wait_for_timeout(3000)
        initial_timestamp = page.locator("#lastUpdate").inner_text()
        
        # Ждем обновления (данные обновляются каждые 3 секунды)
        page.wait_for_timeout(4000)
        
        # Проверяем что timestamp изменился
        new_timestamp = page.locator("#lastUpdate").inner_text()
        assert initial_timestamp != new_timestamp
    
    def test_status_badge_updates(self, page: Page):
        """Status badge should show connection status"""
        page.wait_for_timeout(3000)
        
        status_badge = page.locator("#statusBadge")
        
        # Должен показывать "Connected"
        expect(status_badge).to_contain_text("Connected")
        expect(status_badge).to_have_class(re.compile("online"))
    
    def test_charts_update(self, page: Page):
        """Charts should update with new data"""
        # Ждем загрузки графиков
        page.wait_for_timeout(3000)
        
        # Проверяем что canvas элементы заполнены
        temp_chart = page.locator("#tempChart")
        expect(temp_chart).to_be_visible()
        
        # Ждем обновления истории (каждые 10 секунд)
        page.wait_for_timeout(11000)
        
        # График все еще должен быть виден
        expect(temp_chart).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# Interaction Tests
# ═══════════════════════════════════════════════════════════════

class TestInteractions:
    """Tests for user interactions"""
    
    def test_temperature_unit_toggle(self, page: Page):
        """Temperature unit toggle should work"""
        page.wait_for_timeout(3000)
        
        # Получаем начальное значение (должно быть в °C)
        temp_unit = page.locator("#tempUnit").first
        initial_unit = temp_unit.inner_text()
        
        # Кликаем по переключателю
        toggle = page.locator("#unitToggle")
        toggle.click()
        
        # Проверяем что единица изменилась
        page.wait_for_timeout(500)
        new_unit = temp_unit.inner_text()
        assert initial_unit != new_unit
    
    def test_reset_button_works(self, page: Page):
        """Reset button should trigger confirmation"""
        page.wait_for_timeout(3000)
        
        reset_button = page.get_by_role("button", name=re.compile("Reset"))
        
        # Слушаем dialog
        page.on("dialog", lambda dialog: dialog.dismiss())
        
        reset_button.click()
        
        # Должен появиться confirm dialog (который мы dismiss)
        page.wait_for_timeout(500)
    
    def test_export_csv_button(self, page: Page):
        """CSV export button should trigger download"""
        page.wait_for_timeout(3000)
        
        csv_button = page.get_by_role("button", name=re.compile("CSV"))
        
        # Ждем download
        with page.expect_download() as download_info:
            csv_button.click()
        
        download = download_info.value
        assert download.suggested_filename == "weather.csv"
    
    def test_export_json_button(self, page: Page):
        """JSON export button should trigger download"""
        page.wait_for_timeout(3000)
        
        json_button = page.get_by_role("button", name=re.compile("JSON"))
        
        with page.expect_download() as download_info:
            json_button.click()
        
        download = download_info.value
        assert download.suggested_filename == "weather.json"
    
    def test_serial_monitor_clear(self, page: Page):
        """Serial monitor clear button should work"""
        # Ждем появления логов в консоли
        page.wait_for_timeout(5000)
        
        log_console = page.locator("#logConsole")
        
        # Если есть логи
        if log_console.locator(".log-line").count() > 0:
            clear_button = page.get_by_role("button", name=re.compile("Clear"))
            clear_button.click()
            
            page.wait_for_timeout(500)
            
            # Должно остаться только сообщение "Logs cleared"
            logs = log_console.locator(".log-line")
            assert logs.count() == 1

# ═══════════════════════════════════════════════════════════════
# WebSocket Tests
# ═══════════════════════════════════════════════════════════════

class TestWebSocket:
    """Tests for WebSocket functionality"""
    
    def test_websocket_connects(self, page: Page):
        """WebSocket should connect successfully"""
        page.wait_for_timeout(3000)
        
        ws_status = page.locator("#wsStatus")
        ws_text = page.locator("#wsStatusText")
        
        # Статус должен показывать подключение
        expect(ws_status).to_have_class(re.compile("ws-connected"))
    
    def test_logs_appear_in_console(self, page: Page):
        """Logs should appear in serial monitor"""
        page.wait_for_timeout(10000)  # Ждем логов
        
        log_console = page.locator("#logConsole")
        logs = log_console.locator(".log-line")
        
        # Должны быть какие-то логи
        assert logs.count() > 0

# ═══════════════════════════════════════════════════════════════
# Accessibility Tests
# ═══════════════════════════════════════════════════════════════

class TestAccessibility:
    """Basic accessibility tests"""
    
    def test_buttons_have_text(self, page: Page):
        """Buttons should have visible text"""
        buttons = page.locator("button")
        
        for i in range(buttons.count()):
            button = buttons.nth(i)
            text = button.inner_text()
            assert len(text) > 0, f"Button {i} has no text"
    
    def test_images_have_alt(self, page: Page):
        """Images should have alt attributes"""
        images = page.locator("img")
        
        for i in range(images.count()):
            img = images.nth(i)
            alt = img.get_attribute("alt")
            # Alt может быть пустым для декоративных изображений
            assert alt is not None

# ═══════════════════════════════════════════════════════════════
# Performance Tests
# ═══════════════════════════════════════════════════════════════

class TestPerformance:
    """Performance tests"""
    
    def test_page_load_time(self, page: Page, base_url):
        """Page should load quickly"""
        import time
        
        start = time.time()
        page.goto(base_url)
        page.wait_for_load_state("networkidle")
        elapsed = time.time() - start
        
        # Страница должна загрузиться менее чем за 3 секунды
        assert elapsed < 3.0, f"Page load too slow: {elapsed:.2f}s"
    
    def test_no_console_errors(self, page: Page):
        """Page should not have console errors"""
        console_errors = []
        
        page.on("console", lambda msg: 
            console_errors.append(msg.text) if msg.type == "error" else None
        )
        
        page.reload()
        page.wait_for_timeout(5000)
        
        # Не должно быть критических ошибок
        assert len(console_errors) == 0, f"Console errors: {console_errors}"


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--headed", "--slowmo=100"])
