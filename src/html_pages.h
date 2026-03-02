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
:root{
  --bg-page:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  --bg-card:rgba(255,255,255,.95);
  --bg-info:#f8f9fa;
  --text-main:#333;
  --text-sub:#666;
  --text-muted:#999;
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
  --toggle-bg:#252840;
  --toggle-color:#9fa8da;
  --update-bg:#252840;
  --info-border:#5c6bc0;
  --shadow:0 4px 16px rgba(0,0,0,.4);
  --shadow-hover:0 10px 30px rgba(0,0,0,.5);
}
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:var(--bg-page);min-height:100vh;padding:20px;transition:background .4s}
.container{max-width:700px;margin:0 auto;position:relative;z-index:1}

/* Header */
.header{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:20px;padding:30px;margin-bottom:20px;box-shadow:0 10px 30px rgba(0,0,0,.2);text-align:center;transition:background .3s}
.header h1{color:var(--text-main);font-size:clamp(24px,5vw,36px);margin-bottom:10px;font-weight:700}
.subtitle{color:var(--text-sub);font-size:14px;margin-bottom:15px}
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

/* Countdown bar */
.countdown-wrap{margin-top:12px;display:flex;align-items:center;gap:10px}
.countdown-bar-track{flex:1;height:4px;background:rgba(0,0,0,.08);border-radius:2px;overflow:hidden}
body.dark .countdown-bar-track{background:rgba(255,255,255,.1)}
.countdown-bar-fill{height:100%;border-radius:2px;background:linear-gradient(90deg,#667eea,#764ba2);transition:background .4s}
.countdown-bar-fill.warn{background:linear-gradient(90deg,#f7971e,#ffd200)}
.countdown-text{font-size:12px;font-weight:600;color:var(--text-sub);white-space:nowrap;min-width:60px;text-align:right}

/* Dark toggle */
.theme-toggle{position:fixed;top:18px;right:18px;z-index:999;background:var(--bg-card);border:none;border-radius:50%;width:46px;height:46px;font-size:20px;cursor:pointer;box-shadow:var(--shadow);transition:all .3s;display:flex;align-items:center;justify-content:center}
.theme-toggle:hover{transform:scale(1.12) rotate(20deg)}

/* Settings */
.settings-btn{position:fixed;top:72px;right:18px;z-index:999;background:var(--bg-card);border:none;border-radius:50%;width:46px;height:46px;font-size:20px;cursor:pointer;box-shadow:var(--shadow);transition:all .3s;display:flex;align-items:center;justify-content:center}
.settings-btn:hover{transform:scale(1.12) rotate(30deg)}
.settings-backdrop{display:none;position:fixed;inset:0;background:rgba(0,0,0,.4);z-index:1000;backdrop-filter:blur(4px)}
.settings-backdrop.open{display:block}
.settings-panel{position:fixed;top:0;right:0;height:100%;width:min(320px,90vw);background:var(--bg-card);z-index:1001;box-shadow:-8px 0 32px rgba(0,0,0,.25);padding:30px 24px;overflow-y:auto;transform:translateX(100%);transition:transform .35s cubic-bezier(.4,0,.2,1)}
.settings-panel.open{transform:translateX(0)}
.settings-panel h2{color:var(--text-main);font-size:18px;margin-bottom:6px}
.settings-desc{color:var(--text-sub);font-size:12px;margin-bottom:22px}
.settings-close{position:absolute;top:20px;right:20px;background:none;border:none;font-size:22px;cursor:pointer;color:var(--text-sub)}
.settings-item{display:flex;align-items:center;justify-content:space-between;padding:14px 0;border-bottom:1px solid rgba(128,128,128,.12)}
.settings-item:last-child{border-bottom:none}
.settings-item-label{color:var(--text-main);font-size:14px;font-weight:600}
.settings-item-sub{color:var(--text-sub);font-size:11px;margin-top:2px}
.sw{position:relative;display:inline-block;width:48px;height:26px;flex-shrink:0}
.sw input{opacity:0;width:0;height:0}
.sw-track{position:absolute;inset:0;background:#ccc;border-radius:13px;transition:.3s;cursor:pointer}
.sw-track:before{position:absolute;content:'';height:20px;width:20px;left:3px;bottom:3px;background:#fff;border-radius:50%;transition:.3s;box-shadow:0 1px 4px rgba(0,0,0,.2)}
.sw input:checked+.sw-track{background:#667eea}
.sw input:checked+.sw-track:before{transform:translateX(22px)}
body.dark .sw-track{background:#444}

/* Cards */
.card{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:var(--shadow);transition:background .3s,transform .3s,box-shadow .3s}
.card:hover{transform:translateY(-3px);box-shadow:var(--shadow-hover)}
.card h3{color:var(--text-main)}
.sensor-card{color:#fff;position:relative;overflow:hidden;transition:background 2s ease}
.sensor-card::before{content:'';position:absolute;top:-50%;right:-50%;width:200%;height:200%;background:radial-gradient(circle,rgba(255,255,255,.1) 0%,transparent 70%);pointer-events:none}
.sensor-card::after{content:'';position:absolute;inset:0;background:linear-gradient(105deg,transparent 40%,rgba(255,255,255,.07) 50%,transparent 60%);background-size:200% 100%;animation:shimmer 4s linear infinite;pointer-events:none}
@keyframes shimmer{0%{background-position:200% 0}100%{background-position:-200% 0}}
.temp-card{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%)}
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
.comfort-indicator{margin-top:10px;padding:8px 12px;border-radius:8px;font-size:11px;font-weight:600;text-align:center;background:rgba(255,255,255,.2)}
.comfort-excellent{background:rgba(40,167,69,.3)}
.comfort-good{background:rgba(255,193,7,.3)}
.comfort-poor{background:rgba(220,53,69,.3)}

/* Sparkline */
.sparkline-wrap{margin-top:12px;padding:10px 0 4px;border-top:1px solid rgba(255,255,255,.15)}
.sparkline-label{font-size:10px;opacity:.7;text-transform:uppercase;letter-spacing:1px;margin-bottom:4px}
canvas.sparkline{height:44px!important;min-height:44px!important;max-height:44px!important;width:100%!important;display:block}

/* Temp toggle */
.temp-unit-toggle{margin-top:15px;text-align:center;padding:15px;background:rgba(255,255,255,.1);border-radius:8px}
.toggle-label{color:#fff;font-weight:600;font-size:13px}
.toggle-switch{position:relative;display:inline-block;width:60px;height:30px;margin:0 12px}
.toggle-switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:rgba(255,255,255,.3);transition:.4s;border-radius:30px}
.slider:before{position:absolute;content:'';height:24px;width:24px;left:3px;bottom:3px;background:#fff;transition:.4s;border-radius:50%}
input:checked+.slider{background:rgba(255,255,255,.4)}
input:checked+.slider:before{transform:translateX(30px)}

/* Info grid */
.info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px;margin-top:15px}
.info-item{background:var(--bg-info);padding:15px;border-radius:8px;border-left:4px solid var(--info-border);transition:background .3s}
.info-label{color:var(--text-sub);font-size:11px;text-transform:uppercase;margin-bottom:8px}
.info-value{color:var(--text-main);font-weight:700;font-size:clamp(14px,3vw,18px)}

/* RAM bar */
.ram-bar-track{height:6px;border-radius:3px;background:rgba(0,0,0,.1);margin-top:8px;overflow:hidden}
body.dark .ram-bar-track{background:rgba(255,255,255,.12)}
.ram-bar-fill{height:100%;border-radius:3px;transition:width .6s ease,background .4s}
.ram-ok{background:linear-gradient(90deg,#43e97b,#38f9d7)}
.ram-mid{background:linear-gradient(90deg,#f7971e,#ffd200)}
.ram-high{background:linear-gradient(90deg,#f5515f,#9f041b)}
.ram-stats{display:flex;justify-content:space-between;font-size:10px;color:var(--text-sub);margin-top:4px}

/* Battery */
.battery-widget{display:flex;align-items:center;gap:12px;margin-top:6px}
.battery-icon{position:relative;width:44px;height:22px;border:2.5px solid var(--text-main);border-radius:4px;flex-shrink:0}
.battery-icon::after{content:'';position:absolute;right:-7px;top:50%;transform:translateY(-50%);width:5px;height:11px;background:var(--text-main);border-radius:0 3px 3px 0}
.battery-fill{height:100%;border-radius:2px;transition:width .6s ease,background .4s ease}
.battery-fill.critical{background:#dc3545}
.battery-fill.low{background:#ff9800}
.battery-fill.mid{background:#ffc107}
.battery-fill.good{background:#28a745}
.battery-fill.charging{background:linear-gradient(90deg,#28a745,#00e676);animation:chargepulse 1.2s ease-in-out infinite}
@keyframes chargepulse{0%,100%{opacity:1}50%{opacity:.55}}
.battery-text{font-size:14px;font-weight:700;color:var(--text-main)}
.battery-sub{font-size:11px;color:var(--text-sub);margin-top:2px}

/* WiFi bars */
.wifi-widget{display:flex;align-items:center;gap:10px}
.wifi-bars{display:flex;align-items:flex-end;gap:3px;height:20px}
.wifi-bar{width:6px;border-radius:2px 2px 0 0;transition:background-color .6s ease}
.wifi-bar:nth-child(1){height:5px}
.wifi-bar:nth-child(2){height:10px}
.wifi-bar:nth-child(3){height:15px}
.wifi-bar:nth-child(4){height:20px}
.wifi-rssi-label{font-size:10px;color:var(--text-sub);margin-top:2px}

/* Charts */
.chart-row{display:grid;grid-template-columns:1fr;gap:20px;margin-bottom:20px}
.chart-row.double{grid-template-columns:1fr 1fr}
.chart-card{background:var(--bg-card);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:var(--shadow);transition:background .3s,transform .3s,box-shadow .3s}
.chart-card:hover{transform:translateY(-3px);box-shadow:var(--shadow-hover)}
.chart-card h3{color:var(--text-main)}
canvas{max-height:350px;min-height:250px;width:100%}
.update-time{text-align:center;color:var(--text-muted);font-size:12px;margin-top:15px;padding:10px;background:var(--update-bg);border-radius:8px}

/* Buttons */
.buttons{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:12px;margin-top:15px}
.btn{padding:12px 20px;border:none;border-radius:8px;font-size:14px;cursor:pointer;font-weight:600;display:flex;align-items:center;justify-content:center;gap:8px;transition:.3s;font-family:inherit}
.btn-primary{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff}
.btn-success{background:linear-gradient(135deg,#28a745 0%,#218838 100%);color:#fff}
.btn-danger{background:linear-gradient(135deg,#dc3545 0%,#c82333 100%);color:#fff}
.btn:hover{transform:translateY(-2px)}

/* Serial Monitor */
.log-console{background:#1e1e1e;color:#d4d4d4;font-family:'Consolas','Monaco',monospace;font-size:14px;padding:18px;border-radius:8px;height:450px;overflow-y:auto;margin-top:12px}
.log-console::-webkit-scrollbar{width:8px}
.log-console::-webkit-scrollbar-track{background:#2d2d2d;border-radius:4px}
.log-console::-webkit-scrollbar-thumb{background:#555;border-radius:4px}
.log-line{margin:2px 0;white-space:pre-wrap;word-break:break-all}
.log-line.hidden{display:none}
.log-error{color:#f48771}
.log-warning{color:#dcdcaa}
.log-info{color:#4ec9b0}
.log-success{color:#4fc1ff}
.ws-status{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:8px}
.ws-connected{background:#28a745}
.ws-disconnected{background:#dc3545}
.log-filters{display:flex;flex-wrap:wrap;gap:7px;margin-top:12px;align-items:center}
.log-filter-btn{padding:5px 14px;border:2px solid transparent;border-radius:20px;font-size:12px;font-weight:700;cursor:pointer;transition:all .2s;background:var(--toggle-bg);color:var(--toggle-color);font-family:inherit;opacity:.5}
.log-filter-btn.active{opacity:1}
.lf-error.active{background:#f48771;border-color:#f48771;color:#1e1e1e}
.lf-warning.active{background:#dcdcaa;border-color:#dcdcaa;color:#1e1e1e}
.lf-info.active{background:#4ec9b0;border-color:#4ec9b0;color:#1e1e1e}
.lf-success.active{background:#4fc1ff;border-color:#4fc1ff;color:#1e1e1e}
.log-filter-btn:hover{opacity:1;transform:translateY(-1px)}
.log-counter{font-size:11px;color:var(--text-muted);margin-left:auto;padding:5px 10px;background:var(--bg-info);border-radius:12px}

/* Chart toggles */
.chart-toggles{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:14px}
.chart-toggle-btn{display:flex;align-items:center;gap:7px;padding:8px 16px;border:2px solid transparent;border-radius:25px;font-size:13px;font-weight:600;cursor:pointer;transition:all .25s;background:var(--toggle-bg);color:var(--toggle-color);user-select:none;font-family:inherit}
.chart-toggle-btn .dot{width:10px;height:10px;border-radius:50%;flex-shrink:0;transition:transform .2s}
.chart-toggle-btn.active{color:#fff;transform:translateY(-1px);box-shadow:0 4px 12px rgba(0,0,0,.2)}
.chart-toggle-btn.active .dot{transform:scale(1.3)}
.chart-toggle-btn:not(.active){opacity:.45}
.chart-toggle-btn:hover{opacity:1;transform:translateY(-1px)}
.toggle-temp.active{background:linear-gradient(135deg,#667eea,#764ba2);border-color:#667eea}
.toggle-humid.active{background:linear-gradient(135deg,#4facfe,#00f2fe);border-color:#4facfe}
.toggle-heat.active{background:linear-gradient(135deg,#fa709a,#fee140);border-color:#fa709a}
.toggle-dew.active{background:linear-gradient(135deg,#f093fb,#f5576c);border-color:#f093fb}
.toggle-temp .dot{background:linear-gradient(135deg,#667eea,#764ba2)}
.toggle-humid .dot{background:linear-gradient(135deg,#4facfe,#00f2fe)}
.toggle-heat .dot{background:linear-gradient(135deg,#fa709a,#fee140)}
.toggle-dew .dot{background:linear-gradient(135deg,#f093fb,#f5576c)}

/* Time range */
.time-range{display:flex;gap:6px}
.tr-btn{padding:6px 14px;border:2px solid var(--info-border);border-radius:20px;font-size:12px;font-weight:700;cursor:pointer;background:transparent;color:var(--info-border);transition:all .2s;font-family:inherit}
.tr-btn.active{background:var(--info-border);color:#fff}
.tr-btn:hover{transform:translateY(-1px)}

/* Particle canvas */
#particleCanvas{position:fixed;inset:0;pointer-events:none;z-index:0;opacity:0;transition:opacity .6s ease}
#particleCanvas.visible{opacity:1}

@media(max-width:768px){
body{padding:10px}
.header{padding:20px}
.card{padding:20px}
.info-grid{grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px}
.buttons{grid-template-columns:1fr}
.chart-row.double{grid-template-columns:1fr}
.chart-toggle-btn{padding:7px 12px;font-size:12px}
.theme-toggle{top:10px;right:10px;width:40px;height:40px;font-size:17px}
.settings-btn{top:62px;right:10px;width:40px;height:40px;font-size:17px}
}
</style>
</head>
<body>
<canvas id="particleCanvas"></canvas>
<button class="theme-toggle" id="themeBtn" onclick="toggleTheme()" title="Toggle dark mode">&#x1F319;</button>
<button class="settings-btn" onclick="openSettings()" title="Settings">&#x2699;&#xFE0F;</button>

<div class="settings-backdrop" id="settingsBackdrop" onclick="closeSettings()"></div>
<div class="settings-panel" id="settingsPanel">
<button class="settings-close" onclick="closeSettings()">&#x2715;</button>
<h2>&#x2699;&#xFE0F; Display</h2>
<div class="settings-desc">Show or hide sections</div>
<div class="settings-item">
<div><div class="settings-item-label">&#x1F321;&#xFE0F; Sensor tiles</div><div class="settings-item-sub">Temperature, Humidity cards</div></div>
<label class="sw"><input type="checkbox" id="sw-sensors" checked onchange="toggleBlock('sensorRows',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">&#x1F4BB; System &amp; Control</div><div class="settings-item-sub">RAM, CPU, Battery, WiFi</div></div>
<label class="sw"><input type="checkbox" id="sw-system" checked onchange="toggleBlock('systemRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">&#x1F4DF; Serial Monitor</div><div class="settings-item-sub">WebSocket log console</div></div>
<label class="sw"><input type="checkbox" id="sw-serial" checked onchange="toggleBlock('serialRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">&#x1F4C8; History chart</div><div class="settings-item-sub">Time-series graph</div></div>
<label class="sw"><input type="checkbox" id="sw-chart" checked onchange="toggleBlock('chartRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item" style="flex-direction:column;align-items:flex-start;gap:10px">
<div>
<div class="settings-item-label">&#x1F327;&#xFE0F; Weather particles</div>
<div class="settings-item-sub">Background rain / snow effect</div>
</div>
<div style="display:flex;flex-wrap:wrap;gap:7px">
<button class="log-filter-btn active" id="pbOff"     onclick="forceParticle('none',this)"   >&#x2600;&#xFE0F; Off</button>
<button class="log-filter-btn"        id="pbDrizzle" onclick="forceParticle('drizzle',this)">&#x1F326;&#xFE0F; Drizzle</button>
<button class="log-filter-btn"        id="pbRain"    onclick="forceParticle('rain',this)"   >&#x1F327;&#xFE0F; Rain</button>
<button class="log-filter-btn"        id="pbHeavy"   onclick="forceParticle('heavy',this)"  >&#x26C8;&#xFE0F; Heavy</button>
<button class="log-filter-btn"        id="pbSnow"    onclick="forceParticle('snow',this)"   >&#x2744;&#xFE0F; Snow</button>
</div>
<div style="font-size:11px;color:var(--text-sub)">Auto mode uses sensor data</div>
</div>
</div>

<div class="container">
<div class="header">
<h1>&#x1F321;&#xFE0F; Environmental Statistics</h1>
<div class="subtitle">Temperature and humidity monitoring</div>
<div class="status-container">
<div id="statusBadge" class="status online"><div class="status-dot"></div><span>Connected</span></div>
<div id="lastUpdateBadge" class="status" style="background:#e3f2fd;color:#1976d2"><span id="lastUpdate">Loading...</span></div>
</div>
<div class="countdown-wrap">
<div class="countdown-bar-track"><div class="countdown-bar-fill" id="cdFill" style="width:100%"></div></div>
<span class="countdown-text" id="cdText">--</span>
</div>
</div>

<div id="sensorRows">
<div class="chart-row double">
<div class="card sensor-card temp-card">
<div class="sensor-header"><div class="sensor-label">&#x1F321;&#xFE0F; Temperature</div></div>
<div class="sensor-value"><span id="temperature">--</span><span class="sensor-unit" id="tempUnit">&#xB0;C</span></div>
<div class="minmax">
<div><div style="font-size:10px">&#x2B07;&#xFE0F; Min</div><div class="minmax-value"><span id="minTemp">--</span><span id="minTempUnit">&#xB0;C</span></div></div>
<div class="avg-value">&#x1F4CA; <span id="avgTemp">--</span><span id="avgTempUnit">&#xB0;C</span></div>
<div><div style="font-size:10px">&#x2B06;&#xFE0F; Max</div><div class="minmax-value"><span id="maxTemp">--</span><span id="maxTempUnit">&#xB0;C</span></div></div>
</div>
<div class="temp-unit-toggle">
<span class="toggle-label">&#xB0;C</span>
<label class="toggle-switch"><input type="checkbox" id="unitToggle" onchange="toggleUnit()"><span class="slider"></span></label>
<span class="toggle-label">&#xB0;F</span>
</div>
<div id="tempComfort" class="comfort-indicator"></div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkTemp"></canvas></div>
</div>
<div class="card sensor-card humidity-card">
<div class="sensor-header"><div class="sensor-label">&#x1F4A7; Humidity</div></div>
<div class="sensor-value"><span id="humidity">--</span><span class="sensor-unit">%</span></div>
<div class="minmax">
<div><div style="font-size:10px">&#x2B07;&#xFE0F; Min</div><div class="minmax-value"><span id="minHumid">--</span>%</div></div>
<div class="avg-value">&#x1F4CA; <span id="avgHumid">--</span>%</div>
<div><div style="font-size:10px">&#x2B06;&#xFE0F; Max</div><div class="minmax-value"><span id="maxHumid">--</span>%</div></div>
</div>
<div id="humidComfort" class="comfort-indicator"></div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkHumid"></canvas></div>
</div>
</div>
<div class="chart-row double">
<div class="card sensor-card dewpoint-card">
<div class="sensor-header"><div class="sensor-label">&#x1F4A7; Dew point</div></div>
<div class="sensor-value"><span id="dewPoint">--</span><span class="sensor-unit" id="dewPointUnit">&#xB0;C</span></div>
<div class="sensor-description">Condensation temperature of water vapor</div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkDew"></canvas></div>
</div>
<div class="card sensor-card heatindex-card">
<div class="sensor-header"><div class="sensor-label">&#x1F321;&#xFE0F; Heat Index</div></div>
<div class="sensor-value"><span id="heatIndex">--</span><span class="sensor-unit" id="heatIndexUnit">&#xB0;C</span></div>
<div class="sensor-description">Temperature perception based on humidity</div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkHeat"></canvas></div>
</div>
</div>
</div>

<div id="systemRow" class="chart-row">
<div class="card">
<h3 style="margin-bottom:15px">&#x1F4BB; System &amp; Control</h3>
<div class="info-grid">
<div class="info-item"><div class="info-label">&#x23F1;&#xFE0F; Uptime</div><div class="info-value" id="uptime">--</div></div>
<div class="info-item">
<div class="info-label">&#x1F9E0; RAM</div>
<div class="info-value" id="freeHeap">--</div>
<div class="ram-bar-track"><div class="ram-bar-fill ram-ok" id="ramBarFill" style="width:0%"></div></div>
<div class="ram-stats"><span id="ramUsedPct">-- used</span><span id="ramFreePct">-- free</span></div>
</div>
<div class="info-item"><div class="info-label">&#x1F4CA; CPU</div><div class="info-value" id="cpuUsage">--</div></div>
<div class="info-item"><div class="info-label">&#x1F321;&#xFE0F; Chip Temp</div><div class="info-value" id="chipTemp" style="transition:color .5s">--</div></div>
<div class="info-item" style="grid-column:1/-1">
<div class="info-label">&#x1F50B; Battery</div>
<div class="battery-widget">
<div class="battery-icon"><div class="battery-fill" id="batteryFill" style="width:0%"></div></div>
<div><div class="battery-text" id="batteryPercent">--</div><div class="battery-sub" id="batterySource">--</div></div>
<div style="margin-left:auto;text-align:right"><div class="info-value" id="batteryVoltage">--</div><div class="battery-sub">voltage</div></div>
</div>
</div>
<div class="info-item"><div class="info-label">&#x1F4F6; SSID</div><div class="info-value" id="ssid" style="font-size:13px">--</div></div>
<div class="info-item">
<div class="info-label">&#x1F4E1; WiFi Signal</div>
<div class="wifi-widget">
<div class="wifi-bars">
<div class="wifi-bar" id="wb1"></div>
<div class="wifi-bar" id="wb2"></div>
<div class="wifi-bar" id="wb3"></div>
<div class="wifi-bar" id="wb4"></div>
</div>
<div><div class="info-value" id="rssi">--</div><div class="wifi-rssi-label" id="rssiLabel">--</div></div>
</div>
</div>
<div class="info-item"><div class="info-label">&#x1F310; IP</div><div class="info-value" id="ipAddr" style="font-size:11px">--</div></div>
</div>
<div class="buttons">
<button class="btn btn-primary" onclick="exportCSV()">&#x1F4E5; CSV</button>
<button class="btn btn-success" onclick="exportJSON()">&#x1F4CB; JSON</button>
<button class="btn btn-success" onclick="resetMinMax()">&#x1F504; Reset</button>
<button class="btn btn-danger"  onclick="rebootDevice()">&#x26A1; Reboot</button>
</div>
</div>
</div>

<div id="serialRow" class="chart-row">
<div class="card">
<h3>&#x1F4DF; Serial Monitor <span class="ws-status" id="wsStatus"></span><span id="wsStatusText" style="font-size:12px;color:var(--text-sub)">Connecting...</span></h3>
<div class="log-filters">
<button class="log-filter-btn lf-error active"   onclick="toggleLogFilter('error',this)"  >&#x25CF; ERROR</button>
<button class="log-filter-btn lf-warning active" onclick="toggleLogFilter('warning',this)">&#x25CF; WARN</button>
<button class="log-filter-btn lf-info active"    onclick="toggleLogFilter('info',this)"   >&#x25CF; INFO</button>
<button class="log-filter-btn lf-success active" onclick="toggleLogFilter('success',this)">&#x25CF; OK</button>
<span class="log-counter" id="logCounter">0 lines</span>
</div>
<div class="log-console" id="logConsole"></div>
<div class="buttons" style="margin-top:15px">
<button class="btn btn-primary" onclick="clearLogs()">&#x1F5D1;&#xFE0F; Clear</button>
<button class="btn btn-success" onclick="toggleAutoscroll()"><span id="autoscrollIcon">&#x1F4CC;</span> Auto-scroll</button>
</div>
</div>
</div>

<div id="chartRow" class="chart-row">
<div class="chart-card">
<div style="display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;margin-bottom:14px">
<h3>&#x1F4C8; History</h3>
<div class="time-range">
<button class="tr-btn" onclick="setRange(60,this)">1h</button>
<button class="tr-btn" onclick="setRange(360,this)">6h</button>
<button class="tr-btn" onclick="setRange(1440,this)">24h</button>
<button class="tr-btn active" onclick="setRange(0,this)">All</button>
</div>
</div>
<div class="chart-toggles">
<button class="chart-toggle-btn toggle-temp active"  onclick="toggleSeries(0,this)"><span class="dot"></span>&#x1F321;&#xFE0F; Temp</button>
<button class="chart-toggle-btn toggle-humid active" onclick="toggleSeries(1,this)"><span class="dot"></span>&#x1F4A7; Humid</button>
<button class="chart-toggle-btn toggle-heat active"  onclick="toggleSeries(2,this)"><span class="dot"></span>&#x1F321;&#xFE0F; Heat</button>
<button class="chart-toggle-btn toggle-dew active"   onclick="toggleSeries(3,this)"><span class="dot"></span>&#x1F4A7; Dew</button>
</div>
<canvas id="combinedChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeCombined">--</span></div>
</div>
</div>

</div>
<script>
/* ===== STATE ===== */
var F=false,C,errCnt=0,iU,iS,iH;
var ws,autoscroll=true,maxLogs=500;
var activeLogFilters=new Set(['error','warning','info','success']);
var rawHistory={labels:[],temp:[],humid:[],heat:[],dew:[]};
var rangeMinutes=0;
var sparkCharts={};
var BLOCK_KEY='envBlockPrefs';

/* ===== THEME ===== */
function toggleTheme(){
  var dark=document.body.classList.toggle('dark');
  document.getElementById('themeBtn').textContent=dark?'\u2600\uFE0F':'\uD83C\uDF19';
  localStorage.setItem('theme',dark?'dark':'light');
  syncChartTheme();
}
function applyTheme(){
  if(localStorage.getItem('theme')==='dark'){
    document.body.classList.add('dark');
    document.getElementById('themeBtn').textContent='\u2600\uFE0F';
  }
}
function syncChartTheme(){
  if(!C)return;
  var dark=document.body.classList.contains('dark');
  var gc=dark?'rgba(255,255,255,.06)':'rgba(0,0,0,.05)';
  var tc=dark?'#9fa8da':'#666';
  C.options.scales.x.grid.color=gc;
  C.options.scales.y.grid.color=gc;
  C.options.scales.x.ticks.color=tc;
  C.options.scales.y.ticks.color=tc;
  C.options.scales.y.title.color=tc;
  C.update();
}

/* ===== SETTINGS ===== */
function openSettings(){
  document.getElementById('settingsBackdrop').classList.add('open');
  document.getElementById('settingsPanel').classList.add('open');
}
function closeSettings(){
  document.getElementById('settingsBackdrop').classList.remove('open');
  document.getElementById('settingsPanel').classList.remove('open');
}
function toggleBlock(id,visible){
  var el=document.getElementById(id);
  if(!el)return;
  el.style.opacity=visible?'1':'0';
  el.style.maxHeight=visible?'9999px':'0';
  el.style.overflow='hidden';
  el.style.transition='opacity .3s,max-height .4s';
  var prefs=JSON.parse(localStorage.getItem(BLOCK_KEY)||'{}');
  prefs[id]=visible;
  localStorage.setItem(BLOCK_KEY,JSON.stringify(prefs));
}
function applyBlockPrefs(){
  var prefs=JSON.parse(localStorage.getItem(BLOCK_KEY)||'{}');
  var map={sensorRows:'sw-sensors',systemRow:'sw-system',serialRow:'sw-serial',chartRow:'sw-chart'};
  Object.keys(map).forEach(function(blockId){
    if(prefs[blockId]===false){
      var sw=document.getElementById(map[blockId]);
      if(sw)sw.checked=false;
      toggleBlock(blockId,false);
    }
  });
}

/* ===== WEBSOCKET ===== */
function initWebSocket(){
  var proto=window.location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(proto+'//'+window.location.hostname+':81/');
  ws.onopen=function(){
    document.getElementById('wsStatus').className='ws-status ws-connected';
    document.getElementById('wsStatusText').textContent='Connected';
    addLog('WebSocket connected','success');
  };
  ws.onclose=function(){
    document.getElementById('wsStatus').className='ws-status ws-disconnected';
    document.getElementById('wsStatusText').textContent='Disconnected';
    addLog('WebSocket disconnected, reconnecting...','warning');
    setTimeout(initWebSocket,3000);
  };
  ws.onerror=function(){addLog('WebSocket error','error');};
  ws.onmessage=function(e){
    var m=e.data;
    var t='info';
    if(/error|fail|err/i.test(m))t='error';
    else if(/warn|caution/i.test(m))t='warning';
    else if(/ok|success|done|ready/i.test(m))t='success';
    addLog(m,t);
    // Parse chip temp: "Chip: 52.0C"
    var tm=m.match(/(?:temp[\s_]?chip|chip[\s_]?temp|cpu[\s_]?temp|internal[\s_]?temp|core[\s_]?temp|t_chip|chip_t)[^\d\-]*(-?[\d.]+)/i);
    if(!tm)tm=m.match(/^CHIP[:\s]+(-?[\d.]+)/i);
    if(!tm)tm=m.match(/\bChip:\s*(-?[\d.]+)\s*[CF]?\b/i);
    if(tm){var v=parseFloat(tm[1]);if(!isNaN(v))updateChipTemp(v);}
    // Parse RAM: "RAM: 208 KB free / 280 KB (25.6% used)"
    var rm=m.match(/RAM:\s*([\d.]+)\s*KB\s*free\s*\/\s*([\d.]+)\s*KB\s*\(([\d.]+)%\s*used\)/i);
    if(rm){updateRamDisplay(parseFloat(rm[1]),parseFloat(rm[2]),parseFloat(rm[3]));}
  };
}

/* ===== LOGS ===== */
function addLog(msg,type){
  if(!type)type='info';
  var con=document.getElementById('logConsole');
  var line=document.createElement('div');
  line.className='log-line log-'+type;
  line.dataset.type=type;
  line.textContent='['+new Date().toLocaleTimeString('ru-RU')+'] '+msg;
  if(!activeLogFilters.has(type))line.classList.add('hidden');
  con.appendChild(line);
  while(con.children.length>maxLogs)con.removeChild(con.firstChild);
  if(autoscroll)con.scrollTop=con.scrollHeight;
  updateLogCounter();
}
function toggleLogFilter(type,btn){
  btn.classList.toggle('active');
  if(activeLogFilters.has(type))activeLogFilters.delete(type);
  else activeLogFilters.add(type);
  document.querySelectorAll('#logConsole .log-'+type).forEach(function(l){
    l.classList.toggle('hidden',!activeLogFilters.has(type));
  });
  updateLogCounter();
}
function updateLogCounter(){
  var total=document.querySelectorAll('#logConsole .log-line').length;
  var vis=document.querySelectorAll('#logConsole .log-line:not(.hidden)').length;
  document.getElementById('logCounter').textContent=vis+'/'+total+' lines';
}
function clearLogs(){
  document.getElementById('logConsole').innerHTML='';
  updateLogCounter();
  addLog('Logs cleared','success');
}
function toggleAutoscroll(){
  autoscroll=!autoscroll;
  document.getElementById('autoscrollIcon').textContent=autoscroll?'\uD83D\uDCCC':'\uD83D\uDCCD';
}

/* ===== CHARTS ===== */
var DS_CFG=[
  {label:'Temperature',border:'#667eea',bg:'rgba(102,126,234,.12)'},
  {label:'Humidity',   border:'#4facfe',bg:'rgba(79,172,254,.12)'},
  {label:'Heat Index', border:'#fa709a',bg:'rgba(250,112,154,.12)'},
  {label:'Dew Point',  border:'#f093fb',bg:'rgba(240,147,251,.12)'}
];
function initCharts(){
  var ctx=document.getElementById('combinedChart').getContext('2d');
  C=new Chart(ctx,{
    type:'line',
    data:{labels:[],datasets:DS_CFG.map(function(c){return{
      label:c.label,data:[],borderColor:c.border,backgroundColor:c.bg,
      tension:.4,fill:true,borderWidth:3,pointRadius:3,pointHoverRadius:6
    };})},
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
            title:function(c){return '\u23F1 '+c[0].label;},
            label:function(c){var u=['\u00B0C','%','\u00B0C','\u00B0C'];return' '+c.dataset.label+': '+c.parsed.y.toFixed(1)+' '+u[c.datasetIndex];}
          }
        }
      },
      scales:{
        x:{grid:{color:'rgba(0,0,0,.05)',drawBorder:false},ticks:{font:{size:11},maxRotation:0,autoSkip:true,maxTicksLimit:10,color:'#666'}},
        y:{position:'right',grid:{drawBorder:false,color:'rgba(0,0,0,.05)'},ticks:{font:{size:12},color:'#666'},title:{display:true,text:'\u00B0C / %',color:'#666'}}
      },
      animation:{duration:600,easing:'easeInOutQuart'}
    }
  });
  syncChartTheme();
}
function toggleSeries(idx,btn){
  var m=C.getDatasetMeta(idx);
  m.hidden=!m.hidden;
  btn.classList.toggle('active',!m.hidden);
  C.update();
}
function setRange(minutes,btn){
  rangeMinutes=minutes;
  document.querySelectorAll('.tr-btn').forEach(function(b){b.classList.remove('active');});
  btn.classList.add('active');
  renderChart();
}
function sliceByRange(arr){
  if(!rangeMinutes||!arr.length)return arr;
  var pts=Math.round(rangeMinutes*60/15);
  return arr.slice(-Math.min(pts,arr.length));
}
function renderChart(){
  if(!C||!rawHistory.labels.length)return;
  var c2fA=function(a){return F?a.map(function(v){return v*9/5+32;}):a;};
  C.data.labels=sliceByRange(rawHistory.labels);
  C.data.datasets[0].data=c2fA(sliceByRange(rawHistory.temp));
  C.data.datasets[1].data=sliceByRange(rawHistory.humid);
  C.data.datasets[2].data=c2fA(sliceByRange(rawHistory.heat));
  C.data.datasets[3].data=c2fA(sliceByRange(rawHistory.dew));
  C.options.scales.y.title.text=F?'\u00B0F / %':'\u00B0C / %';
  C.update();
  document.getElementById('updateTimeCombined').textContent=new Date().toLocaleTimeString();
}

/* ===== SPARKLINES ===== */
function initSparklines(){
  var cfgs=['spkTemp','spkHumid','spkDew','spkHeat'];
  cfgs.forEach(function(id){
    var el=document.getElementById(id);
    if(!el)return;
    sparkCharts[id]=new Chart(el.getContext('2d'),{
      type:'line',
      data:{labels:[],datasets:[{data:[],borderColor:'rgba(255,255,255,.9)',backgroundColor:'rgba(255,255,255,.15)',borderWidth:2,pointRadius:0,tension:.4,fill:true}]},
      options:{responsive:true,maintainAspectRatio:false,plugins:{legend:{display:false},tooltip:{enabled:false}},scales:{x:{display:false},y:{display:false}},animation:{duration:400},events:[]}
    });
  });
}
function updateSparklines(){
  if(!rawHistory.temp.length)return;
  var n=20;
  var sl=function(a){return a.slice(-n);};
  var c2fA=function(a){return F?a.map(function(v){return v*9/5+32;}):a;};
  var pairs=[['spkTemp',c2fA(sl(rawHistory.temp))],['spkHumid',sl(rawHistory.humid)],['spkDew',c2fA(sl(rawHistory.dew))],['spkHeat',c2fA(sl(rawHistory.heat))]];
  pairs.forEach(function(p){
    var sc=sparkCharts[p[0]];
    if(!sc)return;
    sc.data.labels=p[1].map(function(_,i){return i;});
    sc.data.datasets[0].data=p[1];
    sc.update('none');
  });
}

/* ===== CARD COLOURS ===== */
function lerpHex(a,b,t){
  var h=function(s){return parseInt(s,16);};
  var r=function(v){return Math.round(v).toString(16).padStart(2,'0');};
  var ar=h(a.slice(1,3)),ag=h(a.slice(3,5)),ab=h(a.slice(5,7));
  var br=h(b.slice(1,3)),bg=h(b.slice(3,5)),bb=h(b.slice(5,7));
  return '#'+r(ar+(br-ar)*t)+r(ag+(bg-ag)*t)+r(ab+(bb-ab)*t);
}
function valueToGradient(val,stops){
  var n=stops.length;
  if(val<=stops[0].at)return 'linear-gradient(135deg,'+stops[0].c1+','+stops[0].c2+')';
  if(val>=stops[n-1].at)return 'linear-gradient(135deg,'+stops[n-1].c1+','+stops[n-1].c2+')';
  for(var i=0;i<n-1;i++){
    if(val>=stops[i].at&&val<stops[i+1].at){
      var t=(val-stops[i].at)/(stops[i+1].at-stops[i].at);
      return 'linear-gradient(135deg,'+lerpHex(stops[i].c1,stops[i+1].c1,t)+','+lerpHex(stops[i].c2,stops[i+1].c2,t)+')';
    }
  }
}
var TEMP_STOPS=[{at:0,c1:'#00c6fb',c2:'#005bea'},{at:10,c1:'#43e97b',c2:'#38f9d7'},{at:22,c1:'#667eea',c2:'#764ba2'},{at:30,c1:'#f7971e',c2:'#ffd200'},{at:40,c1:'#f5515f',c2:'#9f041b'}];
var HUMID_STOPS=[{at:0,c1:'#f7971e',c2:'#ffd200'},{at:30,c1:'#96fbc4',c2:'#f9f586'},{at:50,c1:'#4facfe',c2:'#00f2fe'},{at:75,c1:'#0575e6',c2:'#021b79'},{at:100,c1:'#0c0c0c',c2:'#1a3a5c'}];
var HEAT_STOPS=[{at:0,c1:'#00c6fb',c2:'#005bea'},{at:20,c1:'#f7971e',c2:'#ffd200'},{at:28,c1:'#fa709a',c2:'#fee140'},{at:35,c1:'#f5515f',c2:'#fa7740'},{at:45,c1:'#8b0000',c2:'#ff2400'}];
var DEW_STOPS=[{at:-10,c1:'#a8edea',c2:'#fed6e3'},{at:10,c1:'#f093fb',c2:'#f5576c'},{at:20,c1:'#4facfe',c2:'#00f2fe'},{at:26,c1:'#1a6dff',c2:'#c822ff'}];
function updateCardColors(tc,h,hc,dc){
  document.querySelector('.temp-card').style.background=valueToGradient(tc,TEMP_STOPS);
  document.querySelector('.humidity-card').style.background=valueToGradient(h,HUMID_STOPS);
  document.querySelector('.heatindex-card').style.background=valueToGradient(hc,HEAT_STOPS);
  document.querySelector('.dewpoint-card').style.background=valueToGradient(dc,DEW_STOPS);
}

/* ===== WIFI BARS ===== */
function updateWifiBars(rssi){
  var active,color,label;
  if(rssi>=-55){active=4;color='#28a745';label='Excellent';}
  else if(rssi>=-65){active=3;color='#5cb85c';label='Good';}
  else if(rssi>=-75){active=2;color='#ffc107';label='Fair';}
  else if(rssi>=-85){active=1;color='#ff9800';label='Weak';}
  else{active=1;color='#dc3545';label='Very weak';}
  var dim='rgba(150,150,150,0.18)';
  ['wb1','wb2','wb3','wb4'].forEach(function(id,i){
    document.getElementById(id).style.backgroundColor=i<active?color:dim;
  });
  document.getElementById('rssiLabel').textContent=label;
}

/* ===== CHIP TEMP ===== */
function updateRamDisplay(freeKB,totalKB,usedPct){
  document.getElementById('freeHeap').textContent=freeKB.toFixed(0)+' KB free';
  document.getElementById('ramUsedPct').textContent=usedPct.toFixed(1)+'% used';
  document.getElementById('ramFreePct').textContent=(100-usedPct).toFixed(1)+'% free';
  var bar=document.getElementById('ramBarFill');
  bar.style.width=usedPct+'%';
  bar.className='ram-bar-fill '+(usedPct>=80?'ram-high':usedPct>=60?'ram-mid':'ram-ok');
}
function updateChipTemp(val){
  var el=document.getElementById('chipTemp');
  if(!el)return;
  var display=(F?(val*9/5+32):val).toFixed(1)+(F?'\u00B0F':'\u00B0C');
  el.textContent=display;
  el.style.color=val>=80?'#dc3545':val>=60?'#ff9800':val>=45?'#ffc107':'#28a745';
}

/* ===== DATA ===== */
function c2f(c){return c*9/5+32;}
function toggleUnit(){F=!F;updateDisplay();}
function updateDisplay(){
  var u=F?'\u00B0F':'\u00B0C';
  document.querySelectorAll('#tempUnit,#minTempUnit,#maxTempUnit,#avgTempUnit,#dewPointUnit,#heatIndexUnit').forEach(function(e){e.textContent=u;});
  renderChart();
  updateData();
}
function getComfort(v,isTemp){
  if(isTemp){
    if(v>=20&&v<=24)return{l:'excellent',t:'\u2705 Optimal'};
    if(v>=18&&v<=26)return{l:'good',t:'\uD83D\uDC4D Comfortable'};
    return{l:'poor',t:'\u274C Uncomfortable'};
  }else{
    if(v>=40&&v<=60)return{l:'excellent',t:'\u2705 Optimal'};
    if(v>=30&&v<=70)return{l:'good',t:'\uD83D\uDC4D Normal'};
    return{l:'poor',t:'\u274C Uncomfortable'};
  }
}
function updateData(){
  fetch('/data').then(function(r){return r.json();}).then(function(d){
    var t=F?c2f(d.temperature):d.temperature;
    var minT=F?c2f(d.minTemp):d.minTemp,maxT=F?c2f(d.maxTemp):d.maxTemp,avgT=F?c2f(d.avgTemp):d.avgTemp;
    var dewP=F?c2f(d.dewPoint):d.dewPoint,heatI=F?c2f(d.heatIndex):d.heatIndex;
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
    updateCardColors(d.temperature,d.humidity,d.heatIndex,d.dewPoint);
    setWeatherFromData(d.temperature,d.humidity);
    var tc=getComfort(d.temperature,true),hc=getComfort(d.humidity,false);
    var te=document.getElementById('tempComfort');te.textContent=tc.t;te.className='comfort-indicator comfort-'+tc.l;
    var he=document.getElementById('humidComfort');he.textContent=hc.t;he.className='comfort-indicator comfort-'+hc.l;
    document.getElementById('lastUpdate').textContent='Updated: '+new Date().toLocaleTimeString('ru-RU');
    startCountdown();
    errCnt=0;
    document.getElementById('statusBadge').className='status online';
    document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Connected</span>';
  }).catch(function(){
    errCnt++;
    if(errCnt>2){
      document.getElementById('statusBadge').className='status offline';
      document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Disconnected</span>';
    }
  });
}
function updateStats(){
  fetch('/stats').then(function(r){return r.json();}).then(function(d){
    document.getElementById('uptime').textContent=d.uptime;
    // RAM: use server-calculated percentage directly
    var usedPct=d.heapUsagePct||0;
    var freePct=(100-usedPct);
    document.getElementById('freeHeap').textContent=d.freeHeap;
    document.getElementById('ramUsedPct').textContent=usedPct.toFixed(1)+'% used';
    document.getElementById('ramFreePct').textContent=freePct.toFixed(1)+'% free';
    var bar=document.getElementById('ramBarFill');
    bar.style.width=usedPct+'%';
    bar.className='ram-bar-fill '+(usedPct>=80?'ram-high':usedPct>=60?'ram-mid':'ram-ok');
    document.getElementById('cpuUsage').textContent=d.cpuUsage+'%';
    if(d.chipTemp!=null&&d.chipTemp>0)updateChipTemp(d.chipTemp);
    document.getElementById('ssid').textContent=d.ssid||'--';
    var rv=parseInt(d.rssi);
    document.getElementById('rssi').textContent=d.rssi+' dBm';
    updateWifiBars(rv);
    document.getElementById('ipAddr').textContent=d.ip;
    if(d.battery){
      var b=d.battery;
      var pct=Math.max(0,Math.min(100,b.percent));
      var fill=document.getElementById('batteryFill');
      fill.style.width=pct+'%';
      fill.className='battery-fill '+(b.isCharging?'charging':b.isCritical?'critical':b.isLow?'low':pct<60?'mid':'good');
      var pTxt=pct+'%';
      if(b.isCritical)pTxt='\u203C\uFE0F '+pTxt;
      else if(b.isLow)pTxt='\u26A0\uFE0F '+pTxt;
      else if(b.isCharging)pTxt='\u26A1 '+pTxt;
      else if(pct>=80)pTxt='\uD83D\uDFE2 '+pTxt;
      document.getElementById('batteryPercent').textContent=pTxt;
      document.getElementById('batteryVoltage').textContent=b.voltage+'V';
      var sTxt=b.source;
      if(b.isCharging)sTxt='Charging...';
      else if(b.status==='Fully charged')sTxt='Full';
      document.getElementById('batterySource').textContent=sTxt;
    }
  }).catch(function(e){console.error(e);});
}
function updateHistory(){
  fetch('/history').then(function(r){return r.json();}).then(function(d){
    rawHistory.labels=d.labels;
    rawHistory.temp=d.temp;
    rawHistory.humid=d.humid;
    rawHistory.heat=d.heat||d.temp;
    rawHistory.dew=d.dew||d.temp;
    renderChart();
    updateSparklines();
  }).catch(function(e){console.error(e);});
}
function resetMinMax(){if(confirm('Reset min/max?')){fetch('/reset').then(function(){updateData();});}}
function rebootDevice(){if(confirm('Reboot?')){fetch('/reboot');}}
function exportCSV(){
  var td=C.data.datasets;
  var csv='Time,Temp,Humid,HeatIndex,DewPoint\n';
  for(var i=0;i<C.data.labels.length;i++)
    csv+=C.data.labels[i]+','+(td[0].data[i]||'')+','+(td[1].data[i]||'')+','+(td[2].data[i]||'')+','+(td[3].data[i]||'')+'\n';
  var a=document.createElement('a');
  a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));
  a.download='weather.csv';a.click();
}
function exportJSON(){
  var o={labels:C.data.labels,temp:C.data.datasets[0].data,humid:C.data.datasets[1].data,heat:C.data.datasets[2].data,dew:C.data.datasets[3].data};
  var a=document.createElement('a');
  a.href=URL.createObjectURL(new Blob([JSON.stringify(o)],{type:'application/json'}));
  a.download='weather.json';a.click();
}

/* ===== PARTICLES ===== */
var particlesEnabled=true;
var pRAF=null;
var pCanvas=null,pCtx=null;
var particles=[];
var pMode='none';
var pForced=false;

function initParticles(){
  pCanvas=document.getElementById('particleCanvas');
  pCtx=pCanvas.getContext('2d');
  pCanvas.width=window.innerWidth;
  pCanvas.height=window.innerHeight;
  window.addEventListener('resize',function(){
    if(pCanvas){pCanvas.width=window.innerWidth;pCanvas.height=window.innerHeight;}
  });
  var prefs=JSON.parse(localStorage.getItem(BLOCK_KEY)||'{}');
  if(prefs.pForced&&prefs.pMode&&prefs.pMode!=='none'){
    pForced=true;
    setParticleBtn(prefs.pMode);
    updateParticleMode(prefs.pMode);
  }
}
function setParticleBtn(mode){
  ['pbOff','pbDrizzle','pbRain','pbHeavy','pbSnow'].forEach(function(id){
    var el=document.getElementById(id);
    if(el)el.classList.remove('active');
  });
  var map={none:'pbOff',drizzle:'pbDrizzle',rain:'pbRain',heavy:'pbHeavy',snow:'pbSnow'};
  var el=document.getElementById(map[mode]);
  if(el)el.classList.add('active');
}
function forceParticle(mode,btn){
  pForced=(mode!=='none');
  setParticleBtn(mode);
  updateParticleMode(mode);
  var prefs=JSON.parse(localStorage.getItem(BLOCK_KEY)||'{}');
  prefs.pForced=pForced;prefs.pMode=mode;
  localStorage.setItem(BLOCK_KEY,JSON.stringify(prefs));
}
function updateParticleMode(mode){
  pMode=mode;
  if(!pCanvas)return;
  particles=[];
  if(mode==='none'){
    pCanvas.classList.remove('visible');
    if(pRAF){cancelAnimationFrame(pRAF);pRAF=null;}
    return;
  }
  pCanvas.classList.add('visible');
  var count={rain:150,heavy:220,drizzle:60,snow:90}[mode]||80;
  for(var i=0;i<count;i++)particles.push(makeParticle(mode,true));
  if(!pRAF)animateParticles();
}
function makeParticle(mode,randomY){
  var W=pCanvas.width,H=pCanvas.height;
  if(mode==='rain'||mode==='heavy'||mode==='drizzle'){
    var heavy=(mode==='heavy');
    var vy=heavy?14+Math.random()*8:mode==='rain'?9+Math.random()*5:3+Math.random()*2.5;
    return{x:Math.random()*(W+100)-50,y:randomY?Math.random()*H*1.2:-Math.random()*50,vy:vy,vx:vy*0.18,len:vy*(mode==='drizzle'?1.2:1.8),w:heavy?2:mode==='rain'?1.5:1,alpha:heavy?0.55+Math.random()*0.25:mode==='rain'?0.35+Math.random()*0.25:0.2+Math.random()*0.2,hue:heavy?200:210};
  }else{
    return{x:Math.random()*W,y:randomY?Math.random()*H:-10,vy:0.4+Math.random()*1.0,vx:(Math.random()-0.5)*0.3,r:1.5+Math.random()*3.5,alpha:0.55+Math.random()*0.35,drift:Math.random()*Math.PI*2,driftSpeed:0.008+Math.random()*0.015,sparkle:Math.random()*Math.PI*2};
  }
}
function animateParticles(){
  if(!pCanvas)return;
  pRAF=requestAnimationFrame(animateParticles);
  var W=pCanvas.width,H=pCanvas.height,now=performance.now();
  pCtx.clearRect(0,0,W,H);
  for(var i=0;i<particles.length;i++){
    var p=particles[i];
    if(pMode==='snow'){
      p.drift+=p.driftSpeed;
      p.x+=Math.sin(p.drift)*0.6+p.vx;p.y+=p.vy;
      var sp=0.7+0.3*Math.sin(now*0.003+p.sparkle);
      var grd=pCtx.createRadialGradient(p.x,p.y,0,p.x,p.y,p.r*2.5);
      grd.addColorStop(0,'rgba(220,240,255,'+(p.alpha*sp*0.9)+')');
      grd.addColorStop(0.5,'rgba(200,225,255,'+(p.alpha*sp*0.3)+')');
      grd.addColorStop(1,'rgba(200,225,255,0)');
      pCtx.beginPath();pCtx.arc(p.x,p.y,p.r*2.5,0,Math.PI*2);pCtx.fillStyle=grd;pCtx.fill();
      pCtx.beginPath();pCtx.arc(p.x,p.y,p.r,0,Math.PI*2);pCtx.fillStyle='rgba(235,248,255,'+(p.alpha*sp)+')';pCtx.fill();
      if(p.y>H+12||p.x<-12||p.x>W+12)particles[i]=makeParticle('snow',false);
    }else{
      p.x+=p.vx;p.y+=p.vy;
      var tx=p.x-p.vx*(p.len/p.vy),ty=p.y-p.len;
      var grad=pCtx.createLinearGradient(tx,ty,p.x,p.y);
      grad.addColorStop(0,'hsla('+p.hue+',80%,80%,0)');
      grad.addColorStop(1,'hsla('+p.hue+',80%,88%,'+p.alpha+')');
      pCtx.strokeStyle=grad;pCtx.lineWidth=p.w;pCtx.lineCap='round';
      pCtx.beginPath();pCtx.moveTo(tx,ty);pCtx.lineTo(p.x,p.y);pCtx.stroke();
      if(p.y>H+20||p.x>W+50)particles[i]=makeParticle(pMode,false);
    }
  }
}
function setWeatherFromData(tempC,humid){
  if(pForced)return;
  var mode;
  if(tempC<=2)mode='snow';
  else if(humid>=90)mode='heavy';
  else if(humid>=75)mode='rain';
  else if(humid>=60)mode='drizzle';
  else mode='none';
  if(mode!==pMode)updateParticleMode(mode);
}

/* ===== COUNTDOWN ===== */
var CD_TOTAL=10000;
var cdStart=0;
var cdRAF=null;
function startCountdown(){
  cdStart=performance.now();
  if(!cdRAF)tickCountdown();
}
function tickCountdown(){
  cdRAF=requestAnimationFrame(tickCountdown);
  var elapsed=performance.now()-cdStart;
  var remaining=Math.max(0,CD_TOTAL-elapsed);
  var pct=(remaining/CD_TOTAL)*100;
  var secs=Math.ceil(remaining/1000);
  var fill=document.getElementById('cdFill');
  var text=document.getElementById('cdText');
  if(!fill||!text)return;
  fill.style.width=pct+'%';
  fill.classList.toggle('warn',secs<=3&&remaining>0);
  text.textContent=remaining>50?'in '+secs+'s':'updating...';
}

/* ===== INIT ===== */
document.addEventListener('DOMContentLoaded',function(){
  applyTheme();
  applyBlockPrefs();
  initCharts();
  initSparklines();
  initParticles();
  initWebSocket();
  updateData();
  updateStats();
  updateHistory();
  startCountdown();
  iU=setInterval(updateData,10000);
  iS=setInterval(updateStats,10000);
  iH=setInterval(updateHistory,15000);
});
</script>
</body>
</html>
)rawliteral";

#endif

