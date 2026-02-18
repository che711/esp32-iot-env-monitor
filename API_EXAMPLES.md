# API Usage Examples

## Базовые примеры использования API

### JavaScript (Fetch API)

```javascript
// Получение текущих данных
async function getCurrentData() {
    try {
        const response = await fetch('http://192.168.1.100/data');
        const data = await response.json();
        
        console.log(`Температура: ${data.temperature}°C`);
        console.log(`Влажность: ${data.humidity}%`);
        console.log(`Точка росы: ${data.dewPoint}°C`);
        
        return data;
    } catch (error) {
        console.error('Ошибка:', error);
    }
}

// Получение статистики системы
async function getSystemStats() {
    try {
        const response = await fetch('http://192.168.1.100/stats');
        const stats = await response.json();
        
        console.log(`Uptime: ${stats.uptime}`);
        console.log(`Free Heap: ${stats.freeHeap}`);
        console.log(`CPU Usage: ${stats.cpuUsage}%`);
        
        return stats;
    } catch (error) {
        console.error('Ошибка:', error);
    }
}

// Получение истории
async function getHistory() {
    try {
        const response = await fetch('http://192.168.1.100/history');
        const history = await response.json();
        
        console.log('Точек данных:', history.labels.length);
        console.log('Температура:', history.temp);
        console.log('Влажность:', history.humid);
        
        return history;
    } catch (error) {
        console.error('Ошибка:', error);
    }
}

// Автоматическое обновление каждые 10 секунд
setInterval(async () => {
    const data = await getCurrentData();
    // Обновить UI
}, 3000);
```

### Python (Requests)

```python
import requests
import time

# Адрес устройства
BASE_URL = 'http://192.168.1.100'

def get_data():
    """Получить текущие данные датчика"""
    try:
        response = requests.get(f'{BASE_URL}/data')
        response.raise_for_status()
        data = response.json()
        
        print(f"Температура: {data['temperature']}°C")
        print(f"Влажность: {data['humidity']}%")
        print(f"Точка росы: {data['dewPoint']}°C")
        
        return data
    except requests.exceptions.RequestException as e:
        print(f"Ошибка: {e}")
        return None

def get_stats():
    """Получить статистику системы"""
    try:
        response = requests.get(f'{BASE_URL}/stats')
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Ошибка: {e}")
        return None

def reset_minmax():
    """Сбросить min/max значения"""
    try:
        response = requests.get(f'{BASE_URL}/reset')
        response.raise_for_status()
        result = response.json()
        print(result['message'])
        return True
    except requests.exceptions.RequestException as e:
        print(f"Ошибка: {e}")
        return False

def monitor_continuously(interval=3):
    """Непрерывный мониторинг"""
    print("Начало мониторинга (Ctrl+C для остановки)")
    try:
        while True:
            data = get_data()
            if data:
                print("-" * 40)
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nМониторинг остановлен")

# Пример использования
if __name__ == '__main__':
    # Получить текущие данные
    data = get_data()
    
    # Или запустить непрерывный мониторинг
    # monitor_continuously(interval=5)
```

### Node.js (Axios)

```javascript
const axios = require('axios');

const BASE_URL = 'http://192.168.1.100';

// Получение данных
async function getData() {
    try {
        const response = await axios.get(`${BASE_URL}/data`);
        console.log('Температура:', response.data.temperature);
        console.log('Влажность:', response.data.humidity);
        return response.data;
    } catch (error) {
        console.error('Ошибка:', error.message);
    }
}

// Логирование в файл
const fs = require('fs');

async function logToFile() {
    const data = await getData();
    if (data) {
        const timestamp = new Date().toISOString();
        const logEntry = `${timestamp},${data.temperature},${data.humidity}\n`;
        fs.appendFileSync('weather_log.csv', logEntry);
    }
}

// Логирование каждые 5 минут
setInterval(logToFile, 5 * 60 * 1000);
```

### curl (Command Line)

```bash
# Получить текущие данные
curl http://192.168.1.65/data | jq

# Получить статистику
curl http://192.168.1.65/stats | jq

# Получить историю
curl http://192.168.1.65/history | jq

# Сбросить min/max
curl http://192.168.1.65/reset

# Перезагрузить устройство
curl http://192.168.1.65/reboot

# Сохранить данные в файл
curl http://192.168.1.65/data > weather_data.json

# Непрерывный мониторинг
watch -n 3 'curl -s http://192.168.1.65/data | jq ".temperature, .humidity"'
```

