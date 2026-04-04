# 🌡️ ESP32 Super Mini Weather Station

A modern weather station based on the **ESP32 Super Mini** with the **AHT10** sensor for real-time temperature and humidity monitoring.

# TODO: combine QUICK_START.md and README.md

![Version](https://img.shields.io/badge/version-3.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-green)
![License](https://img.shields.io/badge/license-MIT-yellow)


##  Scope

###  Functions
-  Real-time temperature and humidity monitoring
-  Four interactive graphs (temperature, humidity, perceived temperature, and felt temperature)
-  Dew Point
-  Heat Index
-  History data
-  Auto update
-  Adaptive web UI for all devices
-  WiFi with automatic reconnection
-  Exporting data to CSV and JSON
  
### 🎨 Web UI
- Modern gradient design
- Comfort indicators for temperature and humidity
- Displaying min/max/avg temperature and humidity values
- System info (uptime, RAM, CPU, WiFi)
- Device management (reset, reboot)
- Switch °C / °F


## 🔧 Компоненты

### Equipment
- **ESP32 Super Mini** - Microcontroller
- **AHT10** - temperature and humidity sensor (I2C)
- Wires, a soldering iron, solder, rosin, and other components for assembly
  
### Software
- **PlatformIO** or **Arduino IDE**
- **Libraries:**
  - Adafruit_AHTX0
  - WebServer (built into ESP32)
  - WiFi (built into ESP32)


##  Connection diagram

```
ESP32 Super Mini → AHT10
─────────────────────────
3.3V              → VCC
GND               → GND
GPIO 8            → SDA
GPIO 9            → SCL
```

**Important:** ESP32 Super Mini works on 3.3V! Do not connect 5V to the sensor!



##  Installation and configuration

### 1. Installing PlatformIO

```bash
# Install Visual Studio Code
# Install the PlatformIO IDE extension
```

### 2. Clone the project

```bash
git clone git@github.com:che711/esp32-iot-env-monitor.git
cd esp32-iot-env-monitor
```

### 3. Setting WI-FI up

Edit the file `src/config.h`:

```cpp
inline const char* WIFI_SSID = "YOUR_WIFI_SSID";     // your WiFi SSID
inline const char* WIFI_PASSWORD = "YOUR_PASSWORD";   // your WiFi пароль
```

### 4. Downloading the firmware

```bash
# In PlatformIO
pio run --target upload

# Use the button "Upload" in VSCode
```

### 5. Check Serial Monitor

```bash
pio device monitor --baud 115200

# Or use the built-in Serial Monitor in VSCode
```


##  Structure of the project

```
esp32-weather-station/
├── src/
│   ├── battery_manager.h/cpp     # Battery configuration files
│   ├── main.cpp                  # Main program file
│   ├── config.h                  # Configuration (WiFi, pins, settings)
│   ├── sensor_manager.h/cpp      # AHT10 sensor management
│   ├── wifi_manager.h/cpp        # WiFi connection management
│   ├── web_server.h/cpp          # Web server and API
│   ├── calculations.h/cpp        # Calculations (dew point, thermal index)
│   └── html_pages.h              # HTML interface
├── tests/
│   └── api
│     └── test_api.py             # in progress 
│     └── test_api.sh             # in progress 
│   └── web
│     └── test_web_ui.py          # in progress 
├── .gitignore                    # data for .gitignore
├── API_EXAMPLES.md               
├── LICENSE                       
├── platformio.ini                # PlatformIO configuration
└── README.md                     # This documentation
```



##  API Endpoints

### GET /
The main page of the web interface

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
Returns arrays of data for graphs (60 points)

### GET /reset
Reset min/max values

### GET /reboot
Device reboot

##  Settings in config.h

### I2C pins
```cpp
inline constexpr int I2C_SDA = 8;  // GPIO for SDA
inline constexpr int I2C_SCL = 9;  // GPIO for SCL
```

### Update intervals
```cpp
inline constexpr unsigned long SENSOR_INTERVAL = 3000;       // 3 sec
inline constexpr unsigned long WIFI_CHECK_INTERVAL = 30000;  // 30 sec
```

### history
```cpp
inline constexpr int HISTORY_SIZE = 60;              // 3 mins
inline constexpr int HOURLY_HISTORY_SIZE = 1200;     // 1 hour
```


##  Resolve issues

### The sensor is not detected
1. Check the wiring connection
2. Make sure that the correct GPIO pins are used.
3. Check the sensor's power supply (it should be 3.3V)
4. Try another AHT10 sensor

### Does not connect to WiFi
1. Verify that the SSID and password are correct
2. Make sure that the WiFi network is running on 2.4 GHz
3. Check the distance to the router
4. Look at the logs in Serial Monitor

### The web interface does not open
1. Check that the ESP32 is connected to WiFi
2. Find the IP address in the Serial Monitor
3. Make sure you are on the same network.
4. Try restarting your device

### High CPU usage
1. Increase the `SENSOR_INTERVAL` in config.h
2. Reduce the size of the history
3. Disable unnecessary functions


##  Efficiency

- **RAM Usage:** ~200 KB
- **CPU Usage:** 10-15%
- **Energy consumption:** ~80 mA (WiFi is active)
- **API response time:** <100ms
- **Refresh rate:** 3 seconds

##  Development plans

- [ ] MQTT support
- [ ] Integration with the Home Assistant
- [ ] Support for multiple sensors
- [ ] OTA updates
- [ ] Saving data to SPIFFS
- [ ] Telegram bot for notifications
- [ ] Charts for a longer period of time


##  Changelog

### v3.0 (2025-02-01)
-  Reworking the project
-  Three separate charts
-  New modern UI
-  Improved error handling
-  Better adaptability
-  Performance optimization
-  Extended documentation

### v2.0
- Basic functionality
- One combined schedule
- Simple web interface


##  License

MIT License - use it freely in your projects!


##  Support

If you have any questions or concerns:
1. Check the "Problem Solving" section
2. Look at existing issues
3. Create a new issue with a detailed description

##  Thanks

- Adafruit for excellent libraries
- Espressif for ESP32
- To the PlatformIO Community
---

**Made with ❤️ for DIY enthusiasts**
