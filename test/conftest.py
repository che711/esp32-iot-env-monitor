"""
Shared pytest fixtures for ESP32 Weather Station tests
"""

import pytest
import os
import sys
from pathlib import Path

# ═══════════════════════════════════════════════════════════════
# Configuration
# ═══════════════════════════════════════════════════════════════

def pytest_configure(config):
    """Configure pytest"""
    # Регистрация кастомных маркеров
    config.addinivalue_line(
        "markers", "api: API endpoint tests"
    )
    config.addinivalue_line(
        "markers", "web: Web UI tests"
    )
    config.addinivalue_line(
        "markers", "integration: Integration tests"
    )
    config.addinivalue_line(
        "markers", "performance: Performance tests"
    )
    config.addinivalue_line(
        "markers", "hardware: Tests requiring actual hardware"
    )
    config.addinivalue_line(
        "markers", "slow: Slow running tests"
    )

# ═══════════════════════════════════════════════════════════════
# Environment Setup
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session", autouse=True)
def setup_environment():
    """Setup test environment"""
    # Загрузка .env если существует
    env_file = Path(__file__).parent / ".env"
    if env_file.exists():
        from dotenv import load_dotenv
        load_dotenv(env_file)
    
    # Проверка обязательных переменных
    esp32_ip = os.getenv("ESP32_IP")
    if not esp32_ip:
        print("\n⚠️  WARNING: ESP32_IP not set, using default 192.168.1.100")
        print("   Set ESP32_IP environment variable for your device")
        os.environ["ESP32_IP"] = "192.168.1.100"
    else:
        print(f"\n✓ ESP32 IP configured: {esp32_ip}")

# ═══════════════════════════════════════════════════════════════
# Session Fixtures
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session")
def esp32_ip():
    """Get ESP32 IP address"""
    return os.getenv("ESP32_IP", "192.168.1.100")

@pytest.fixture(scope="session")
def base_url(esp32_ip):
    """Base URL for ESP32"""
    return f"http://{esp32_ip}"

@pytest.fixture(scope="session")
def test_timeout():
    """Default timeout for requests"""
    return int(os.getenv("TEST_TIMEOUT", "5"))

# ═══════════════════════════════════════════════════════════════
# Report Customization
# ═══════════════════════════════════════════════════════════════

def pytest_html_report_title(report):
    """Customize HTML report title"""
    report.title = "ESP32 Weather Station Test Report"

@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Add extra information to test reports"""
    outcome = yield
    report = outcome.get_result()
    
    # Добавляем ESP32 IP к отчету
    if report.when == "call":
        esp32_ip = os.getenv("ESP32_IP", "192.168.1.100")
        report.extra = getattr(report, "extra", [])
        report.extra.append(pytest_html.extras.text(
            f"ESP32 IP: {esp32_ip}", 
            name="Environment"
        ))

# ═══════════════════════════════════════════════════════════════
# Cleanup
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="function", autouse=True)
def cleanup_after_test():
    """Cleanup after each test"""
    yield
    # Cleanup code here if needed
    pass

# ═══════════════════════════════════════════════════════════════
# Skip Conditions
# ═══════════════════════════════════════════════════════════════

def pytest_collection_modifyitems(config, items):
    """Modify test collection"""
    # Пропускаем hardware тесты если флаг не установлен
    if not config.getoption("--hardware", default=False):
        skip_hardware = pytest.mark.skip(reason="Requires --hardware flag")
        for item in items:
            if "hardware" in item.keywords:
                item.add_marker(skip_hardware)

def pytest_addoption(parser):
    """Add custom command line options"""
    parser.addoption(
        "--hardware",
        action="store_true",
        default=False,
        help="Run hardware-dependent tests"
    )
    parser.addoption(
        "--slow",
        action="store_true",
        default=False,
        help="Run slow tests"
    )

# ═══════════════════════════════════════════════════════════════
# Playwright Configuration
# ═══════════════════════════════════════════════════════════════

@pytest.fixture(scope="session")
def browser_context_args(browser_context_args):
    """Configure Playwright browser context"""
    return {
        **browser_context_args,
        "viewport": {
            "width": 1920,
            "height": 1080,
        },
        "ignore_https_errors": True,
    }

@pytest.fixture(scope="session")
def browser_type_launch_args(browser_type_launch_args):
    """Configure Playwright browser launch"""
    return {
        **browser_type_launch_args,
        "headless": os.getenv("HEADLESS", "true").lower() == "true",
        "slow_mo": int(os.getenv("SLOW_MO", "0")),
    }
