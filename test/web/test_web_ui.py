"""
ESP32 Weather Station Web UI Tests
End-to-end testing with Playwright

На плате:        ESP32_IP=192.168.1.65 pytest web
Без платы (CI):  pytest web --mock   (WebSocket-тесты пропускаются)
"""

import re
import time

import pytest
from playwright.sync_api import Page, expect

pytestmark = pytest.mark.web

# Интервалы опроса на странице (setInterval в html_pages.h) + запас
DATA_INTERVAL_MS = 10_000
HISTORY_INTERVAL_MS = 15_000
SLACK_MS = 5_000

NUMBER = re.compile(r"^-?\d+\.\d$")

# ═══════════════════════════════════════════════════════════════
# Fixtures
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="function")
def page(page: Page, base_url):
    """Open the dashboard and wait for the first data update"""
    page.goto(base_url)
    expect(page.locator("#temperature")).not_to_have_text("--", timeout=SLACK_MS)
    return page

# ═══════════════════════════════════════════════════════════════
# Page Load Tests
# ═══════════════════════════════════════════════════════════════

class TestPageLoad:
    """Tests for initial page load"""

    def test_page_loads_successfully(self, page: Page, base_url):
        """Page should load without errors"""
        assert page.url.startswith(base_url)

    def test_page_title(self, page: Page):
        """Page should have correct title"""
        assert page.title() == "ENV Station"

    def test_no_javascript_errors(self, page: Page):
        """Page should not throw JavaScript exceptions"""
        errors = []
        page.on("pageerror", lambda exc: errors.append(str(exc)))
        page.reload()
        expect(page.locator("#temperature")).not_to_have_text("--", timeout=SLACK_MS)
        assert errors == [], f"JavaScript errors: {errors}"

    @pytest.mark.parametrize("viewport", [
        {"width": 1920, "height": 1080},  # Desktop
        {"width": 768, "height": 1024},   # Tablet
        {"width": 375, "height": 667},    # Mobile
    ])
    def test_page_is_responsive(self, page: Page, viewport):
        """Page fits the viewport without horizontal scroll"""
        page.set_viewport_size(viewport)
        expect(page.locator(".header")).to_be_visible()
        page.wait_for_function(
            "() => document.documentElement.scrollWidth <= window.innerWidth")

# ═══════════════════════════════════════════════════════════════
# UI Elements Tests
# ═══════════════════════════════════════════════════════════════

class TestUIElements:
    """Tests for UI elements presence"""

    def test_header_visible(self, page: Page):
        """Header should be visible"""
        expect(page.locator(".header")).to_be_visible()

    def test_title_visible(self, page: Page):
        """Page title should be visible"""
        title = page.locator("h1")
        expect(title).to_be_visible()
        expect(title).to_have_text("Environment")

    def test_status_badge_visible(self, page: Page):
        """Connection status badge should be visible"""
        expect(page.locator("#statusBadge")).to_be_visible()

    def test_firmware_badge_visible(self, page: Page):
        """Firmware badge shows version and build time"""
        badge = page.locator("#fwBadge")
        expect(badge).to_be_visible(timeout=SLACK_MS)
        expect(badge).to_have_text(re.compile(r"^v\d+\.\d+(\.\d+)? · \d{4}-\d{2}-\d{2} \d{2}:\d{2}$"))

    @pytest.mark.parametrize("card_class,label", [
        ("temp-card", "Temperature"),
        ("humidity-card", "Humidity"),
        ("dewpoint-card", "Dew point"),
        ("heatindex-card", "Heat index"),
    ])
    def test_sensor_card_visible(self, page: Page, card_class, label):
        """Sensor cards should be visible with their labels"""
        card = page.locator(f".{card_class}")
        expect(card).to_be_visible()
        expect(card.locator(".sensor-label")).to_have_text(label)

    def test_system_control_card_visible(self, page: Page):
        """System & control card should be visible"""
        expect(page.get_by_role("heading", name="System & control")).to_be_visible()

    def test_charts_visible(self, page: Page):
        """History chart and four sparklines should be present"""
        expect(page.locator("#combinedChart")).to_be_visible()
        expect(page.locator("canvas.sparkline")).to_have_count(4)

    def test_buttons_visible(self, page: Page):
        """Control buttons should be visible"""
        for name in ("Export CSV", "Export JSON", "Reset min/max", "Reboot"):
            expect(page.get_by_role("button", name=name, exact=True)).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# Data Display Tests
