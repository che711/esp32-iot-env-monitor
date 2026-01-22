#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Метеостанция</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            background: white;
            border-radius: 20px;
            padding: 30px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
        }
        
        .header h1 {
            color: #333;
            font-size: 32px;
            margin-bottom: 10px;
        }
        
        .header .subtitle {
            color: #666;
            font-size: 14px;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        
        .sensor-card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            transition: transform 0.3s ease;
        }
        
        .sensor-card:hover {
            transform: translateY(-5px);
        }
        
        .humidity-card {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
        }
        
        .sensor-label {
            font-size: 14px;
            opacity: 0.9;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 10px;
        }
        
        .sensor-value {
            font-size: 48px;
            font-weight: bold;
            text-align: center;
        }
        
        .sensor-unit {
            font-size: 24px;
            opacity: 0.8;
        }
        
        .minmax {
            display: flex;
            justify-content: space-around;
            align-items: center;
            margin-top: 15px;
            font-size: 11px;
            opacity: 0.9;
        }
        
        .minmax .avg-value {
            font-size: 22px;
            font-weight: bold;
            opacity: 1;
            padding: 5px 10px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 8px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-top: 10px;
        }
        
        .info-item {
            background: #f8f9fa;
            padding: 10px;
            border-radius: 8px;
            font-size: 13px;
        }
        
        .info-label {
            color: #666;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .info-value {
            color: #333;
            font-weight: bold;
            font-size: 16px;
            margin-top: 5px;
        }
        
        .status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
        }
        
        .status.online {
            background: #d4edda;
            color: #155724;
        }
        
        .status.offline {
            background: #f8d7da;
            color: #721c24;
        }
        
        .chart-card {
            grid-column: 1 / -1;
        }
        
        canvas {
            max-height: 300px;
        }
        
        .buttons {
            display: flex;
            gap: 10px;
            margin-top: 15px;
            flex-wrap: wrap;
        }
        
        .btn {
            padding: 10px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: bold;
        }
        
        .btn-primary {
            background: #667eea;
            color: white;
        }
        
        .btn-primary:hover {
            background: #5568d3;
            transform: translateY(-2px);
        }
        
        .btn-danger {
            background: #dc3545;
            color: white;
        }
        
        .btn-danger:hover {
            background: #c82333;
            transform: translateY(-2px);
        }
        
        .btn-success {
            background: #28a745;
            color: white;
        }
        
        .btn-success:hover {
            background: #218838;
            transform: translateY(-2px);
        }
        
        .wifi-signal {
            display: inline-block;
            vertical-align: middle;
        }
        
        .update-time {
            text-align: center;
            color: #999;
            font-size: 12px;
            margin-top: 15px;
        }
        
        .temp-unit-toggle {
            margin-top: 15px;
            text-align: center;
        }
        
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 30px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 30px;
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 22px;
            width: 22px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        input:checked + .slider {
            background-color: #667eea;
        }
        
        input:checked + .slider:before {
            transform: translateX(30px);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌡️ Метеостанция ESP32</h1>
            <div class="subtitle">Мониторинг температуры и влажности</div>
            <div style="margin-top: 15px;">
                <span id="statusBadge" class="status online">🟢 Подключено</span>
            </div>
        </div>
        
        <div class="grid">
            <div class="card sensor-card">
                <div class="sensor-label">🌡️ Температура</div>
                <div class="sensor-value">
                    <span id="temperature">--</span>
                    <span class="sensor-unit" id="tempUnit">°C</span>
                </div>
                <div class="minmax">
                    <div>⬇️ Min: <span id="minTemp">--</span><span id="minTempUnit">°C</span></div>
                    <div class="avg-value">📊 <span id="avgTemp">--</span><span id="avgTempUnit">°C</span></div>
                    <div>⬆️ Max: <span id="maxTemp">--</span><span id="maxTempUnit">°C</span></div>
                </div>
                <div class="temp-unit-toggle">
                    <label style="color: white; margin-right: 10px;">°C</label>
                    <label class="toggle-switch">
                        <input type="checkbox" id="unitToggle" onchange="toggleUnit()">
                        <span class="slider"></span>
                    </label>
                    <label style="color: white; margin-left: 10px;">°F</label>
                </div>
            </div>
            
            <div class="card sensor-card humidity-card">
                <div class="sensor-label">💧 Влажность</div>
                <div class="sensor-value">
                    <span id="humidity">--</span>
                    <span class="sensor-unit">%</span>
                </div>
                <div class="minmax">
                    <div>⬇️ Min: <span id="minHumid">--</span>%</div>
                    <div class="avg-value">📊 <span id="avgHumid">--</span>%</div>
                    <div>⬆️ Max: <span id="maxHumid">--</span>%</div>
                </div>
            </div>
            
            <div class="card">
                <h3 style="margin-bottom: 15px; color: #333;">📊 Расчётные данные</h3>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Точка росы</div>
                        <div class="info-value"><span id="dewPoint">--</span> <span id="dewPointUnit">°C</span></div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Теплоощущение</div>
                        <div class="info-value"><span id="heatIndex">--</span> <span id="heatIndexUnit">°C</span></div>
                    </div>
                </div>
            </div>
            
            <div class="card sensor-card" style="background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);">
                <div class="sensor-label">💧 Точка росы</div>
                <div class="sensor-value">
                    <span id="dewPoint2">--</span>
                    <span class="sensor-unit" id="dewPointUnit2">°C</span>
                </div>
                <div class="sensor-description" style="margin-top: 10px; font-size: 11px; opacity: 0.85; text-align: center;">
                    Температура конденсации влаги
                </div>
            </div>
            
            <div class="card sensor-card" style="background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);">
                <div class="sensor-label">🌡️ Теплоощущение</div>
                <div class="sensor-value">
                    <span id="heatIndex2">--</span>
                    <span class="sensor-unit" id="heatIndexUnit2">°C</span>
                </div>
                <div class="sensor-description" style="margin-top: 10px; font-size: 11px; opacity: 0.85; text-align: center;">
                    Ощущаемая температура с учётом влажности
                </div>
            </div>
            
            <div class="card">
                <h3 style="margin-bottom: 15px; color: #333;">⚙️ Система</h3>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Время работы</div>
                        <div class="info-value" id="uptime">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Свободно RAM</div>
                        <div class="info-value" id="freeHeap">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Использование CPU</div>
                        <div class="info-value" id="cpuUsage">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">WiFi канал</div>
                        <div class="info-value" id="wifiChannel">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">WiFi SSID</div>
                        <div class="info-value" id="ssid">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">
                            <span class="wifi-signal" id="wifiSignal">📶</span> RSSI
                        </div>
                        <div class="info-value" id="rssi">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">IP адрес</div>
                        <div class="info-value" id="ipAddr" style="font-size: 12px;">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Переподключений</div>
                        <div class="info-value" id="reconnects">--</div>
                    </div>
                </div>
                
                <div class="buttons">
                    <button class="btn btn-primary" onclick="exportCSV()">📥 Экспорт CSV</button>
                    <button class="btn btn-success" onclick="resetMinMax()">🔄 Сброс Min/Max</button>
                    <button class="btn btn-danger" onclick="rebootDevice()">⚡ Перезагрузка</button>
                </div>
            </div>
            
            <div class="card chart-card">
                <h3 style="margin-bottom: 15px; color: #333;">📈 История (последние 3 минуты)</h3>
                <canvas id="historyChart"></canvas>
                <div class="update-time">
                    Обновлено: <span id="updateTime">--</span>
                </div>
            </div>
        </div>
    </div>

    <script>
        let isFahrenheit = false;
        let chartData = {
            labels: [],
            temp: [],
            humid: []
        };
        let chart;
        
        const ctx = document.getElementById('historyChart').getContext('2d');
        chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: chartData.labels,
                datasets: [{
                    label: 'Температура (°C)',
                    data: chartData.temp,
                    borderColor: '#667eea',
                    backgroundColor: 'rgba(102, 126, 234, 0.1)',
                    tension: 0.4
                }, {
                    label: 'Влажность (%)',
                    data: chartData.humid,
                    borderColor: '#4facfe',
                    backgroundColor: 'rgba(79, 172, 254, 0.1)',
                    tension: 0.4,
                    yAxisID: 'y1'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                interaction: {
                    mode: 'index',
                    intersect: false,
                },
                scales: {
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Температура (°C)'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Влажность (%)'
                        },
                        grid: {
                            drawOnChartArea: false,
                        },
                    },
                }
            }
        });
        
        function celsiusToFahrenheit(c) {
            return (c * 9/5) + 32;
        }
        
        function toggleUnit() {
            isFahrenheit = !isFahrenheit;
            updateDisplay();
        }
        
        function updateDisplay() {
            const units = document.querySelectorAll('#tempUnit, #minTempUnit, #maxTempUnit, #avgTempUnit, #dewPointUnit, #dewPointUnit2, #heatIndexUnit, #heatIndexUnit2');
            units.forEach(el => el.textContent = isFahrenheit ? '°F' : '°C');
            updateData();
        }
        
        let errorCount = 0;
        
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    if (data.error) throw new Error(data.error);
                    
                    const temp = isFahrenheit ? celsiusToFahrenheit(data.temperature) : data.temperature;
                    const minT = isFahrenheit ? celsiusToFahrenheit(data.minTemp) : data.minTemp;
                    const maxT = isFahrenheit ? celsiusToFahrenheit(data.maxTemp) : data.maxTemp;
                    const avgT = isFahrenheit ? celsiusToFahrenheit(data.avgTemp) : data.avgTemp;
                    const dewP = isFahrenheit ? celsiusToFahrenheit(data.dewPoint) : data.dewPoint;
                    const heatI = isFahrenheit ? celsiusToFahrenheit(data.heatIndex) : data.heatIndex;
                    
                    document.getElementById('temperature').textContent = temp.toFixed(1);
                    document.getElementById('humidity').textContent = data.humidity.toFixed(1);
                    document.getElementById('minTemp').textContent = minT.toFixed(1);
                    document.getElementById('maxTemp').textContent = maxT.toFixed(1);
                    document.getElementById('avgTemp').textContent = avgT.toFixed(1);
                    document.getElementById('minHumid').textContent = data.minHumid.toFixed(1);
                    document.getElementById('maxHumid').textContent = data.maxHumid.toFixed(1);
                    document.getElementById('avgHumid').textContent = data.avgHumid.toFixed(1);
                    document.getElementById('dewPoint').textContent = dewP.toFixed(1);
                    document.getElementById('dewPoint2').textContent = dewP.toFixed(1);
                    document.getElementById('heatIndex').textContent = heatI.toFixed(1);
                    document.getElementById('heatIndex2').textContent = heatI.toFixed(1);
                    
                    const now = new Date();
                    document.getElementById('updateTime').textContent = now.toLocaleTimeString('ru-RU');
                    
                    errorCount = 0;
                    document.getElementById('statusBadge').className = 'status online';
                    document.getElementById('statusBadge').innerHTML = '🟢 Подключено';
                })
                .catch(error => {
                    console.error('Ошибка:', error);
                    errorCount++;
                    if (errorCount > 2) {
                        document.getElementById('statusBadge').className = 'status offline';
                        document.getElementById('statusBadge').innerHTML = '🔴 Ошибка';
                    }
                });
        }
        
        function updateStats() {
            fetch('/stats')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('uptime').textContent = data.uptime;
                    document.getElementById('freeHeap').textContent = data.freeHeap;
                    document.getElementById('cpuUsage').textContent = data.cpuUsage + '%';
                    document.getElementById('wifiChannel').textContent = data.wifiChannel;
                    document.getElementById('ssid').textContent = data.ssid;
                    document.getElementById('rssi').textContent = data.rssi + ' dBm';
                    document.getElementById('ipAddr').textContent = data.ip;
                    document.getElementById('reconnects').textContent = data.reconnects;
                    
                    const rssi = parseInt(data.rssi);
                    let signal = '📶';
                    if (rssi > -50) signal = '📶';
                    else if (rssi > -60) signal = '📶';
                    else if (rssi > -70) signal = '📡';
                    else signal = '📉';
                    document.getElementById('wifiSignal').textContent = signal;
                });
        }
        
        function updateHistory() {
            fetch('/history')
                .then(response => response.json())
                .then(data => {
                    chartData.labels = data.labels;
                    chartData.temp = data.temp;
                    chartData.humid = data.humid;
                    
                    chart.data.labels = chartData.labels;
                    chart.data.datasets[0].data = chartData.temp;
                    chart.data.datasets[1].data = chartData.humid;
                    chart.update('none');
                });
        }
        
        function resetMinMax() {
            if (confirm('Сбросить минимальные и максимальные значения?')) {
                fetch('/reset')
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message || 'Min/Max сброшены');
                        updateData();
                    });
            }
        }
        
        function rebootDevice() {
            if (confirm('⚠️ ВНИМАНИЕ! Устройство будет перезагружено. Продолжить?')) {
                fetch('/reboot')
                    .then(() => {
                        alert('Устройство перезагружается... Подождите 10 секунд.');
                        setTimeout(() => location.reload(), 10000);
                    });
            }
        }
        
        function exportCSV() {
            let csv = 'Время,Температура (°C),Влажность (%)\n';
            for (let i = 0; i < chartData.labels.length; i++) {
                csv += `${chartData.labels[i]},${chartData.temp[i]},${chartData.humid[i]}\n`;
            }
            
            const blob = new Blob([csv], { type: 'text/csv' });
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `esp32_data_${new Date().toISOString().slice(0,10)}.csv`;
            a.click();
        }
        
        updateData();
        updateStats();
        updateHistory();
        
        setInterval(updateData, 3000);
        setInterval(updateStats, 5000);
        setInterval(updateHistory, 3000);
    </script>
</body>
</html>
)rawliteral";

#endif // HTML_PAGES_H
