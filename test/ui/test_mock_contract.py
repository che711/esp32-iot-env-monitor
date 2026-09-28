"""
Mock-сервер обязан отдавать те же JSON-поля, что и прошивка — иначе
api/ и web/ с --mock проверяли бы выдуманный формат. Ключи берутся прямо
из обработчиков в src/web_server.cpp.
"""

import re

import pytest

from mock_server import ROOT, data_payload, history_payload, stats_payload

SOURCE = (ROOT / "src" / "web_server.cpp").read_text(encoding="utf-8")


def firmware_keys(handler: str) -> set[str]:
    body = re.search(rf"void WeatherWebServer::{handler}\(\) \{{(.*?)\n\}}", SOURCE, re.S)
    assert body, f"{handler} not found in web_server.cpp"
    return set(re.findall(r'\\"(\w+)\\":', body.group(1)))


def payload_keys(payload: dict) -> set[str]:
    keys = set(payload)
    for value in payload.values():
        if isinstance(value, dict):
            keys |= set(value)
    return keys


@pytest.mark.parametrize("handler,payload,ignore", [
    # error/code — ответ 503 при отказе датчика, mock его не эмулирует
    ("handleData", data_payload(), {"error", "code"}),
    ("handleStats", stats_payload(0), set()),
    ("handleHistory", history_payload(), set()),
    ("handleReset", {"success": True, "message": ""}, set()),
])
def test_mock_json_matches_firmware(handler, payload, ignore):
    assert payload_keys(payload) == firmware_keys(handler) - ignore
