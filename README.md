# 🌡️ ESP32 Super Mini Weather Station

A modern weather station based on the **ESP32 Super Mini** with the **AHT10** sensor for real-time temperature and humidity monitoring.

![Version](https://img.shields.io/badge/version-3.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-green)
![License](https://img.shields.io/badge/license-MIT-yellow)

## 📋 Scope

### 🎯 Functions
- ✅ Real-time temperature and humidity monitoring
- 📊 Four interactive graphs (temperature, humidity, perceived temperature, and felt temperature)
- 💧 Dew Point
- 🌡️ Heat Index
- 📈 History data
- 🔄 Auto update
- 📱 Adaptive web UI for all devices
- 🌐 WiFi with automatic reconnection
- 💾 Exporting data to CSV and JSON
  
### 🎨 Web UI
- Modern gradient design
- Comfort indicators for temperature and humidity
- Displaying min/max/avg temperature and humidity values
- System info (uptime, RAM, CPU, WiFi)
- Device management (reset, reboot)
- Switch °C / °F

## 🔧 Компоненты

### Оборудование
- **ESP32 Super Mini** - микроконтроллер
- **AHT10** - датчик температуры и влажности (I2C)
- Wires, a soldering iron, solder, rosin, and other components for assembly
  
### Программное обеспечение
- **PlatformIO** or **Arduino IDE**
- **Libraries:**
  - Adafruit_AHTX0
  - WebServer (встроена в ESP32)
  - WiFi (встроена в ESP32)

## 📐 Схема подключения

```
ESP32 Super Mini → AHT10
─────────────────────────
3.3V              → VCC
GND               → GND
GPIO 8            → SDA
GPIO 9            → SCL
```

⚠️ **Important:** ESP32 Super Mini works on 3.3V! Do not connect 5V to the sensor!

## 🚀 Installation and configuration

### 1. Installing PlatformIO

```bash
# Install Visual Studio Code
# Install the PlatformIO IDE extension
```

### 2. Клонирование проекта

```bash
git clone <repository-url>
cd esp32-weather-station
```

### 3. Настройка WiFi

Отредактируйте файл `src/config.h`:

```cpp
inline const char* WIFI_SSID = "YOUR_WIFI_SSID";     // Ваш WiFi SSID
inline const char* WIFI_PASSWORD = "YOUR_PASSWORD";   // Ваш WiFi пароль
```

### 4. Загрузка прошивки

```bash
# В PlatformIO
pio run --target upload

# Или используйте кнопку "Upload" в VSCode
```

### 5. Просмотр Serial Monitor

```bash
pio device monitor --baud 115200

# Или используйте встроенный Serial Monitor в VSCode
```

## 📊 Структура проекта

```
esp32-weather-station/
├── src/
│   ├── main.cpp              # Основной файл программы
│   ├── config.h              # Конфигурация (WiFi, пины, настройки)
│   ├── sensor_manager.h/cpp  # Управление датчиком AHT10
│   ├── wifi_manager.h/cpp    # Управление WiFi подключением
│   ├── web_server.h/cpp      # Веб-сервер и API
│   ├── calculations.h/cpp    # Расчеты (точка росы, тепловой индекс)
│   └── html_pages.h          # HTML интерфейс
├── platformio.ini            # Конфигурация PlatformIO
└── README.md                 # Эта документация
```

## 🌐 API Endpoints

### GET /
Главная страница веб-интерфейса

### GET /data
```json
{
  "temperature": 23.5,
  "humidity": 45.2,
  "minTemp": 22.1,
  "maxTemp": 24.8,
  "minHumid": 42.0,
  "maxHumid": 48.5,
  "avgTemp": 23.2,
  "avgHumid": 44.8,
  "dewPoint": 10.5,
  "heatIndex": 23.5,
  "timestamp": 123456
}
```

### GET /stats
```json
{
  "uptime": "01:23:45",
  "freeHeap": "245.5 KB",
  "heapUsage": "23.5%",
  "cpuUsage": "12.3",
  "ssid": "MyWiFi",
  "rssi": "-67",
  "ip": "192.168.1.100",
  "requests": 1234,
  "errors": 0
}
```

### GET /history
Возвращает массивы данных для графиков (60 точек)

### GET /reset
Сброс min/max значений

### GET /reboot
Перезагрузка устройства

## ⚙️ Настройки в config.h

### I2C пины
```cpp
inline constexpr int I2C_SDA = 8;  // GPIO для SDA
inline constexpr int I2C_SCL = 9;  // GPIO для SCL
```

### Интервалы обновления
```cpp
inline constexpr unsigned long SENSOR_INTERVAL = 3000;       // 3 сек
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 30000;  // 30 сек
```

### История
```cpp
inline constexpr int HISTORY_SIZE = 60;              // 3 минуты
inline constexpr int HOURLY_HISTORY_SIZE = 1200;     // 1 час
```

## 🐛 Решение проблем

### Датчик не обнаружен
1. Проверьте подключение проводов
2. Убедитесь, что используются правильные GPIO пины
3. Проверьте питание датчика (должно быть 3.3V)
4. Попробуйте другой датчик AHT10

### Не подключается к WiFi
1. Проверьте правильность SSID и пароля
2. Убедитесь, что WiFi сеть работает на 2.4 GHz
3. Проверьте расстояние до роутера
4. Посмотрите логи в Serial Monitor

### Веб-интерфейс не открывается
1. Проверьте, что ESP32 подключен к WiFi
2. Найдите IP адрес в Serial Monitor
3. Убедитесь, что вы в той же сети
4. Попробуйте перезагрузить устройство

### Высокое использование CPU
1. Увеличьте `SENSOR_INTERVAL` в config.h
2. Уменьшите размер истории
3. Отключите ненужные функции

## 📈 Производительность

- **Использование RAM:** ~200 KB
- **Использование CPU:** 10-15%
- **Потребление энергии:** ~80 mA (WiFi активен)
- **Время отклика API:** <100ms
- **Частота обновления:** 3 секунды

## 🔮 Планы развития

- [ ] MQTT поддержка
- [ ] Интеграция с Home Assistant
- [ ] Поддержка нескольких датчиков
- [ ] OTA обновления
- [ ] Сохранение данных в SPIFFS
- [ ] Telegram бот для уведомлений
- [ ] Графики на больший период времени

## 📝 Changelog

### v3.0 (2025-02-01)
- ✨ Полная переработка проекта
- 📊 Три отдельных графика
- 🎨 Новый современный UI
- 🔧 Улучшенная обработка ошибок
- 📱 Лучшая адаптивность
- ⚡ Оптимизация производительности
- 📖 Расширенная документация

### v2.0
- Базовая функциональность
- Один комбинированный график
- Простой веб-интерфейс

## 📄 Лицензия

MIT License - свободно используйте в своих проектах!

## 🤝 Вклад в проект

Буду рад pull requests! Для крупных изменений сначала откройте issue.

## 💬 Поддержка

Если у вас возникли вопросы или проблемы:
1. Проверьте раздел "Решение проблем"
2. Посмотрите существующие issues
3. Создайте новый issue с подробным описанием

## 🙏 Благодарности

- Adafruit за отличные библиотеки
- Espressif за ESP32
- Сообществу PlatformIO

---

**Сделано с ❤️ для DIY энтузиастов**
