"""
Mock ESP32 для тестов веб-интерфейса без платы.

Отдаёт страницу прямо из src/html_pages.h (ту же, что прошивается в
устройство) и JSON в формате прошивки для /data, /stats, /history,
/reset, /reboot. WebSocket (порт 81) не эмулируется — Serial monitor
на странице просто покажет Disconnected.

Ручной запуск — удобно править UI без перепрошивки:
    python test/mock_server.py --port 8080
"""

import argparse
import json
import math
import re
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

BUILD_TIME = "2026-01-01 12:00"
HISTORY_POINTS = 20
SENSOR_INTERVAL_SEC = 30  # SENSOR_INTERVAL в config.h


def load_page() -> str:
    src = (ROOT / "src" / "html_pages.h").read_text(encoding="utf-8")
    return re.search(r'R"rawliteral\((.*)\)rawliteral"', src, re.S).group(1)


def firmware_version() -> str:
    src = (ROOT / "src" / "config.h").read_text(encoding="utf-8")
    return re.search(r'FIRMWARE_VERSION\s*=\s*"([^"]+)"', src).group(1)


def data_payload() -> dict:
    return {
        "temperature": 22.5, "humidity": 45.0,
        "minTemp": 21.0, "maxTemp": 23.0,
        "minHumid": 40.0, "maxHumid": 50.0,
        "avgTemp": 22.0, "avgHumid": 45.0,
        "dewPoint": 10.0, "heatIndex": 22.5,
        "timestamp": 123456,
    }


def stats_payload(requests: int) -> dict:
    return {
        "uptime": "00:06:11",
        "firmware": firmware_version(),
        "buildTime": BUILD_TIME,
        "millisOverflows": 0,
        "freeHeap": "203.8 KB", "freeHeapRaw": 208691, "totalHeapRaw": 287000,
        "heapUsagePct": 27.3, "heapUsage": "27.3%",
        "cpuUsage": "10.0", "chipTemp": 40.0,
        "ssid": "MockNet", "rssi": "-58", "ip": "127.0.0.1",
        "requests": requests, "errors": 0,
        "battery": {
            "voltage": 4.07, "percent": 93, "status": "Discharging",
            "source": "Battery", "isCharging": False, "isUsb": False,
            "isLow": False, "isCritical": False,
        },
    }


def history_payload() -> dict:
    labels, temp, humid = [], [], []
    for i in range(HISTORY_POINTS):
        seconds_ago = (HISTORY_POINTS - 1 - i) * SENSOR_INTERVAL_SEC
        labels.append("now" if seconds_ago == 0 else f"-{seconds_ago / 60:g}m")
        temp.append(round(22 + math.sin(i / 3), 1))
        humid.append(round(45 + 3 * math.cos(i / 4), 1))
    return {
        "labels": labels, "temp": temp, "humid": humid,
        "dew": [round(t - 12, 1) for t in temp],
        "heat": temp,
    }


class MockHandler(BaseHTTPRequestHandler):
    """Маршруты и заголовки повторяют src/web_server.cpp."""

    server: "MockServer"

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        self.server.calls.append(path)
        data = self.server.data
        if path == "/":
            self._send(200, "text/html; charset=utf-8", self.server.page)
        elif path == "/data":
            self._json(data, {"Cache-Control": "no-cache, no-store, must-revalidate"})
        elif path == "/stats":
            self._json(stats_payload(len(self.server.calls)))
        elif path == "/history":
            self._json(history_payload())
        elif path == "/reset":
            # Как SensorManager::resetMinMax(): min = max = текущее значение
            data["minTemp"] = data["maxTemp"] = data["temperature"]
            data["minHumid"] = data["maxHumid"] = data["humidity"]
            self._json({"success": True, "message": "Min/Max reset"})
        elif path == "/reboot":
            self._json({"success": True, "message": "Rebooting..."})
        else:
            self._not_found("GET")

    def do_POST(self):
        # Прошивка регистрирует только HTTP_GET — остальное уходит в onNotFound
        self.server.calls.append(self.path)
        self._not_found("POST")

    def _not_found(self, method):
        self._send(404, "text/plain", f"404: Not Found\n\nURI: {self.path}\nMethod: {method}\n")

    def _json(self, payload, headers=None):
        cors = {
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
            "Access-Control-Allow-Headers": "Content-Type",
        }
        self._send(200, "application/json", json.dumps(payload), {**cors, **(headers or {})})

    def _send(self, code, content_type, body, headers=None):
        data = body.encode("utf-8")
        self.send_response(code)
        for name, value in (headers or {}).items():
            self.send_header(name, value)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def log_message(self, *args):
        pass


class MockServer(ThreadingHTTPServer):
    def __init__(self, port: int = 0):
        super().__init__(("127.0.0.1", port), MockHandler)
        self.page = load_page()
        self.data = data_payload()
        self.calls: list[str] = []

    @property
    def url(self) -> str:
        return f"http://127.0.0.1:{self.server_address[1]}"

    def start_background(self) -> "MockServer":
        threading.Thread(target=self.serve_forever, daemon=True).start()
        return self

    def stop(self):
        self.shutdown()
        self.server_close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    server = MockServer(args.port)
    print(f"Mock ESP32 on {server.url}  (Ctrl+C to stop)")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