# ═══════════════════════════════════════════════════════════════

class TestDataDisplay:
    """Tests for data display and updates"""

    @pytest.mark.parametrize("element_id", [
        "temperature", "humidity", "dewPoint", "heatIndex",
        "minTemp", "maxTemp", "avgTemp", "minHumid", "maxHumid", "avgHumid",
    ])
    def test_sensor_value_displays(self, page: Page, element_id):
        """Sensor values are numbers with one decimal"""
        expect(page.locator(f"#{element_id}")).to_have_text(NUMBER)

    def test_battery_info_displays(self, page: Page):
        """Battery information should be displayed"""
        # На батарее — "92%", от USB — "USB power"
        expect(page.locator("#batteryPercent")).to_have_text(re.compile(r"\d+%|USB power"), timeout=SLACK_MS)
        expect(page.locator("#batteryVoltage")).to_have_text(re.compile(r"\d(\.\d+)?V"))
        expect(page.locator("#batterySource")).not_to_have_text("--")

    def test_system_stats_display(self, page: Page):
        """System statistics should be displayed"""
        expect(page.locator("#uptime")).to_have_text(re.compile(r"\d{2}:\d{2}:\d{2}"), timeout=SLACK_MS)
        expect(page.locator("#millisOverflows")).to_have_text(re.compile(r"^\d+$"))
        expect(page.locator("#cpuUsage")).to_have_text(re.compile(r"%$"))
        expect(page.locator("#freeHeap")).not_to_have_text("--")

    def test_wifi_info_displays(self, page: Page):
        """WiFi information should be displayed"""
        expect(page.locator("#ssid")).not_to_have_text("--", timeout=SLACK_MS)
        expect(page.locator("#rssi")).to_have_text(re.compile(r"^-\d+ dBm$"))
        expect(page.locator("#ipAddr")).to_have_text(re.compile(r"^\d+\.\d+\.\d+\.\d+$"))

# ═══════════════════════════════════════════════════════════════
# Real-time Updates Tests
# ═══════════════════════════════════════════════════════════════

@pytest.mark.slow
class TestRealTimeUpdates:
    """Tests for real-time data updates"""

    def test_data_updates_automatically(self, page: Page):
        """Last-update time changes on the next /data poll"""
        initial = page.locator("#lastUpdate").inner_text()
        page.wait_for_function(
            "t => document.getElementById('lastUpdate').textContent !== t",
            arg=initial, timeout=DATA_INTERVAL_MS + SLACK_MS)

    def test_status_badge_updates(self, page: Page):
        """Status badge should show connection status"""
        status_badge = page.locator("#statusBadge")
        expect(status_badge).to_contain_text("Connected")
        expect(status_badge).to_have_class(re.compile(r"\bonline\b"))

    def test_charts_update(self, page: Page):
        """History chart re-renders on the next /history poll"""
        updated = page.locator("#updateTimeCombined")
        expect(updated).not_to_have_text("--", timeout=SLACK_MS)
        initial = updated.inner_text()
        page.wait_for_function(
            "t => document.getElementById('updateTimeCombined').textContent !== t",
            arg=initial, timeout=HISTORY_INTERVAL_MS + SLACK_MS)
        expect(page.locator("#combinedChart")).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# Interaction Tests
# ═══════════════════════════════════════════════════════════════

