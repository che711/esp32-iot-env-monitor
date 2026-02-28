#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Env_Stats</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
<style>
/* ── CSS variables (light / dark) ──────────────────────────── */
:root{
  --bg-page:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  --bg-card:rgba(255,255,255,.95);
  --bg-info:#f8f9fa;
  --text-main:#333;
  --text-sub:#666;
  --text-muted:#999;
  --chart-grid:rgba(0,0,0,.05);
  --toggle-bg:#f0f0f0;
  --toggle-color:#666;
  --update-bg:#f8f9fa;
  --info-border:#667eea;
  --shadow:0 4px 16px rgba(0,0,0,.15);
  --shadow-hover:0 10px 30px rgba(0,0,0,.2);
}
body.dark{
  --bg-page:linear-gradient(135deg,#1a1a2e 0%,#16213e 50%,#0f3460 100%);
  --bg-card:rgba(30,34,54,.97);
  --bg-info:#252840;
  --text-main:#e8eaf6;
  --text-sub:#9fa8da;
  --text-muted:#5c6bc0;
  --chart-grid:rgba(255,255,255,.06);
  --toggle-bg:#252840;
  --toggle-color:#9fa8da;
  --update-bg:#252840;
  --info-border:#5c6bc0;
  --shadow:0 4px 16px rgba(0,0,0,.4);
  --shadow-hover:0 10px 30px rgba(0,0,0,.5);
}

*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:var(--bg-page);min-height:100vh;padding:20px;transition:background .4s}
.container{max-width:700px;margin:0 auto}

