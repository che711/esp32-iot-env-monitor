"""
Тесты веб-интерфейса без платы: страница из src/html_pages.h против
mock-сервера (test/mock_server.py). Запускаются в CI.

    cd test && pytest ui
"""

import re

import pytest
from playwright.sync_api import Page, expect

from mock_server import BUILD_TIME, MockServer, firmware_version

pytestmark = pytest.mark.web


@pytest.fixture(scope="module")
def mock_device():
    server = MockServer().start_background()
    yield server
    server.stop()


@pytest.fixture
def dashboard(page: Page, mock_device):
    """Открытая страница с загруженными данными; падает, если был JS-exception."""
    errors = []
    page.on("pageerror", lambda exc: errors.append(str(exc)))
    page.set_viewport_size({"width": 1280, "height": 900})
    page.goto(mock_device.url)
    expect(page.locator("#temperature")).to_have_text("22.5")
    yield page
    assert errors == [], f"JavaScript errors on page: {errors}"


def card(page: Page, key: str):
    return page.locator(f'[data-collapse="{key}"]')


# ═══════════════════════════════════════════════════════════════
# Данные и шапка
# ═══════════════════════════════════════════════════════════════

class TestDataDisplay:
    def test_title(self, dashboard: Page):
        assert dashboard.title() == "ENV Station"

    def test_sensor_values(self, dashboard: Page):
        expect(dashboard.locator("#humidity")).to_have_text("45.0")
        expect(dashboard.locator("#dewPoint")).to_have_text("10.0")
        expect(dashboard.locator("#heatIndex")).to_have_text("22.5")

    def test_firmware_badge_shows_version_and_build_time(self, dashboard: Page):
        badge = dashboard.locator("#fwBadge")
        expect(badge).to_be_visible()
        expect(badge).to_have_text(f"v{firmware_version()} · {BUILD_TIME}")

    def test_uptime_and_millis_overflows(self, dashboard: Page):
        expect(dashboard.locator("#uptime")).to_have_text("00:06:11")
        expect(dashboard.locator("#millisOverflows")).to_have_text("0")


# ═══════════════════════════════════════════════════════════════
# Вёрстка
# ═══════════════════════════════════════════════════════════════

class TestLayout:
    def test_system_grid_rows_are_filled(self, dashboard: Page):
        """Каждый ряд плиток System & control доходит до правого края сетки."""
        rows = dashboard.evaluate("""() => {
            const grid = document.querySelector('.info-grid');
            const right = grid.getBoundingClientRect().right;
            const rows = {};
            grid.querySelectorAll(':scope > .info-item').forEach(el => {
                const r = el.getBoundingClientRect();
                const top = Math.round(r.top);
                rows[top] = Math.max(rows[top] || 0, r.right);
            });
            return Object.values(rows).map(r => Math.abs(right - r));
        }""")
        assert len(rows) == 3
        assert all(gap < 1 for gap in rows), f"row gaps to the right edge: {rows}"

    @pytest.mark.parametrize("width", [375, 768])
    def test_no_horizontal_scroll_after_shrinking(self, dashboard: Page, width):
        """Графики должны ужиматься вслед за окном (поворот телефона, resize)."""
        dashboard.set_viewport_size({"width": width, "height": 800})
        dashboard.wait_for_function(
            "() => document.documentElement.scrollWidth <= window.innerWidth")


# ═══════════════════════════════════════════════════════════════
# Сворачиваемые блоки
# ═══════════════════════════════════════════════════════════════

CARDS = ["temp", "humidity", "dewpoint", "heatindex", "system", "serial", "history"]


class TestCollapsible:
    def test_every_card_has_collapse_button(self, dashboard: Page):
        assert dashboard.locator("[data-collapse]").count() == len(CARDS)
        for key in CARDS:
            btn = card(dashboard, key).locator(".collapse-btn")
            expect(btn).to_be_visible()
            expect(btn).to_have_attribute("aria-expanded", "true")

    @pytest.mark.parametrize("key", CARDS)
    def test_button_toggles_card(self, dashboard: Page, key):
        body = card(dashboard, key).locator(".card-body")
        btn = card(dashboard, key).locator(".collapse-btn")
        btn.click()
        expect(body).to_be_hidden()
        expect(btn).to_have_attribute("aria-expanded", "false")
        btn.click()
        expect(body).to_be_visible()
        expect(btn).to_have_attribute("aria-expanded", "true")

    def test_click_on_title_toggles_card(self, dashboard: Page):
        card(dashboard, "humidity").locator(".sensor-label").click()
        expect(dashboard.locator("#humidity")).to_be_hidden()

    def test_state_survives_reload(self, dashboard: Page):
        card(dashboard, "system").locator(".collapse-btn").click()
        dashboard.reload()
        expect(dashboard.locator("#temperature")).to_have_text("22.5")
        expect(dashboard.locator("#uptime")).to_be_hidden()
        expect(dashboard.locator("#humidity")).to_be_visible()

    def test_chart_range_buttons_do_not_collapse(self, dashboard: Page):
        history = card(dashboard, "history")
        btn_5m = history.get_by_role("button", name="5m", exact=True)
        btn_5m.click()
        expect(btn_5m).to_have_class(re.compile(r"\bactive\b"))
        expect(history).not_to_have_class(re.compile(r"\bcollapsed\b"))
        expect(dashboard.locator("#combinedChart")).to_be_visible()

    def test_collapsed_card_does_not_stretch_to_neighbour(self, dashboard: Page):
        card(dashboard, "humidity").locator(".collapse-btn").click()
        collapsed = card(dashboard, "humidity").bounding_box()["height"]
        neighbour = card(dashboard, "temp").bounding_box()["height"]
        assert collapsed < neighbour / 2


# ═══════════════════════════════════════════════════════════════
# Действия
# ═══════════════════════════════════════════════════════════════

class TestActions:
    def test_reset_asks_confirmation_then_calls_device(self, dashboard: Page, mock_device):
        dashboard.once("dialog", lambda d: d.accept())
        with dashboard.expect_request("**/reset"):
            dashboard.get_by_role("button", name="Reset min/max").click()
        assert "/reset" in mock_device.calls

    def test_reboot_cancelled_does_not_call_device(self, dashboard: Page, mock_device):
        dashboard.once("dialog", lambda d: d.dismiss())
        dashboard.get_by_role("button", name="Reboot").click()
        dashboard.wait_for_timeout(300)
        assert "/reboot" not in mock_device.calls