## Интеграция с Home Assistant

```yaml
# configuration.yaml

sensor:
  - platform: rest
    name: "ESP32 Temperature"
    resource: http://192.168.1.100/data
    value_template: "{{ value_json.temperature }}"
    unit_of_measurement: "°C"
    scan_interval: 30
    
  - platform: rest
    name: "ESP32 Humidity"
    resource: http://192.168.1.100/data
    value_template: "{{ value_json.humidity }}"
    unit_of_measurement: "%"
    scan_interval: 30
    
  - platform: rest
    name: "ESP32 Dew Point"
    resource: http://192.168.1.100/data
    value_template: "{{ value_json.dewPoint }}"
    unit_of_measurement: "°C"
    scan_interval: 30
```

## Интеграция с Grafana

```python
# Python скрипт для отправки данных в InfluxDB
import requests
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import time

# Настройки InfluxDB
INFLUXDB_URL = "http://localhost:8086"
INFLUXDB_TOKEN = "your-token"
INFLUXDB_ORG = "your-org"
INFLUXDB_BUCKET = "weather"

# Настройки ESP32
ESP32_URL = "http://192.168.1.100/data"

# Инициализация клиента
client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN, org=INFLUXDB_ORG)
write_api = client.write_api(write_options=SYNCHRONOUS)

def collect_and_send():
    try:
        # Получить данные с ESP32
        response = requests.get(ESP32_URL)
        data = response.json()
        
        # Создать точку данных
        point = Point("weather") \
            .tag("location", "room") \
            .field("temperature", float(data['temperature'])) \
            .field("humidity", float(data['humidity'])) \
            .field("dewPoint", float(data['dewPoint'])) \
            .field("heatIndex", float(data['heatIndex']))
        
        # Отправить в InfluxDB
        write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point)
        print(f"Данные отправлены: {data['temperature']}°C, {data['humidity']}%")
        
    except Exception as e:
        print(f"Ошибка: {e}")

# Сбор данных каждые 30 секунд
while True:
    collect_and_send()
    time.sleep(30)
```

## Создание простого дашборда на HTML

```html
<!DOCTYPE html>
<html>
<head>
    <title>Weather Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
    <style>
        body { font-family: Arial, sans-serif; padding: 20px; }
        .card { border: 1px solid #ddd; padding: 20px; margin: 10px; border-radius: 8px; }
        .value { font-size: 48px; font-weight: bold; color: #667eea; }
    </style>
</head>
<body>
    <h1>ESP32 Weather Station</h1>
    
    <div class="card">
        <h2>Current Data</h2>
        <div>Temperature: <span class="value" id="temp">--</span>°C</div>
        <div>Humidity: <span class="value" id="humid">--</span>%</div>
    </div>
    
    <div class="card">
        <canvas id="chart"></canvas>
    </div>
    
    <script>
        const ESP32_URL = 'http://192.168.1.100';
        
        async function updateData() {
            try {
                const response = await fetch(`${ESP32_URL}/data`);
                const data = await response.json();
                
                document.getElementById('temp').textContent = data.temperature.toFixed(1);
                document.getElementById('humid').textContent = data.humidity.toFixed(1);
            } catch (error) {
                console.error('Error:', error);
            }
        }
        
        // Обновление каждые 3 секунды
        setInterval(updateData, 3000);
        updateData();
    </script>
</body>
</html>
```

## Telegram Bot Integration

```python
import requests
import telebot
from telebot import types

BOT_TOKEN = 'YOUR_BOT_TOKEN'
ESP32_URL = 'http://192.168.1.100'

bot = telebot.TeleBot(BOT_TOKEN)

@bot.message_handler(commands=['start'])
def send_welcome(message):
    bot.reply_to(message, "Привет! Я бот метеостанции. Используй /weather для получения данных.")

@bot.message_handler(commands=['weather'])
def send_weather(message):
    try:
        response = requests.get(f'{ESP32_URL}/data')
        data = response.json()
        
        text = f"""
🌡️ Температура: {data['temperature']}°C
💧 Влажность: {data['humidity']}%
💎 Точка росы: {data['dewPoint']}°C
🔥 Ощущается как: {data['heatIndex']}°C

📊 Min/Max:
  T: {data['minTemp']}°C / {data['maxTemp']}°C
  H: {data['minHumid']}% / {data['maxHumid']}%
        """
        
        bot.reply_to(message, text)
    except Exception as e:
        bot.reply_to(message, f"Ошибка: {e}")

bot.polling()
```