/* ── Header ──────────────────────────────────────────────────── */
.header{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:20px;padding:30px;margin-bottom:20px;box-shadow:0 10px 30px rgba(0,0,0,.2);text-align:center;transition:background .3s}
.header h1{color:var(--text-main);font-size:clamp(24px,5vw,36px);margin-bottom:10px;font-weight:700;transition:color .3s}
.subtitle{color:var(--text-sub);font-size:14px;margin-bottom:15px;transition:color .3s}
.status-container{display:flex;gap:10px;justify-content:center;flex-wrap:wrap}
.status{display:inline-flex;align-items:center;gap:8px;padding:8px 20px;border-radius:25px;font-size:13px;font-weight:600}
.status.online{background:#d4edda;color:#155724}
.status.offline{background:#f8d7da;color:#721c24}
body.dark .status.online{background:rgba(40,167,69,.2);color:#69f0ae}
body.dark .status.offline{background:rgba(220,53,69,.2);color:#ff5252}
body.dark #lastUpdateBadge{background:rgba(25,118,210,.2)!important;color:#90caf9!important}
.status-dot{width:8px;height:8px;border-radius:50%;animation:pulse 2s infinite}
.status.online .status-dot{background:#28a745}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}

/* ── Dark theme toggle ────────────────────────────────────────── */
.theme-toggle{position:fixed;top:18px;right:18px;z-index:999;background:var(--bg-card);border:none;border-radius:50%;width:46px;height:46px;font-size:20px;cursor:pointer;box-shadow:var(--shadow);transition:all .3s;display:flex;align-items:center;justify-content:center}
.theme-toggle:hover{transform:scale(1.12) rotate(20deg);box-shadow:var(--shadow-hover)}

/* ── Cards ────────────────────────────────────────────────────── */
.card{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:var(--shadow);transition:background .3s,transform .3s,box-shadow .3s}
.card:hover{transform:translateY(-3px);box-shadow:var(--shadow-hover)}
.card h3{color:var(--text-main);transition:color .3s}
.sensor-card{color:#fff;position:relative;overflow:hidden}
.sensor-card::before{content:'';position:absolute;top:-50%;right:-50%;width:200%;height:200%;background:radial-gradient(circle,rgba(255,255,255,.1) 0%,transparent 70%);pointer-events:none}
.temp-card    {background:linear-gradient(135deg,#667eea 0%,#764ba2 100%)}
.humidity-card{background:linear-gradient(135deg,#4facfe 0%,#00f2fe 100%)}
.dewpoint-card{background:linear-gradient(135deg,#f093fb 0%,#f5576c 100%)}
.heatindex-card{background:linear-gradient(135deg,#fa709a 0%,#fee140 100%)}
.sensor-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:15px}
.sensor-label{font-size:14px;text-transform:uppercase;letter-spacing:1.5px;font-weight:600}
.sensor-value{font-size:clamp(40px,8vw,52px);font-weight:700;text-align:center;margin:15px 0}
.sensor-unit{font-size:.5em;opacity:.85}
.sensor-description{margin-top:15px;font-size:12px;opacity:.9;text-align:center;padding:10px;background:rgba(255,255,255,.1);border-radius:8px}
.minmax{display:grid;grid-template-columns:1fr auto 1fr;gap:10px;align-items:center;margin-top:15px;font-size:11px;padding:15px;background:rgba(255,255,255,.15);border-radius:8px}
.minmax>div{display:flex;flex-direction:column;align-items:center;gap:5px}
.minmax-value{font-size:16px;font-weight:700}
.avg-value{font-size:20px;font-weight:700;padding:10px 15px;background:rgba(255,255,255,.25);border-radius:8px}

/* ── Info grid ────────────────────────────────────────────────── */
.info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px;margin-top:15px}
.info-item{background:var(--bg-info);padding:15px;border-radius:8px;border-left:4px solid var(--info-border);transition:background .3s}
.info-label{color:var(--text-sub);font-size:11px;text-transform:uppercase;margin-bottom:8px;transition:color .3s}
.info-value{color:var(--text-main);font-weight:700;font-size:clamp(14px,3vw,18px);transition:color .3s}

/* ── Battery widget ───────────────────────────────────────────── */
.battery-widget{display:flex;align-items:center;gap:12px;margin-top:6px}
.battery-icon{position:relative;width:44px;height:22px;border:2.5px solid var(--text-main);border-radius:4px;flex-shrink:0;transition:border-color .3s}
.battery-icon::after{content:'';position:absolute;right:-7px;top:50%;transform:translateY(-50%);width:5px;height:11px;background:var(--text-main);border-radius:0 3px 3px 0;transition:background .3s}
.battery-fill{height:100%;border-radius:2px;transition:width .6s ease,background .4s ease}
.battery-fill.critical{background:#dc3545}
.battery-fill.low{background:#ff9800}
.battery-fill.mid{background:#ffc107}
.battery-fill.good{background:#28a745}
.battery-fill.charging{background:linear-gradient(90deg,#28a745,#00e676);animation:chargepulse 1.2s ease-in-out infinite}
@keyframes chargepulse{0%,100%{opacity:1}50%{opacity:.55}}
.battery-text{font-size:14px;font-weight:700;color:var(--text-main);transition:color .3s}
.battery-sub{font-size:11px;color:var(--text-sub);margin-top:2px;transition:color .3s}
.battery-voltage{font-weight:700;color:var(--text-main);transition:color .3s}

/* ── Charts ───────────────────────────────────────────────────── */
.chart-row{display:grid;grid-template-columns:1fr;gap:20px;margin-bottom:20px}
.chart-row.double{grid-template-columns:1fr 1fr}
.chart-card{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:var(--shadow);transition:background .3s,transform .3s,box-shadow .3s}
.chart-card:hover{transform:translateY(-3px);box-shadow:var(--shadow-hover)}
.chart-card h3{color:var(--text-main);transition:color .3s}
canvas{max-height:350px;min-height:250px;width:100%}

/* ── Buttons ──────────────────────────────────────────────────── */
.buttons{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:12px;margin-top:15px}
.btn{padding:12px 20px;border:none;border-radius:8px;font-size:14px;cursor:pointer;font-weight:600;display:flex;align-items:center;justify-content:center;gap:8px;transition:.3s;font-family:inherit}
.btn-primary{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff}
.btn-success{background:linear-gradient(135deg,#28a745 0%,#218838 100%);color:#fff}
.btn-danger{background:linear-gradient(135deg,#dc3545 0%,#c82333 100%);color:#fff}
.btn:hover{transform:translateY(-2px)}

/* ── °C/°F toggle ─────────────────────────────────────────────── */
.temp-unit-toggle{margin-top:15px;text-align:center;padding:15px;background:rgba(255,255,255,.1);border-radius:8px}
.toggle-label{color:#fff;font-weight:600;font-size:13px}
.toggle-switch{position:relative;display:inline-block;width:60px;height:30px;margin:0 12px}
.toggle-switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:rgba(255,255,255,.3);transition:.4s;border-radius:30px}
.slider:before{position:absolute;content:"";height:24px;width:24px;left:3px;bottom:3px;background:#fff;transition:.4s;border-radius:50%}
input:checked+.slider{background:rgba(255,255,255,.4)}
input:checked+.slider:before{transform:translateX(30px)}

/* ── Update time ──────────────────────────────────────────────── */
.update-time{text-align:center;color:var(--text-muted);font-size:12px;margin-top:15px;padding:10px;background:var(--update-bg);border-radius:8px;transition:background .3s,color .3s}

/* ── Comfort ──────────────────────────────────────────────────── */
.comfort-indicator{margin-top:10px;padding:8px 12px;border-radius:8px;font-size:11px;font-weight:600;text-align:center;background:rgba(255,255,255,.2)}
.comfort-excellent{background:rgba(40,167,69,.3)}
.comfort-good{background:rgba(255,193,7,.3)}
.comfort-poor{background:rgba(220,53,69,.3)}

/* ── Serial Monitor ───────────────────────────────────────────── */
.log-console{background:#1e1e1e;color:#d4d4d4;font-family:'Consolas','Monaco',monospace;font-size:14px;padding:18px;border-radius:8px;height:450px;overflow-y:auto;margin-top:12px}
.log-console::-webkit-scrollbar{width:8px}
.log-console::-webkit-scrollbar-track{background:#2d2d2d;border-radius:4px}
.log-console::-webkit-scrollbar-thumb{background:#555;border-radius:4px}
.log-console::-webkit-scrollbar-thumb:hover{background:#777}
.log-line{margin:2px 0;white-space:pre-wrap;word-break:break-all}
.log-line.hidden{display:none}
.log-error{color:#f48771}
.log-warning{color:#dcdcaa}
.log-info{color:#4ec9b0}
.log-success{color:#4fc1ff}
.ws-status{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:8px}
.ws-connected{background:#28a745}
.ws-disconnected{background:#dc3545}

/* Log filter bar */
.log-filters{display:flex;flex-wrap:wrap;gap:7px;margin-top:12px;align-items:center}
.log-filter-btn{padding:5px 14px;border:2px solid transparent;border-radius:20px;font-size:12px;font-weight:700;cursor:pointer;transition:all .2s;background:var(--toggle-bg);color:var(--toggle-color);font-family:inherit;opacity:.5}
.log-filter-btn.active{opacity:1;color:#1e1e1e}
.lf-error.active  {background:#f48771;border-color:#f48771;color:#1e1e1e}
.lf-warning.active{background:#dcdcaa;border-color:#dcdcaa;color:#1e1e1e}
.lf-info.active   {background:#4ec9b0;border-color:#4ec9b0;color:#1e1e1e}
.lf-success.active{background:#4fc1ff;border-color:#4fc1ff;color:#1e1e1e}
.log-filter-btn:hover{opacity:1;transform:translateY(-1px)}
.log-counter{font-size:11px;color:var(--text-muted);margin-left:auto;padding:5px 10px;background:var(--bg-info);border-radius:12px;transition:background .3s,color .3s}

/* ── Chart series toggles ─────────────────────────────────────── */
.chart-toggles{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:14px}
.chart-toggle-btn{display:flex;align-items:center;gap:7px;padding:8px 16px;border:2px solid transparent;border-radius:25px;font-size:13px;font-weight:600;cursor:pointer;transition:all .25s;background:var(--toggle-bg);color:var(--toggle-color);user-select:none;font-family:inherit}
.chart-toggle-btn .dot{width:10px;height:10px;border-radius:50%;flex-shrink:0;transition:transform .2s}
.chart-toggle-btn.active{color:#fff;transform:translateY(-1px);box-shadow:0 4px 12px rgba(0,0,0,.2)}
.chart-toggle-btn.active .dot{transform:scale(1.3)}
.chart-toggle-btn:not(.active){opacity:.45}
.chart-toggle-btn:hover{opacity:1;transform:translateY(-1px)}
.toggle-temp.active  {background:linear-gradient(135deg,#667eea,#764ba2);border-color:#667eea}
.toggle-humid.active {background:linear-gradient(135deg,#4facfe,#00f2fe);border-color:#4facfe}
.toggle-heat.active  {background:linear-gradient(135deg,#fa709a,#fee140);border-color:#fa709a}
.toggle-dew.active   {background:linear-gradient(135deg,#f093fb,#f5576c);border-color:#f093fb}
.toggle-temp  .dot{background:linear-gradient(135deg,#667eea,#764ba2)}
.toggle-humid .dot{background:linear-gradient(135deg,#4facfe,#00f2fe)}
.toggle-heat  .dot{background:linear-gradient(135deg,#fa709a,#fee140)}
.toggle-dew   .dot{background:linear-gradient(135deg,#f093fb,#f5576c)}

/* ── Time range buttons ───────────────────────────────────────── */
.time-range{display:flex;gap:6px}
.tr-btn{padding:6px 14px;border:2px solid var(--info-border);border-radius:20px;font-size:12px;font-weight:700;cursor:pointer;background:transparent;color:var(--info-border);transition:all .2s;font-family:inherit}
.tr-btn.active{background:var(--info-border);color:#fff}
.tr-btn:hover{transform:translateY(-1px)}

/* ── Responsive ───────────────────────────────────────────────── */
@media(max-width:768px){
body{padding:10px}
.header{padding:20px}
.card{padding:20px}
.info-grid{grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px}
.buttons{grid-template-columns:1fr}
.chart-row.double{grid-template-columns:1fr}
.chart-toggle-btn{padding:7px 12px;font-size:12px}
.theme-toggle{top:10px;right:10px;width:40px;height:40px;font-size:17px}
}
</style>
</head>
<body>

<!-- Dark/light theme toggle (fixed) -->
<button class="theme-toggle" id="themeBtn" onclick="toggleTheme()" title="Toggle dark mode">🌙</button>

<div class="container">

<!-- Header -->
<div class="header">
<h1>🌡️ Environmental Statistics</h1>
<div class="subtitle">Temperature and humidity monitoring</div>
<div class="status-container">
<div id="statusBadge" class="status online"><div class="status-dot"></div><span>Connected</span></div>
<div id="lastUpdateBadge" class="status" style="background:#e3f2fd;color:#1976d2"><span id="lastUpdate">Loading...</span></div>
</div>
</div>

<!-- Sensor tiles -->
<div class="chart-row double">
<div class="card sensor-card temp-card">
<div class="sensor-header"><div class="sensor-label">🌡️ Temperature</div></div>
<div class="sensor-value"><span id="temperature">--</span><span class="sensor-unit" id="tempUnit">°C</span></div>
<div class="minmax">
<div><div style="font-size:10px">⬇️ Min</div><div class="minmax-value"><span id="minTemp">--</span><span id="minTempUnit">°C</span></div></div>
<div class="avg-value">📊 <span id="avgTemp">--</span><span id="avgTempUnit">°C</span></div>
<div><div style="font-size:10px">⬆️ Max</div><div class="minmax-value"><span id="maxTemp">--</span><span id="maxTempUnit">°C</span></div></div>
</div>
<div class="temp-unit-toggle">
<span class="toggle-label">°C</span>
<label class="toggle-switch"><input type="checkbox" id="unitToggle" onchange="toggleUnit()"><span class="slider"></span></label>
<span class="toggle-label">°F</span>
</div>
<div id="tempComfort" class="comfort-indicator"></div>
</div>
<div class="card sensor-card humidity-card">
<div class="sensor-header"><div class="sensor-label">💧 Humidity</div></div>
<div class="sensor-value"><span id="humidity">--</span><span class="sensor-unit">%</span></div>
<div class="minmax">
<div><div style="font-size:10px">⬇️ Min</div><div class="minmax-value"><span id="minHumid">--</span>%</div></div>
<div class="avg-value">📊 <span id="avgHumid">--</span>%</div>
<div><div style="font-size:10px">⬆️ Max</div><div class="minmax-value"><span id="maxHumid">--</span>%</div></div>
</div>
<div id="humidComfort" class="comfort-indicator"></div>
</div>
</div>

<div class="chart-row double">
<div class="card sensor-card dewpoint-card">
<div class="sensor-header"><div class="sensor-label">💧 Dew point</div></div>
<div class="sensor-value"><span id="dewPoint">--</span><span class="sensor-unit" id="dewPointUnit">°C</span></div>
<div class="sensor-description">Condensation temperature of water vapor</div>
</div>
<div class="card sensor-card heatindex-card">
<div class="sensor-header"><div class="sensor-label">🌡️ Heat Index</div></div>
<div class="sensor-value"><span id="heatIndex">--</span><span class="sensor-unit" id="heatIndexUnit">°C</span></div>
<div class="sensor-description">Temperature perception based on humidity</div>
</div>
</div>

<!-- System & Control -->
<div class="chart-row">
<div class="card">
<h3 style="margin-bottom:15px">💻 System & Control</h3>
<div class="info-grid">
<div class="info-item"><div class="info-label">⏱️ Uptime</div><div class="info-value" id="uptime">--</div></div>
<div class="info-item"><div class="info-label">🧠 RAM</div><div class="info-value" id="freeHeap">--</div></div>
<div class="info-item"><div class="info-label">📊 CPU</div><div class="info-value" id="cpuUsage">--</div></div>

<!-- Battery: spans 3 cols on wide, 1 on narrow -->
<div class="info-item" style="grid-column:1/-1">
<div class="info-label">🔋 Battery</div>
<div class="battery-widget">
<div class="battery-icon"><div class="battery-fill" id="batteryFill" style="width:0%"></div></div>
<div>
<div class="battery-text" id="batteryPercent">--</div>
<div class="battery-sub" id="batterySource">--</div>
</div>
<div style="margin-left:auto;text-align:right">
<div class="battery-voltage info-value" id="batteryVoltage">--</div>
<div class="battery-sub">voltage</div>
</div>
</div>
</div>

<div class="info-item"><div class="info-label">📶 SSID</div><div class="info-value" id="ssid" style="font-size:13px">--</div></div>
<div class="info-item"><div class="info-label"><span id="wifiSignal">📶</span> RSSI</div><div class="info-value" id="rssi">--</div></div>
<div class="info-item"><div class="info-label">🌐 IP</div><div class="info-value" id="ipAddr" style="font-size:11px">--</div></div>
</div>
<div class="buttons">
<button class="btn btn-primary" onclick="exportCSV()">📥 CSV</button>
<button class="btn btn-success" onclick="exportJSON()">📋 JSON</button>
<button class="btn btn-success" onclick="resetMinMax()">🔄 Reset</button>
<button class="btn btn-danger"  onclick="rebootDevice()">⚡ Reboot</button>
</div>
</div>
</div>

<!-- Serial Monitor -->
<div class="chart-row">
<div class="card">
<h3>📟 Serial Monitor <span class="ws-status" id="wsStatus"></span><span id="wsStatusText" style="font-size:12px;color:var(--text-sub)">Connecting...</span></h3>
<div class="log-filters">
<button class="log-filter-btn lf-error   active" onclick="toggleLogFilter('error',this)"  >● ERROR</button>
<button class="log-filter-btn lf-warning active" onclick="toggleLogFilter('warning',this)">● WARN</button>
<button class="log-filter-btn lf-info    active" onclick="toggleLogFilter('info',this)"   >● INFO</button>
<button class="log-filter-btn lf-success active" onclick="toggleLogFilter('success',this)">● OK</button>
<span class="log-counter" id="logCounter">0 lines</span>
</div>
<div class="log-console" id="logConsole"></div>
<div class="buttons" style="margin-top:15px">
<button class="btn btn-primary" onclick="clearLogs()">🗑️ Clear</button>
<button class="btn btn-success" onclick="toggleAutoscroll()"><span id="autoscrollIcon">📌</span> Auto-scroll</button>
</div>
</div>
</div>

<!-- Combined history chart -->
<div class="chart-row">
<div class="chart-card">
<div style="display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;margin-bottom:14px">
<h3>📈 History</h3>
<div class="time-range">
<button class="tr-btn" onclick="setRange(60,this)">1h</button>
<button class="tr-btn" onclick="setRange(360,this)">6h</button>
<button class="tr-btn" onclick="setRange(1440,this)">24h</button>
<button class="tr-btn active" onclick="setRange(0,this)">All</button>
</div>
</div>
<div class="chart-toggles">
<button class="chart-toggle-btn toggle-temp  active" onclick="toggleSeries(0,this)"><span class="dot"></span>🌡️ Temp</button>
<button class="chart-toggle-btn toggle-humid active" onclick="toggleSeries(1,this)"><span class="dot"></span>💧 Humid</button>
<button class="chart-toggle-btn toggle-heat  active" onclick="toggleSeries(2,this)"><span class="dot"></span>🌡️ Heat</button>
<button class="chart-toggle-btn toggle-dew   active" onclick="toggleSeries(3,this)"><span class="dot"></span>💧 Dew</button>
</div>
<canvas id="combinedChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeCombined">--</span></div>
</div>
</div>

</div><!-- /container -->

<script>
/* ── State ─────────────────────────────────────────────────── */
let F=false, C, errCnt=0, iU, iS, iH;
let ws, autoscroll=true, maxLogs=500;
let activeLogFilters=new Set(['error','warning','info','success']);
let rawHistory={labels:[],temp:[],humid:[],heat:[],dew:[]};
let rangeMinutes=0;

/* ── Theme ─────────────────────────────────────────────────── */
function toggleTheme(){
  const dark=document.body.classList.toggle('dark');
  document.getElementById('themeBtn').textContent=dark?'☀️':'🌙';
  localStorage.setItem('theme',dark?'dark':'light');
  syncChartTheme();
}
function applyTheme(){
  if(localStorage.getItem('theme')==='dark'){
    document.body.classList.add('dark');
    document.getElementById('themeBtn').textContent='☀️';
  }
}
function syncChartTheme(){
  if(!C)return;
  const dark=document.body.classList.contains('dark');
  const gc=dark?'rgba(255,255,255,.06)':'rgba(0,0,0,.05)';
  const tc=dark?'#9fa8da':'#666';
  C.options.scales.x.grid.color=gc;
  C.options.scales.y.grid.color=gc;
  C.options.scales.x.ticks.color=tc;
  C.options.scales.y.ticks.color=tc;
  C.options.scales.y.title.color=tc;
  C.update();
}

/* ── WebSocket / Serial ────────────────────────────────────── */
function initWebSocket(){
  const proto=window.location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(`${proto}//${window.location.hostname}:81/`);
  ws.onopen=()=>{
    document.getElementById('wsStatus').className='ws-status ws-connected';
    document.getElementById('wsStatusText').textContent='Connected';
    addLog('WebSocket connected','success');
  };
  ws.onclose=()=>{
    document.getElementById('wsStatus').className='ws-status ws-disconnected';
    document.getElementById('wsStatusText').textContent='Disconnected';
    addLog('WebSocket disconnected, reconnecting...','warning');
    setTimeout(initWebSocket,3000);
  };
  ws.onerror=()=>addLog('WebSocket error','error');
  ws.onmessage=e=>{
    const m=e.data;
    let t='info';
    if(/error|fail|err/i.test(m)) t='error';
    else if(/warn|caution/i.test(m)) t='warning';
    else if(/ok|success|done|ready/i.test(m)) t='success';
    addLog(m,t);
  };
}

function addLog(msg,type='info'){
  const con=document.getElementById('logConsole');
  const line=document.createElement('div');
  line.className=`log-line log-${type}`;
  line.dataset.type=type;
  line.textContent=`[${new Date().toLocaleTimeString('ru-RU')}] ${msg}`;
  if(!activeLogFilters.has(type)) line.classList.add('hidden');
  con.appendChild(line);
  while(con.children.length>maxLogs) con.removeChild(con.firstChild);
  if(autoscroll) con.scrollTop=con.scrollHeight;
  updateLogCounter();
}

function toggleLogFilter(type,btn){
  btn.classList.toggle('active');
  if(activeLogFilters.has(type)) activeLogFilters.delete(type);
  else activeLogFilters.add(type);
  document.querySelectorAll(`#logConsole .log-${type}`).forEach(l=>
    l.classList.toggle('hidden',!activeLogFilters.has(type)));
  updateLogCounter();
}

function updateLogCounter(){
  const total=document.querySelectorAll('#logConsole .log-line').length;
  const vis=document.querySelectorAll('#logConsole .log-line:not(.hidden)').length;
  document.getElementById('logCounter').textContent=`${vis}/${total} lines`;
}

function clearLogs(){
  document.getElementById('logConsole').innerHTML='';
  updateLogCounter();
  addLog('Logs cleared','success');
}
function toggleAutoscroll(){
  autoscroll=!autoscroll;
  document.getElementById('autoscrollIcon').textContent=autoscroll?'📌':'📍';
}

/* ── Chart init ────────────────────────────────────────────── */
const DS_CFG=[
  {label:'Temperature',border:'#667eea',bg:'rgba(102,126,234,.12)'},
  {label:'Humidity',   border:'#4facfe',bg:'rgba(79,172,254,.12)'},
  {label:'Heat Index', border:'#fa709a',bg:'rgba(250,112,154,.12)'},
  {label:'Dew Point',  border:'#f093fb',bg:'rgba(240,147,251,.12)'},
];

function initCharts(){
  const ctx=document.getElementById('combinedChart').getContext('2d');
  C=new Chart(ctx,{
    type:'line',
    data:{labels:[],datasets:DS_CFG.map(c=>({
      label:c.label,data:[],
      borderColor:c.border,backgroundColor:c.bg,
      tension:.4,fill:true,borderWidth:3,pointRadius:3,pointHoverRadius:6
    }))},
    options:{
      responsive:true,maintainAspectRatio:false,
      interaction:{mode:'index',intersect:false},
      plugins:{
        legend:{display:false},
        tooltip:{
          backgroundColor:'rgba(20,20,30,.92)',padding:14,
          titleFont:{size:13,weight:'bold'},bodyFont:{size:13},
          borderColor:'rgba(255,255,255,.1)',borderWidth:1,
          callbacks:{
            title:c=>'⏱ '+c[0].label,
            label:c=>{const u=['°C','%','°C','°C'];return' '+c.dataset.label+': '+c.parsed.y.toFixed(1)+' '+u[c.datasetIndex]}
          }
        }
      },
      scales:{
        x:{grid:{color:'rgba(0,0,0,.05)',drawBorder:false},ticks:{font:{size:11},maxRotation:0,autoSkip:true,maxTicksLimit:10,color:'#666'}},
        y:{position:'right',grid:{drawBorder:false,color:'rgba(0,0,0,.05)'},ticks:{font:{size:12},color:'#666'},title:{display:true,text:'°C / %',color:'#666'}}
      },
      animation:{duration:600,easing:'easeInOutQuart'}
    }
  });
  syncChartTheme();
}

function toggleSeries(idx,btn){
  const m=C.getDatasetMeta(idx);
  m.hidden=!m.hidden;
  btn.classList.toggle('active',!m.hidden);
  C.update();
}

/* ── Time range ────────────────────────────────────────────── */
function setRange(minutes,btn){
  rangeMinutes=minutes;
  document.querySelectorAll('.tr-btn').forEach(b=>b.classList.remove('active'));
  btn.classList.add('active');
  renderChart();
}

function sliceByRange(arr){
  if(!rangeMinutes||!arr.length) return arr;
  const pts=Math.round(rangeMinutes*60/15);
  return arr.slice(-Math.min(pts,arr.length));
}

function renderChart(){
  if(!C||!rawHistory.labels.length) return;
  const c2fA=a=>F?a.map(v=>v*9/5+32):a;
  C.data.labels=sliceByRange(rawHistory.labels);
  C.data.datasets[0].data=c2fA(sliceByRange(rawHistory.temp));
  C.data.datasets[1].data=sliceByRange(rawHistory.humid);
  C.data.datasets[2].data=c2fA(sliceByRange(rawHistory.heat));
  C.data.datasets[3].data=c2fA(sliceByRange(rawHistory.dew));
  C.options.scales.y.title.text=F?'°F / %':'°C / %';
  C.update();
  document.getElementById('updateTimeCombined').textContent=new Date().toLocaleTimeString();
}

/* ── Data fetchers ─────────────────────────────────────────── */
function c2f(c){return c*9/5+32}
function toggleUnit(){F=!F;updateDisplay()}

function updateDisplay(){
  const u=F?'°F':'°C';
  document.querySelectorAll('#tempUnit,#minTempUnit,#maxTempUnit,#avgTempUnit,#dewPointUnit,#heatIndexUnit').forEach(e=>e.textContent=u);
  renderChart();updateData();
}

function getComfort(v,isTemp){
  if(isTemp){
    if(v>=20&&v<=24)return{l:'excellent',t:'✅ Optimal'};
    if(v>=18&&v<=26)return{l:'good',t:'👍 Comfortable'};
    return{l:'poor',t:'❌ Uncomfortable'};
  }else{
    if(v>=40&&v<=60)return{l:'excellent',t:'✅ Optimal'};
    if(v>=30&&v<=70)return{l:'good',t:'👍 Normal'};
    return{l:'poor',t:'❌ Uncomfortable'};
  }
}

function updateData(){
  fetch('/data').then(r=>r.json()).then(d=>{
    const t=F?c2f(d.temperature):d.temperature;
    const minT=F?c2f(d.minTemp):d.minTemp,maxT=F?c2f(d.maxTemp):d.maxTemp,avgT=F?c2f(d.avgTemp):d.avgTemp;
    const dewP=F?c2f(d.dewPoint):d.dewPoint,heatI=F?c2f(d.heatIndex):d.heatIndex;
    document.getElementById('temperature').textContent=t.toFixed(1);
    document.getElementById('humidity').textContent=d.humidity.toFixed(1);
    document.getElementById('minTemp').textContent=minT.toFixed(1);
    document.getElementById('maxTemp').textContent=maxT.toFixed(1);
    document.getElementById('avgTemp').textContent=avgT.toFixed(1);
    document.getElementById('minHumid').textContent=d.minHumid.toFixed(1);
    document.getElementById('maxHumid').textContent=d.maxHumid.toFixed(1);
    document.getElementById('avgHumid').textContent=d.avgHumid.toFixed(1);
    document.getElementById('dewPoint').textContent=dewP.toFixed(1);
    document.getElementById('heatIndex').textContent=heatI.toFixed(1);
    const tc=getComfort(d.temperature,true),hc=getComfort(d.humidity,false);
    const te=document.getElementById('tempComfort');te.textContent=tc.t;te.className='comfort-indicator comfort-'+tc.l;
    const he=document.getElementById('humidComfort');he.textContent=hc.t;he.className='comfort-indicator comfort-'+hc.l;
    document.getElementById('lastUpdate').textContent='Updated: '+new Date().toLocaleTimeString('ru-RU');
    errCnt=0;
    document.getElementById('statusBadge').className='status online';
    document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Connected</span>';
  }).catch(()=>{
    errCnt++;
    if(errCnt>2){
      document.getElementById('statusBadge').className='status offline';
      document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Disconnected</span>';
    }
  });
}

function updateStats(){
  fetch('/stats').then(r=>r.json()).then(d=>{
    document.getElementById('uptime').textContent=d.uptime;
    document.getElementById('freeHeap').textContent=d.freeHeap;
    document.getElementById('cpuUsage').textContent=d.cpuUsage+'%';
    document.getElementById('ssid').textContent=d.ssid||'--';
    const rv=parseInt(d.rssi);
    document.getElementById('wifiSignal').textContent=rv>=-60?'📶':rv>=-75?'📶':'⚠️';
    document.getElementById('rssi').textContent=d.rssi+' dBm';
    document.getElementById('ipAddr').textContent=d.ip;

    /* Battery visual */
    if(d.battery){
      const b=d.battery;
      const pct=Math.max(0,Math.min(100,b.percent));
      const fill=document.getElementById('batteryFill');
      fill.style.width=pct+'%';
      fill.className='battery-fill '+(
        b.isCharging?'charging':
        b.isCritical?'critical':
        b.isLow?'low':
        pct<60?'mid':'good'
      );
      let pTxt=pct+'%';
      if(b.isCritical) pTxt='‼️ '+pTxt;
      else if(b.isLow) pTxt='⚠️ '+pTxt;
      else if(b.isCharging) pTxt='⚡ '+pTxt;
      else if(pct>=80) pTxt='🟢 '+pTxt;
      document.getElementById('batteryPercent').textContent=pTxt;
      document.getElementById('batteryVoltage').textContent=b.voltage+'V';
      let sTxt=b.source;
      if(b.isCharging) sTxt='Charging...';
      else if(b.status==='Fully charged') sTxt='Full ✓';
      document.getElementById('batterySource').textContent=sTxt;
    }
  }).catch(e=>console.error(e));
}

function updateHistory(){
  fetch('/history').then(r=>r.json()).then(d=>{
    rawHistory.labels=d.labels;
    rawHistory.temp=d.temp;
    rawHistory.humid=d.humid;
    rawHistory.heat=d.heat||d.temp;
    rawHistory.dew=d.dew||d.temp;
    renderChart();
  }).catch(e=>console.error(e));
}

/* ── Controls ──────────────────────────────────────────────── */
function resetMinMax(){if(confirm('Reset min/max?')){fetch('/reset').then(()=>updateData());}}
function rebootDevice(){if(confirm('Reboot?')){fetch('/reboot');}}

function exportCSV(){
  const td=C.data.datasets;
  let csv='Time,Temp,Humid,HeatIndex,DewPoint\n';
  for(let i=0;i<C.data.labels.length;i++)
    csv+=`${C.data.labels[i]},${td[0].data[i]||''},${td[1].data[i]||''},${td[2].data[i]||''},${td[3].data[i]||''}\n`;
  const a=document.createElement('a');
  a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));
  a.download='weather.csv';a.click();
}
function exportJSON(){
  const o={labels:C.data.labels,temp:C.data.datasets[0].data,humid:C.data.datasets[1].data,heat:C.data.datasets[2].data,dew:C.data.datasets[3].data};
  const a=document.createElement('a');
  a.href=URL.createObjectURL(new Blob([JSON.stringify(o)],{type:'application/json'}));
  a.download='weather.json';a.click();
}

/* ── Init ──────────────────────────────────────────────────── */
document.addEventListener('DOMContentLoaded',()=>{
  applyTheme();
  initCharts();
  initWebSocket();
  updateData();updateStats();updateHistory();
  iU=setInterval(updateData,10000);
  iS=setInterval(updateStats,10000);
  iH=setInterval(updateHistory,15000);
});
</script>
</body>
</html>
)rawliteral";

#endif