class TestInteractions:
    """Tests for user interactions"""

    def test_temperature_unit_toggle(self, page: Page):
        """Temperature unit toggle switches °C ↔ °F"""
        temp_unit = page.locator("#tempUnit")
        expect(temp_unit).to_have_text("°C")

        page.locator(".toggle-switch:has(#unitToggle)").click()
        expect(temp_unit).to_have_text("°F")

    def test_reset_cancelled_does_not_call_device(self, page: Page):
        """Reset asks for confirmation; cancel sends nothing to the device"""
        requests = []
        page.on("request", lambda r: requests.append(r.url))
        page.once("dialog", lambda dialog: dialog.dismiss())

        page.get_by_role("button", name="Reset min/max").click()
        page.wait_for_timeout(500)
        assert not any(url.endswith("/reset") for url in requests)

    @pytest.mark.parametrize("button,filename", [
        ("Export CSV", "weather.csv"),
        ("Export JSON", "weather.json"),
    ])
    def test_export_downloads_file(self, page: Page, button, filename):
        """Export buttons download the chart data"""
        expect(page.locator("#updateTimeCombined")).not_to_have_text("--", timeout=SLACK_MS)
        with page.expect_download() as download_info:
            page.get_by_role("button", name=button).click()

        assert download_info.value.suggested_filename == filename

    def test_serial_monitor_clear(self, page: Page):
        """Serial monitor clear leaves only the 'Logs cleared' line"""
        log_lines = page.locator("#logConsole .log-line")
        expect(log_lines.first).to_be_attached(timeout=SLACK_MS)

        page.get_by_role("button", name="Clear", exact=True).click()
        expect(log_lines).to_have_count(1)
        expect(log_lines.first).to_contain_text("Logs cleared")

    def test_card_collapse_toggle(self, page: Page):
        """Cards collapse via the chevron button"""
        system = page.locator('[data-collapse="system"]')
        system.locator(".collapse-btn").click()
        expect(page.locator("#uptime")).to_be_hidden()
        system.locator(".collapse-btn").click()
        expect(page.locator("#uptime")).to_be_visible()

# ═══════════════════════════════════════════════════════════════
# WebSocket Tests (только на плате: mock не поднимает порт 81)
# ═══════════════════════════════════════════════════════════════

@pytest.mark.device_only
class TestWebSocket:
    """Tests for WebSocket functionality"""

    def test_websocket_connects(self, page: Page):
        """WebSocket should connect successfully"""
        expect(page.locator("#wsStatus")).to_have_class(re.compile("ws-connected"), timeout=SLACK_MS)
        expect(page.locator("#wsStatusText")).to_have_text("Connected")

    def test_welcome_message_in_console(self, page: Page):
        """Device greets a new WebSocket client in the serial monitor"""
        expect(page.locator("#logConsole")).to_contain_text("Serial Monitor connected", timeout=SLACK_MS)

# ═══════════════════════════════════════════════════════════════
# Accessibility Tests
# ═══════════════════════════════════════════════════════════════

class TestAccessibility:
    """Basic accessibility tests"""

    def test_buttons_have_accessible_name(self, page: Page):
        """Every button has text, aria-label or title (icon buttons)"""
        unnamed = page.evaluate("""() => [...document.querySelectorAll('button')]
            .filter(b => !(b.textContent.trim() || b.getAttribute('aria-label') || b.title))
            .map(b => b.outerHTML.slice(0, 80))""")
        assert unnamed == [], f"Buttons without accessible name: {unnamed}"

    def test_images_have_alt(self, page: Page):
        """Images should have alt attributes"""
        images = page.locator("img")

        for i in range(images.count()):
            # Alt может быть пустым для декоративных изображений
            assert images.nth(i).get_attribute("alt") is not None

# ═══════════════════════════════════════════════════════════════
# Performance Tests
# ═══════════════════════════════════════════════════════════════

@pytest.mark.performance
class TestPerformance:
    """Performance tests"""

    def test_page_load_time(self, page: Page, base_url):
        """Page should load quickly"""
        start = time.time()
        page.goto(base_url, wait_until="load")
        elapsed = time.time() - start

        # Страница должна загрузиться менее чем за 3 секунды
        assert elapsed < 3.0, f"Page load too slow: {elapsed:.2f}s"

    def test_no_console_errors(self, page: Page, mock_mode):
        """Page should not log errors to the console"""
        console_errors = []
        page.on("console", lambda msg:
            console_errors.append(msg.text) if msg.type == "error" else None
        )

        page.reload()
        expect(page.locator("#temperature")).not_to_have_text("--", timeout=SLACK_MS)
        page.wait_for_timeout(2000)

        if mock_mode:
            # У mock нет WebSocket-сервера на порту 81
            console_errors = [e for e in console_errors if "WebSocket" not in e]
        assert console_errors == [], f"Console errors: {console_errors}"


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--headed", "--slowmo=100"])
