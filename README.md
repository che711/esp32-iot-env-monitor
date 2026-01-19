# esp32-iot-env-monitor
ESP32‑based IoT project for monitoring temperature and humidity from an AHT10 sensor and displaying data via an integrated web server at an IP address on a local network.

# ESP32 + AHT10 Web Server

Веб-сервер на ESP32 для мониторинга температуры и влажности с датчика AHT10 через браузер.

## 🌟 Возможности

- ✅ Веб-интерфейс для просмотра данных
- ✅ Автоматическое обновление каждые 2 секунды
- ✅ Красивый адаптивный дизайн
- ✅ JSON API для интеграции
- ✅ Поддержка WiFi

## 🔧 Компоненты

- ESP32 Development Board
- AHT10 Temperature & Humidity Sensor
- Провода для подключения

## 📟 Подключение

| AHT10 | ESP32 |
|-------|-------|
| VIN   | 3.3V  |
| GND   | GND   |
| SDA   | GPIO 21 |
| SCL   | GPIO 22 |

## 📚 Установка библиотек

Через Arduino IDE Library Manager установите:
- `Adafruit AHTX0`
- `Adafruit BusIO`

## 🚀 Быстрый старт

1. Откройте скетч в Arduino IDE: `src/ESP32_AHT10_WebServer/ESP32_AHT10_WebServer.ino`
2. Измените WiFi credentials:
```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
```
3. Выберите плату: `Tools > Board > ESP32 Dev Module`
4. Загрузите скетч на ESP32
5. Откройте Serial Monitor (115200 baud)
6. Найдите IP адрес в Serial Monitor
7. Откройте браузер и введите IP адрес

## 📡 API Endpoints

- `GET /` - Веб-интерфейс
- `GET /data` - JSON данные:
```json
  {
    "temperature": 23.45,
    "humidity": 56.78
  }
```

## 📸 Скриншоты

![Web Interface](docs/images/web-interface.png)
![Wiring Diagram](docs/images/wiring-diagram.png)

## 📄 Лицензия

MIT License - свободное использование

## 🤝 Вклад

Pull requests приветствуются! Для крупных изменений сначала откройте issue.

## 👤 Автор

[@che711](https://github.com/che711)