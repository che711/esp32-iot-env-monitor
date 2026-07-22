#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ENV Station</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
<style>
/* ============ TOKENS — light / purple gradient (default) ============ */
:root{
  --bg-page:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  /* Мягкий лавандовый вместо чистого белого — не режет глаз на фиолетовом фоне */
  --card:rgba(244,242,251,.96);
  --card-2:#eae7f6;
  --line:rgba(102,126,234,.18);
  --line-soft:rgba(102,126,234,.10);
  --text:#2d3348;
  --sub:#5b6379;
  --muted:#9aa1b5;
  --acc-temp:#f5576c;
  --acc-humid:#4facfe;
  --acc-heat:#f0a028;
  --acc-dew:#22b8c8;
  --ok:#2eb872;
  --warn:#e6a700;
  --danger:#e5484d;
  --console:#1e2233;
  --chip:#e7e4f4;
  --grad-brand:linear-gradient(135deg,#667eea,#764ba2);
  --shadow:0 4px 16px rgba(31,38,135,.14);
  --shadow-hover:0 12px 32px rgba(31,38,135,.22);
  --font-ui:'Segoe UI',system-ui,-apple-system,Roboto,sans-serif;
  --font-mono:ui-monospace,'Cascadia Code','SF Mono',Consolas,'Roboto Mono',monospace;
}
body.dark{
  --bg-page:linear-gradient(135deg,#1a1a2e 0%,#16213e 50%,#0f3460 100%);
  --card:rgba(30,34,54,.96);
  --card-2:#252840;
  --line:rgba(159,168,218,.16);
  --line-soft:rgba(159,168,218,.08);
  --text:#e8eaf6;
  --sub:#9fa8da;
  --muted:#7986cb;
  --acc-temp:#ff7b8a;
  --acc-humid:#6cb8ff;
  --acc-heat:#ffc04d;
  --acc-dew:#43d6e6;
  --ok:#69f0ae;
  --warn:#ffd54f;
  --danger:#ff6b6b;
  --console:#12141f;
  --chip:#252840;
  --grad-brand:linear-gradient(135deg,#9fa8da,#b39ddb);
  --shadow:0 4px 16px rgba(0,0,0,.4);
  --shadow-hover:0 12px 32px rgba(0,0,0,.5);
}

*{margin:0;padding:0;box-sizing:border-box}
html{color-scheme:light}
body.dark{color-scheme:dark}
body{
  font-family:var(--font-ui);
  background:var(--bg-page);
  background-attachment:fixed;
  color:var(--text);
  min-height:100vh;
  padding:20px 16px 48px;
  transition:color .3s;
  -webkit-font-smoothing:antialiased;
}
.container{max-width:840px;margin:0 auto;position:relative;z-index:1}
:focus-visible{outline:2px solid #fff;outline-offset:2px;border-radius:4px}
::selection{background:rgba(102,126,234,.28)}
.mono{font-family:var(--font-mono);font-variant-numeric:tabular-nums}

/* ============ HEADER ============ */
.header{
  background:var(--card);backdrop-filter:blur(10px);border-radius:20px;
  padding:28px 26px 22px;margin-bottom:18px;box-shadow:var(--shadow-hover);
  text-align:center;transition:background .3s;
}
.eyebrow{
  font-size:11px;letter-spacing:.16em;font-weight:600;
  color:var(--muted);text-transform:uppercase;
  display:inline-flex;align-items:center;gap:8px;
}
.eyebrow .live-dot{width:7px;height:7px;border-radius:50%;background:var(--ok);animation:pulse 2.4s ease-in-out infinite}
.header h1{
  font-size:clamp(26px,5vw,36px);font-weight:700;letter-spacing:-.01em;margin:8px 0 14px;
  background:var(--grad-brand);-webkit-background-clip:text;background-clip:text;
  -webkit-text-fill-color:transparent;color:transparent;
}
.status-container{display:flex;gap:10px;flex-wrap:wrap;align-items:center;justify-content:center}
.status{
  display:inline-flex;align-items:center;gap:8px;padding:7px 18px;border-radius:25px;
  font-size:12.5px;font-weight:600;background:var(--chip);color:var(--sub);
}
.status.online{color:#0f7a45;background:#d9f4e6}
.status.offline{color:#a02830;background:#fbdfe1}
body.dark .status.online{color:#69f0ae;background:rgba(46,184,114,.16)}
body.dark .status.offline{color:#ff8a90;background:rgba(229,72,77,.16)}
.status-dot{width:8px;height:8px;border-radius:50%;background:currentColor;animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.45}}
#lastUpdateBadge{font-variant-numeric:tabular-nums}
.countdown-wrap{margin-top:16px;display:flex;align-items:center;gap:10px}
.countdown-bar-track{flex:1;height:4px;background:var(--line-soft);border-radius:2px;overflow:hidden}
.countdown-bar-fill{height:100%;border-radius:2px;background:var(--grad-brand)}
.countdown-bar-fill.warn{background:linear-gradient(90deg,#f7971e,#ffd200)}
.countdown-text{font-family:var(--font-mono);font-size:11px;color:var(--muted);white-space:nowrap;min-width:70px;text-align:right}

/* ============ FLOATING CONTROLS ============ */
.fab{
  position:fixed;right:18px;z-index:999;width:46px;height:46px;border-radius:50%;
  border:none;background:var(--card);backdrop-filter:blur(10px);color:var(--sub);
  font-size:19px;cursor:pointer;box-shadow:var(--shadow);
  display:flex;align-items:center;justify-content:center;transition:transform .25s,box-shadow .25s;
}
.fab:hover{transform:scale(1.1) rotate(16deg);box-shadow:var(--shadow-hover)}
.theme-toggle{top:18px}
.settings-btn{top:74px}

/* ============ CARDS ============ */
.chart-row{display:grid;grid-template-columns:1fr;gap:16px;margin-bottom:16px}
.chart-row.double{grid-template-columns:1fr 1fr}
.card,.chart-card{
  background:var(--card);backdrop-filter:blur(10px);border-radius:20px;
  padding:22px;box-shadow:var(--shadow);
  transition:transform .25s ease,box-shadow .25s ease,background .3s;
}
.card:hover,.chart-card:hover{transform:translateY(-3px);box-shadow:var(--shadow-hover)}
h3{font-size:15px;font-weight:700;letter-spacing:.01em;display:flex;align-items:center;gap:8px;color:var(--text)}

/* --- Channel (sensor) cards --- */
.sensor-card{--acc:var(--sub);position:relative;overflow:hidden}
.sensor-card::before{
  content:'';position:absolute;top:0;left:0;right:0;height:4px;
  background:linear-gradient(90deg,var(--acc),color-mix(in srgb,var(--acc) 25%,transparent));
  transition:background .8s ease;
}
.sensor-header{display:flex;align-items:center;gap:9px;margin-bottom:14px}
.sensor-header svg{width:17px;height:17px;stroke:var(--acc);transition:stroke .8s ease;flex-shrink:0}
.sensor-label{
  font-size:11.5px;letter-spacing:.14em;
  text-transform:uppercase;color:var(--sub);font-weight:700;
}
.sensor-value{
  font-variant-numeric:tabular-nums;
  font-size:clamp(40px,7vw,52px);font-weight:700;letter-spacing:-.02em;line-height:1;
  margin:4px 0 14px;color:var(--text);
}
.sensor-unit{font-size:.42em;color:var(--acc);font-weight:600;margin-left:4px;transition:color .8s ease}
.sensor-description{font-size:12.5px;color:var(--muted);margin-bottom:12px;line-height:1.45}
.minmax{
  display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:12px;
  padding:12px;background:var(--card-2);border-radius:14px;
}
.minmax>div{display:flex;flex-direction:column;gap:3px;align-items:center}
.mm-label{font-size:9.5px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted);font-weight:700}
.minmax-value,.avg-value{font-variant-numeric:tabular-nums;font-size:15px;font-weight:700;color:var(--text)}
.avg-value{color:var(--acc);transition:color .8s ease}
.comfort-indicator{
  display:inline-flex;align-items:center;gap:7px;padding:6px 14px;border-radius:25px;
  font-size:11.5px;font-weight:700;margin-bottom:12px;
}
.comfort-indicator::before{content:'';width:6px;height:6px;border-radius:50%;background:currentColor}
.comfort-excellent{color:#0f7a45;background:#d9f4e6}
.comfort-good{color:#8a6400;background:#faf0cf}
.comfort-poor{color:#a02830;background:#fbdfe1}
body.dark .comfort-excellent{color:#69f0ae;background:rgba(46,184,114,.16)}
body.dark .comfort-good{color:#ffd54f;background:rgba(230,167,0,.16)}
body.dark .comfort-poor{color:#ff8a90;background:rgba(229,72,77,.16)}
.sparkline-wrap{padding-top:12px;border-top:1px solid var(--line-soft)}
.sparkline-label{font-size:9.5px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted);font-weight:700;margin-bottom:6px}
canvas.sparkline{height:40px!important;min-height:40px!important;max-height:40px!important;width:100%!important;display:block}

/* ============ FRIENDLY COLOURED BLOCKS (light theme only) ============ */
/* Шапка Environment — мягкий сине-фиолетовый градиент вместо простого белого */
body:not(.dark) .header{
  background:linear-gradient(150deg,#f2efff 0%,#eaf3ff 55%,#ffffff 100%);
  border:1px solid rgba(102,126,234,.16);
}
/* Каждая сенсорная карточка тонируется в свой акцентный цвет (--acc),
   который к тому же меняется вслед за значением (updateCardColors). */
body:not(.dark) .sensor-card{
  background:linear-gradient(155deg,
    color-mix(in srgb,var(--acc) 26%,#fff) 0%,
    color-mix(in srgb,var(--acc) 14%,#fff) 55%,
    color-mix(in srgb,var(--acc) 8%,#fff) 100%);
  border:1px solid color-mix(in srgb,var(--acc) 34%,transparent);
}
body:not(.dark) .sensor-card .sensor-label{color:color-mix(in srgb,var(--acc) 60%,#3a3a48)}
body:not(.dark) .sensor-card .minmax{
  background:color-mix(in srgb,var(--acc) 12%,#fff);
  border:1px solid color-mix(in srgb,var(--acc) 18%,transparent);
}
body:not(.dark) .sensor-card .sparkline-wrap{border-top-color:color-mix(in srgb,var(--acc) 22%,transparent)}

/* --- Unit toggle (C/F) --- */
.temp-unit-toggle{display:flex;align-items:center;gap:9px;margin-bottom:12px}
.toggle-label{font-size:12.5px;color:var(--muted);font-weight:700}
.toggle-switch{position:relative;display:inline-block;width:42px;height:23px}
.toggle-switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;inset:0;background:var(--chip);transition:.3s;border-radius:23px;box-shadow:inset 0 1px 3px rgba(0,0,0,.08)}
.slider:before{position:absolute;content:'';height:17px;width:17px;left:3px;top:3px;background:#fff;transition:.3s;border-radius:50%;box-shadow:0 1px 3px rgba(0,0,0,.25)}
input:checked+.slider{background:var(--acc-temp)}
input:checked+.slider:before{transform:translateX(19px)}

/* ============ SYSTEM GRID ============ */
.info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px;margin-top:14px}
.info-item{
  background:var(--card-2);
  padding:14px;border-radius:14px;
  transition:transform .2s;
}
.info-item:hover{transform:translateY(-2px)}
.info-label{
  font-size:9.5px;letter-spacing:.14em;font-weight:700;
  text-transform:uppercase;color:var(--muted);margin-bottom:7px;
}
.info-value{font-variant-numeric:tabular-nums;font-weight:700;font-size:15px;color:var(--text)}

/* RAM bar */
.ram-bar-track{height:5px;border-radius:3px;background:var(--line-soft);margin-top:8px;overflow:hidden}
.ram-bar-fill{height:100%;border-radius:3px;transition:width .6s ease,background .4s}
.ram-ok{background:linear-gradient(90deg,#2eb872,#5fd99a)}
.ram-mid{background:linear-gradient(90deg,#e6a700,#ffd200)}
.ram-high{background:linear-gradient(90deg,#e5484d,#ff7b8a)}
.ram-stats{display:flex;justify-content:space-between;font-size:9.5px;color:var(--muted);margin-top:5px;font-weight:600}

/* Battery */
.battery-widget{display:flex;align-items:center;gap:12px;margin-top:4px}
.battery-icon{position:relative;width:42px;height:21px;border:2px solid var(--sub);border-radius:5px;flex-shrink:0;padding:2px}
.battery-icon::after{content:'';position:absolute;right:-6px;top:50%;transform:translateY(-50%);width:4px;height:9px;background:var(--sub);border-radius:0 2px 2px 0}
.battery-fill{height:100%;border-radius:2px;transition:width .6s ease,background .4s ease}
.battery-fill.critical{background:var(--danger)}
.battery-fill.low{background:var(--warn)}
.battery-fill.mid{background:var(--acc-heat)}
.battery-fill.good{background:var(--ok)}
.battery-fill.charging{background:var(--ok);animation:chargepulse 1.4s ease-in-out infinite}
@keyframes chargepulse{0%,100%{opacity:1}50%{opacity:.5}}
.battery-text{font-variant-numeric:tabular-nums;font-size:14.5px;font-weight:700;color:var(--text)}
.battery-sub{font-size:11px;color:var(--muted);margin-top:2px}

/* WiFi bars */
.wifi-widget{display:flex;align-items:center;gap:10px}
.wifi-bars{display:flex;align-items:flex-end;gap:3px;height:18px}
.wifi-bar{width:5px;border-radius:2px 2px 0 0;transition:background-color .6s ease;background:var(--line)}
.wifi-bar:nth-child(1){height:5px}
.wifi-bar:nth-child(2){height:9px}
.wifi-bar:nth-child(3){height:13px}
.wifi-bar:nth-child(4){height:18px}
.wifi-rssi-label{font-size:10px;color:var(--muted);margin-top:2px}

/* ============ BUTTONS ============ */
.buttons{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:10px;margin-top:16px}
.btn{
  padding:11px 14px;border:none;border-radius:12px;
  font-size:13px;font-weight:700;font-family:var(--font-ui);
  cursor:pointer;background:var(--chip);color:var(--sub);
  display:flex;align-items:center;justify-content:center;gap:7px;
  transition:transform .2s,box-shadow .2s,background .2s,color .2s;
}
.btn:hover{
  background:var(--grad-brand);color:#fff;
  transform:translateY(-2px);box-shadow:0 6px 18px rgba(102,126,234,.35);
}
.btn-danger:hover{background:linear-gradient(135deg,#f5576c,#e5484d);box-shadow:0 6px 18px rgba(229,72,77,.35)}

/* ============ SERIAL MONITOR ============ */
.log-console{
  background:var(--console);color:#b8c4cf;
  font-family:var(--font-mono);font-size:12.5px;line-height:1.55;
  padding:14px 16px;border-radius:14px;
  height:420px;overflow-y:auto;margin-top:12px;
}
.log-console::-webkit-scrollbar{width:8px}
.log-console::-webkit-scrollbar-track{background:transparent}
.log-console::-webkit-scrollbar-thumb{background:#3a4258;border-radius:4px}
.log-line{margin:1px 0;white-space:pre-wrap;word-break:break-all;padding-left:10px;border-left:2px solid transparent}
.log-line.hidden{display:none}
.log-error{color:#ff9088;border-left-color:#ff9088}
.log-warning{color:#e5c07b;border-left-color:#e5c07b}
.log-info{color:#8fbcbb}
.log-success{color:#7cc7ff;border-left-color:#7cc7ff}
.ws-status{display:inline-block;width:8px;height:8px;border-radius:50%;margin:0 4px 0 6px}
.ws-connected{background:var(--ok)}
.ws-disconnected{background:var(--danger)}
.log-filters{display:flex;flex-wrap:wrap;gap:6px;margin-top:12px;align-items:center}
.log-filter-btn{
  padding:5px 13px;border:none;border-radius:25px;
  font-size:10.5px;font-weight:700;letter-spacing:.06em;
  cursor:pointer;transition:all .15s;background:var(--chip);color:var(--muted);
}
.log-filter-btn.active{color:#fff;background:var(--grad-brand)}
.log-filter-btn:hover{color:var(--text)}
.log-filter-btn.active:hover{color:#fff}
.log-counter{font-family:var(--font-mono);font-size:10px;color:var(--muted);margin-left:auto}

/* ============ CHART ============ */
.chart-head{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;margin-bottom:12px}
.chart-toggles{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:14px}
.chart-toggle-btn{
  display:flex;align-items:center;gap:7px;padding:6px 14px;
  border:none;border-radius:25px;
  font-size:12px;font-weight:700;cursor:pointer;transition:all .15s;
  background:var(--chip);color:var(--muted);user-select:none;font-family:var(--font-ui);
}
.chart-toggle-btn .dot{width:8px;height:8px;border-radius:50%;flex-shrink:0;opacity:.3;transition:opacity .2s}
.chart-toggle-btn.active{color:var(--text);background:var(--card-2);box-shadow:inset 0 0 0 1.5px var(--line)}
.chart-toggle-btn.active .dot{opacity:1}
.chart-toggle-btn:hover{color:var(--text)}
.toggle-temp .dot{background:var(--acc-temp)}
.toggle-humid .dot{background:var(--acc-humid)}
.toggle-heat .dot{background:var(--acc-heat)}
.toggle-dew .dot{background:var(--acc-dew)}
.time-range{display:flex;gap:5px}
.tr-btn{
  padding:5px 13px;border:none;border-radius:25px;
  font-size:11px;font-weight:700;cursor:pointer;
  background:var(--chip);color:var(--muted);transition:all .15s;
}
.tr-btn.active{background:var(--grad-brand);color:#fff}
.tr-btn:hover{color:var(--text)}
.tr-btn.active:hover{color:#fff}
#combinedChart{max-height:340px;min-height:240px;width:100%}
.update-time{text-align:right;color:var(--muted);font-family:var(--font-mono);font-size:10px;margin-top:12px}

/* ============ SETTINGS PANEL ============ */
.settings-backdrop{display:none;position:fixed;inset:0;background:rgba(20,16,40,.45);z-index:1000;backdrop-filter:blur(4px)}
.settings-backdrop.open{display:block}
.settings-panel{
  position:fixed;top:0;right:0;height:100%;width:min(330px,92vw);
  background:var(--card);backdrop-filter:blur(12px);z-index:1001;
  padding:26px 22px;transform:translateX(105%);transition:transform .28s ease;overflow-y:auto;
  box-shadow:-8px 0 32px rgba(20,16,40,.25);border-radius:20px 0 0 20px;
}
.settings-panel.open{transform:translateX(0)}
.settings-panel h2{font-size:17px;font-weight:700;margin-bottom:4px}
.settings-close{position:absolute;top:16px;right:16px;background:none;border:none;color:var(--muted);font-size:16px;cursor:pointer;padding:6px}
.settings-close:hover{color:var(--text)}
.settings-desc{font-size:12px;color:var(--muted);margin-bottom:18px}
.settings-item{
  display:flex;justify-content:space-between;align-items:center;gap:12px;
  padding:13px 0;border-bottom:1px solid var(--line-soft);
}
.settings-item-label{font-size:13.5px;font-weight:700;color:var(--text)}
.settings-item-sub{font-size:11px;color:var(--muted);margin-top:2px}
.sw{position:relative;display:inline-block;width:40px;height:22px;flex-shrink:0}
.sw input{opacity:0;width:0;height:0}
.sw-track{position:absolute;cursor:pointer;inset:0;background:var(--chip);border-radius:22px;transition:.25s;box-shadow:inset 0 1px 3px rgba(0,0,0,.08)}
.sw-track:before{position:absolute;content:'';height:16px;width:16px;left:3px;top:3px;background:#fff;border-radius:50%;transition:.25s;box-shadow:0 1px 3px rgba(0,0,0,.25)}
.sw input:checked+.sw-track{background:var(--ok)}
.sw input:checked+.sw-track:before{transform:translateX(18px)}
.pbtns{display:flex;flex-wrap:wrap;gap:6px}

/* ============ PARTICLES ============ */
#particleCanvas{position:fixed;inset:0;pointer-events:none;z-index:0;opacity:0;transition:opacity .6s ease}
#particleCanvas.visible{opacity:1}

@media(prefers-reduced-motion:reduce){
  *,*::before,*::after{animation-duration:.01ms!important;animation-iteration-count:1!important;transition-duration:.01ms!important}
}
@media(max-width:700px){
  body{padding:14px 10px 40px}
  .header{padding:20px 16px 16px}
  .card,.chart-card{padding:16px}
  .chart-row.double{grid-template-columns:1fr}
  .buttons{grid-template-columns:1fr 1fr}
  .fab{right:12px;width:42px;height:42px;font-size:17px}
  .theme-toggle{top:12px}
  .settings-btn{top:62px}
}
</style>
</head>
<body>
<canvas id="particleCanvas"></canvas>
<button class="fab theme-toggle" id="themeBtn" onclick="toggleTheme()" title="Theme">&#x1F319;</button>
<button class="fab settings-btn" onclick="openSettings()" title="Settings">
<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 1 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 1 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 1 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 1 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
</button>

<div class="settings-backdrop" id="settingsBackdrop" onclick="closeSettings()"></div>
<div class="settings-panel" id="settingsPanel">
<button class="settings-close" onclick="closeSettings()">&#x2715;</button>
<h2>Display</h2>
<div class="settings-desc">Show or hide dashboard sections</div>
<div class="settings-item">
<div><div class="settings-item-label">Sensor channels</div><div class="settings-item-sub">Temperature, humidity, derived values</div></div>
<label class="sw"><input type="checkbox" id="sw-sensors" checked onchange="toggleBlock('sensorRows',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">System &amp; control</div><div class="settings-item-sub">RAM, CPU, battery, WiFi, actions</div></div>
<label class="sw"><input type="checkbox" id="sw-system" checked onchange="toggleBlock('systemRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">Serial monitor</div><div class="settings-item-sub">Live WebSocket log console</div></div>
<label class="sw"><input type="checkbox" id="sw-serial" checked onchange="toggleBlock('serialRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item">
<div><div class="settings-item-label">History chart</div><div class="settings-item-sub">Time-series graph</div></div>
<label class="sw"><input type="checkbox" id="sw-chart" checked onchange="toggleBlock('chartRow',this.checked)"><span class="sw-track"></span></label>
</div>
<div class="settings-item" style="flex-direction:column;align-items:flex-start;gap:10px;border-bottom:none">
<div>
<div class="settings-item-label">Weather particles</div>
<div class="settings-item-sub">Ambient rain / snow. Auto mode follows sensor data.</div>
</div>
<div class="pbtns">
<button class="log-filter-btn active" id="pbOff"     onclick="forceParticle('none',this)">OFF</button>
<button class="log-filter-btn"        id="pbDrizzle" onclick="forceParticle('drizzle',this)">DRIZZLE</button>
<button class="log-filter-btn"        id="pbRain"    onclick="forceParticle('rain',this)">RAIN</button>
<button class="log-filter-btn"        id="pbHeavy"   onclick="forceParticle('heavy',this)">HEAVY</button>
<button class="log-filter-btn"        id="pbSnow"    onclick="forceParticle('snow',this)">SNOW</button>
</div>
</div>
</div>

<div class="container">
<div class="header">
<div class="eyebrow"><span class="live-dot"></span>ESP32-C3 &middot; AHT10 &middot; ENV STATION</div>
<h1>Environment</h1>
<div class="status-container">
<div id="statusBadge" class="status online"><div class="status-dot"></div><span>Connected</span></div>
<div id="lastUpdateBadge" class="status"><span id="lastUpdate">Loading...</span></div>
</div>
<div class="countdown-wrap">
<div class="countdown-bar-track"><div class="countdown-bar-fill" id="cdFill" style="width:100%"></div></div>
<span class="countdown-text" id="cdText">--</span>
</div>
</div>

<div id="sensorRows">
<div class="chart-row double">
<div class="card sensor-card temp-card" style="--acc:var(--acc-temp)">
<div class="sensor-header">
<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
<div class="sensor-label">Temperature</div>
</div>
<div class="sensor-value"><span id="temperature">--</span><span class="sensor-unit" id="tempUnit">&#xB0;C</span></div>
<div class="minmax">
<div><div class="mm-label">Min</div><div class="minmax-value"><span id="minTemp">--</span><span id="minTempUnit">&#xB0;C</span></div></div>
<div><div class="mm-label">Avg</div><div class="avg-value"><span id="avgTemp">--</span><span id="avgTempUnit">&#xB0;C</span></div></div>
<div><div class="mm-label">Max</div><div class="minmax-value"><span id="maxTemp">--</span><span id="maxTempUnit">&#xB0;C</span></div></div>
</div>
<div class="temp-unit-toggle">
<span class="toggle-label">&#xB0;C</span>
<label class="toggle-switch"><input type="checkbox" id="unitToggle" onchange="toggleUnit()"><span class="slider"></span></label>
<span class="toggle-label">&#xB0;F</span>
</div>
<div id="tempComfort" class="comfort-indicator"></div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkTemp"></canvas></div>
</div>
<div class="card sensor-card humidity-card" style="--acc:var(--acc-humid)">
<div class="sensor-header">
<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
<div class="sensor-label">Humidity</div>
</div>
<div class="sensor-value"><span id="humidity">--</span><span class="sensor-unit">%</span></div>
<div class="minmax">
<div><div class="mm-label">Min</div><div class="minmax-value"><span id="minHumid">--</span>%</div></div>
<div><div class="mm-label">Avg</div><div class="avg-value"><span id="avgHumid">--</span>%</div></div>
<div><div class="mm-label">Max</div><div class="minmax-value"><span id="maxHumid">--</span>%</div></div>
</div>
<div id="humidComfort" class="comfort-indicator"></div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkHumid"></canvas></div>
</div>
</div>
<div class="chart-row double">
<div class="card sensor-card dewpoint-card" style="--acc:var(--acc-dew)">
<div class="sensor-header">
<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/><line x1="8" y1="16" x2="16" y2="16"/></svg>
<div class="sensor-label">Dew point</div>
</div>
<div class="sensor-value"><span id="dewPoint">--</span><span class="sensor-unit" id="dewPointUnit">&#xB0;C</span></div>
<div class="sensor-description">Temperature at which water vapor condenses</div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkDew"></canvas></div>
</div>
<div class="card sensor-card heatindex-card" style="--acc:var(--acc-heat)">
<div class="sensor-header">
<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41"/></svg>
<div class="sensor-label">Heat index</div>
</div>
<div class="sensor-value"><span id="heatIndex">--</span><span class="sensor-unit" id="heatIndexUnit">&#xB0;C</span></div>
<div class="sensor-description">Perceived temperature adjusted for humidity</div>
<div class="sparkline-wrap"><div class="sparkline-label">Trend</div><canvas class="sparkline" id="spkHeat"></canvas></div>
</div>
</div>
</div>

<div id="systemRow" class="chart-row">
<div class="card">
<h3>System &amp; control</h3>
<div class="info-grid">
<div class="info-item"><div class="info-label">Uptime</div><div class="info-value" id="uptime">--</div></div>
<div class="info-item">
<div class="info-label">RAM</div>
<div class="info-value" id="freeHeap">--</div>
<div class="ram-bar-track"><div class="ram-bar-fill ram-ok" id="ramBarFill" style="width:0%"></div></div>
<div class="ram-stats"><span id="ramUsedPct">-- used</span><span id="ramFreePct">-- free</span></div>
</div>
<div class="info-item"><div class="info-label">CPU load</div><div class="info-value" id="cpuUsage">--</div></div>
<div class="info-item"><div class="info-label">Chip temp</div><div class="info-value" id="chipTemp" style="transition:color .5s">--</div></div>
<div class="info-item" style="grid-column:1/-1">
<div class="info-label">Battery</div>
<div class="battery-widget">
<div class="battery-icon"><div class="battery-fill" id="batteryFill" style="width:0%"></div></div>
<div><div class="battery-text" id="batteryPercent">--</div><div class="battery-sub" id="batterySource">--</div></div>
<div style="margin-left:auto;text-align:right"><div class="info-value" id="batteryVoltage">--</div><div class="battery-sub">voltage</div></div>
</div>
</div>
<div class="info-item"><div class="info-label">SSID</div><div class="info-value" id="ssid" style="font-size:12px">--</div></div>
<div class="info-item">
<div class="info-label">WiFi signal</div>
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
<div class="info-item"><div class="info-label">IP address</div><div class="info-value" id="ipAddr" style="font-size:12px">--</div></div>
</div>
<div class="buttons">
<button class="btn" onclick="exportCSV()">Export CSV</button>
<button class="btn" onclick="exportJSON()">Export JSON</button>
<button class="btn" onclick="resetMinMax()">Reset min/max</button>
<button class="btn btn-danger" onclick="rebootDevice()">Reboot</button>
</div>
</div>
</div>

<div id="serialRow" class="chart-row">
<div class="card">
<h3>Serial monitor <span class="ws-status" id="wsStatus"></span><span id="wsStatusText" style="font-size:11px;font-weight:500;color:var(--muted)">Connecting...</span></h3>
<div class="log-filters">
<button class="log-filter-btn active" style="--c:#FF9088" onclick="toggleLogFilter('error',this)">ERROR</button>
<button class="log-filter-btn active" onclick="toggleLogFilter('warning',this)">WARN</button>
<button class="log-filter-btn active" onclick="toggleLogFilter('info',this)">INFO</button>
<button class="log-filter-btn active" onclick="toggleLogFilter('success',this)">OK</button>
<span class="log-counter" id="logCounter">0 lines</span>
</div>
<div class="log-console" id="logConsole"></div>
<div class="buttons" style="margin-top:14px">
<button class="btn" onclick="clearLogs()">Clear</button>
<button class="btn" onclick="toggleAutoscroll()"><span id="autoscrollIcon">&#x25CF;</span> Auto-scroll</button>
</div>
</div>
</div>

<div id="chartRow" class="chart-row">
<div class="chart-card">
<div class="chart-head">
<h3>History</h3>
<div class="time-range">
<button class="tr-btn" onclick="setRange(5,this)">5m</button>
<button class="tr-btn" onclick="setRange(15,this)">15m</button>
<button class="tr-btn active" onclick="setRange(0,this)">All</button>
</div>
</div>
<div class="chart-toggles">
<button class="chart-toggle-btn toggle-temp active"  onclick="toggleSeries(0,this)"><span class="dot"></span>Temp</button>
<button class="chart-toggle-btn toggle-humid active" onclick="toggleSeries(1,this)"><span class="dot"></span>Humidity</button>
<button class="chart-toggle-btn toggle-heat active"  onclick="toggleSeries(2,this)"><span class="dot"></span>Heat</button>
<button class="chart-toggle-btn toggle-dew active"   onclick="toggleSeries(3,this)"><span class="dot"></span>Dew</button>
</div>
<canvas id="combinedChart"></canvas>
<div class="update-time">Updated <span id="updateTimeCombined">--</span></div>
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
var SENSOR_SEC=30; /* must match SENSOR_INTERVAL in config.h */
var sparkCharts={};
var BLOCK_KEY='envBlockPrefs';

/* ===== THEME (dark by default) ===== */
function isDark(){return document.body.classList.contains('dark');}
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
function chartColors(){
  return isDark()
    ?{grid:'rgba(232,234,246,.07)',tick:'#9fa8da'}
    :{grid:'rgba(45,51,72,.08)',tick:'#5b6379'};
}
function syncChartTheme(){
  if(!C)return;
  var cc=chartColors();
  C.options.scales.x.grid.color=cc.grid;
  C.options.scales.y.grid.color=cc.grid;
  C.options.scales.x.ticks.color=cc.tick;
  C.options.scales.y.ticks.color=cc.tick;
  C.options.scales.y.title.color=cc.tick;
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
  el.style.marginBottom=visible?'':'0';
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
    var tm=m.match(/(?:temp[\s_]?chip|chip[\s_]?temp|cpu[\s_]?temp|internal[\s_]?temp|core[\s_]?temp|t_chip|chip_t)[^\d\-]*(-?[\d.]+)/i);
    if(!tm)tm=m.match(/^CHIP[:\s]+(-?[\d.]+)/i);
    if(!tm)tm=m.match(/\bChip:\s*(-?[\d.]+)\s*[CF]?\b/i);
    if(tm){var v=parseFloat(tm[1]);if(!isNaN(v))updateChipTemp(v);}
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
  document.getElementById('autoscrollIcon').textContent=autoscroll?'\u25CF':'\u25CB';
}

/* ===== CHARTS ===== */
var DS_CFG=[
  {label:'Temperature',border:'#f5576c',bg:'rgba(245,87,108,.12)'},
  {label:'Humidity',   border:'#4facfe',bg:'rgba(79,172,254,.12)'},
  {label:'Heat index', border:'#f0a028',bg:'rgba(240,160,40,.12)'},
  {label:'Dew point',  border:'#22b8c8',bg:'rgba(34,184,200,.12)'}
];
function initCharts(){
  var ctx=document.getElementById('combinedChart').getContext('2d');
  var cc=chartColors();
  C=new Chart(ctx,{
    type:'line',
    data:{labels:[],datasets:DS_CFG.map(function(c){return{
      label:c.label,data:[],borderColor:c.border,backgroundColor:c.bg,
      tension:.35,fill:true,borderWidth:2,pointRadius:0,pointHoverRadius:5,
      pointBackgroundColor:c.border
    };})},
    options:{
      responsive:true,maintainAspectRatio:false,
      interaction:{mode:'index',intersect:false},
      plugins:{
        legend:{display:false},
        tooltip:{
          backgroundColor:'rgba(30,26,46,.94)',padding:12,cornerRadius:8,
          titleFont:{size:12,family:'ui-monospace,Consolas,monospace'},
          bodyFont:{size:12,family:'ui-monospace,Consolas,monospace'},
          borderColor:'rgba(255,255,255,.14)',borderWidth:1,
          callbacks:{
            label:function(c){var u=['\u00B0C','%','\u00B0C','\u00B0C'];if(F){u=['\u00B0F','%','\u00B0F','\u00B0F'];}return' '+c.dataset.label+': '+c.parsed.y.toFixed(1)+' '+u[c.datasetIndex];}
          }
        }
      },
      scales:{
        x:{grid:{color:cc.grid,drawBorder:false},ticks:{font:{size:10,family:'ui-monospace,Consolas,monospace'},maxRotation:0,autoSkip:true,maxTicksLimit:8,color:cc.tick}},
        y:{position:'right',grid:{drawBorder:false,color:cc.grid},ticks:{font:{size:11,family:'ui-monospace,Consolas,monospace'},color:cc.tick},title:{display:true,text:'\u00B0C / %',color:cc.tick,font:{size:10}}}
      },
      animation:{duration:500,easing:'easeOutQuart'}
    }
  });
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
  var pts=Math.round(rangeMinutes*60/SENSOR_SEC);
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
var SPK_DEFAULT={spkTemp:'#f5576c',spkHumid:'#4facfe',spkDew:'#22b8c8',spkHeat:'#f0a028'};
function initSparklines(){
  Object.keys(SPK_DEFAULT).forEach(function(id){
    var el=document.getElementById(id);
    if(!el)return;
    var c=SPK_DEFAULT[id];
    sparkCharts[id]=new Chart(el.getContext('2d'),{
      type:'line',
      data:{labels:[],datasets:[{data:[],borderColor:c,backgroundColor:hexToRgba(c,.12),borderWidth:1.5,pointRadius:0,tension:.35,fill:true}]},
      options:{responsive:true,maintainAspectRatio:false,plugins:{legend:{display:false},tooltip:{enabled:false}},scales:{x:{display:false},y:{display:false}},animation:{duration:300},events:[]}
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

/* ===== CHANNEL ACCENTS (colour follows the value) ===== */
function lerpHex(a,b,t){
  var h=function(s){return parseInt(s,16);};
  var r=function(v){return Math.round(v).toString(16).padStart(2,'0');};
  var ar=h(a.slice(1,3)),ag=h(a.slice(3,5)),ab=h(a.slice(5,7));
  var br=h(b.slice(1,3)),bg=h(b.slice(3,5)),bb=h(b.slice(5,7));
  return '#'+r(ar+(br-ar)*t)+r(ag+(bg-ag)*t)+r(ab+(bb-ab)*t);
}
function hexToRgba(hex,a){
  var h=function(s){return parseInt(s,16);};
  return 'rgba('+h(hex.slice(1,3))+','+h(hex.slice(3,5))+','+h(hex.slice(5,7))+','+a+')';
}
function valueToColor(val,stops){
  var n=stops.length;
  if(val<=stops[0].at)return stops[0].c;
  if(val>=stops[n-1].at)return stops[n-1].c;
  for(var i=0;i<n-1;i++){
    if(val>=stops[i].at&&val<stops[i+1].at){
      var t=(val-stops[i].at)/(stops[i+1].at-stops[i].at);
      return lerpHex(stops[i].c,stops[i+1].c,t);
    }
  }
  return stops[n-1].c;
}
var TEMP_STOPS=[{at:0,c:'#4facfe'},{at:12,c:'#22b8c8'},{at:19,c:'#2eb872'},{at:25,c:'#f0a028'},{at:31,c:'#f5576c'},{at:40,c:'#e5484d'}];
var HUMID_STOPS=[{at:0,c:'#f0a028'},{at:30,c:'#2eb872'},{at:55,c:'#4facfe'},{at:80,c:'#8f7ee7'},{at:100,c:'#b45de0'}];
var HEAT_STOPS=[{at:0,c:'#4facfe'},{at:24,c:'#2eb872'},{at:28,c:'#f0a028'},{at:35,c:'#f5576c'},{at:45,c:'#e5484d'}];
var DEW_STOPS=[{at:-5,c:'#22b8c8'},{at:10,c:'#2eb872'},{at:16,c:'#f0a028'},{at:21,c:'#f5576c'},{at:26,c:'#e5484d'}];
function updateCardColors(tc,h,hc,dc){
  var map=[
    ['.temp-card',     valueToColor(tc,TEMP_STOPS), 'spkTemp'],
    ['.humidity-card', valueToColor(h, HUMID_STOPS),'spkHumid'],
    ['.heatindex-card',valueToColor(hc,HEAT_STOPS), 'spkHeat'],
    ['.dewpoint-card', valueToColor(dc,DEW_STOPS),  'spkDew']
  ];
  map.forEach(function(m){
    var el=document.querySelector(m[0]);
    if(el)el.style.setProperty('--acc',m[1]);
    var sc=sparkCharts[m[2]];
    if(sc){
      sc.data.datasets[0].borderColor=m[1];
      sc.data.datasets[0].backgroundColor=hexToRgba(m[1],.12);
      sc.update('none');
    }
  });
}

/* ===== WIFI BARS ===== */
function updateWifiBars(rssi){
  var active,color,label;
  if(rssi>=-50){active=4;color='#2eb872';label='Excellent';}
  else if(rssi>=-60){active=3;color='#2eb872';label='Good';}
  else if(rssi>=-70){active=2;color='#f0a028';label='Fair';}
  else if(rssi>=-80){active=1;color='#e6a700';label='Weak';}
  else{active=1;color='#e5484d';label='Very weak';}
  ['wb1','wb2','wb3','wb4'].forEach(function(id,i){
    document.getElementById(id).style.backgroundColor=i<active?color:'';
  });
  document.getElementById('rssiLabel').textContent=label;
}

/* ===== SYSTEM DISPLAYS ===== */
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
  el.textContent=(F?(val*9/5+32):val).toFixed(1)+(F?'\u00B0F':'\u00B0C');
  el.style.color=val>=80?'#e5484d':val>=60?'#f5576c':val>=45?'#f0a028':'';
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
    if(v>=20&&v<=24)return{l:'excellent',t:'Optimal'};
    if(v>=18&&v<=26)return{l:'good',t:'Comfortable'};
    return{l:'poor',t:'Uncomfortable'};
  }else{
    if(v>=40&&v<=60)return{l:'excellent',t:'Optimal'};
    if(v>=30&&v<=70)return{l:'good',t:'Normal'};
    return{l:'poor',t:'Uncomfortable'};
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
    document.getElementById('lastUpdate').textContent=new Date().toLocaleTimeString('ru-RU');
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
    var usedPct=d.heapUsagePct||0;
    document.getElementById('freeHeap').textContent=d.freeHeap;
    document.getElementById('ramUsedPct').textContent=usedPct.toFixed(1)+'% used';
    document.getElementById('ramFreePct').textContent=(100-usedPct).toFixed(1)+'% free';
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
      var isExternalPower=b.isCharging||b.isUsb||(b.voltage>4.25);
      if(isExternalPower){
        fill.style.width='100%';
        fill.className='battery-fill charging';
        document.getElementById('batteryPercent').textContent='USB power';
        var voltTxt=b.voltage+'V';
        if(b.voltage>4.25)voltTxt=b.voltage+'V (ext)';
        document.getElementById('batteryVoltage').textContent=voltTxt;
        document.getElementById('batterySource').textContent=b.isCharging?'Charging':(b.isUsb?'Fully charged':'Direct power');
      } else {
        fill.style.width=pct+'%';
        fill.className='battery-fill '+(b.isCritical?'critical':b.isLow?'low':pct<60?'mid':'good');
        var pTxt=pct+'%';
        if(b.isCritical)pTxt=pct+'% \u00B7 CRITICAL';
        else if(b.isLow)pTxt=pct+'% \u00B7 LOW';
        document.getElementById('batteryPercent').textContent=pTxt;
        document.getElementById('batteryVoltage').textContent=b.voltage+'V';
        document.getElementById('batterySource').textContent=(b.status==='Fully charged')?'Full':b.source;
      }
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
function resetMinMax(){if(confirm('Reset min/max values?')){fetch('/reset').then(function(){updateData();});}}
function rebootDevice(){if(confirm('Reboot the device?')){fetch('/reboot');}}
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
  var count={rain:130,heavy:200,drizzle:55,snow:80}[mode]||70;
  for(var i=0;i<count;i++)particles.push(makeParticle(mode,true));
  if(!pRAF)animateParticles();
}
function makeParticle(mode,randomY){
  var W=pCanvas.width,H=pCanvas.height;
  if(mode==='rain'||mode==='heavy'||mode==='drizzle'){
    var heavy=(mode==='heavy');
    var vy=heavy?14+Math.random()*8:mode==='rain'?9+Math.random()*5:3+Math.random()*2.5;
    return{x:Math.random()*(W+100)-50,y:randomY?Math.random()*H*1.2:-Math.random()*50,vy:vy,vx:vy*0.18,len:vy*(mode==='drizzle'?1.2:1.8),w:heavy?1.6:mode==='rain'?1.2:1,alpha:heavy?0.4+Math.random()*0.2:mode==='rain'?0.25+Math.random()*0.2:0.15+Math.random()*0.15,hue:210};
  }else{
    return{x:Math.random()*W,y:randomY?Math.random()*H:-10,vy:0.4+Math.random()*1.0,vx:(Math.random()-0.5)*0.3,r:1.2+Math.random()*2.8,alpha:0.4+Math.random()*0.3,drift:Math.random()*Math.PI*2,driftSpeed:0.008+Math.random()*0.015,sparkle:Math.random()*Math.PI*2};
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
      pCtx.beginPath();pCtx.arc(p.x,p.y,p.r,0,Math.PI*2);pCtx.fillStyle='rgba(225,240,255,'+(p.alpha*sp)+')';pCtx.fill();
      if(p.y>H+12||p.x<-12||p.x>W+12)particles[i]=makeParticle('snow',false);
    }else{
      p.x+=p.vx;p.y+=p.vy;
      var tx=p.x-p.vx*(p.len/p.vy),ty=p.y-p.len;
      var grad=pCtx.createLinearGradient(tx,ty,p.x,p.y);
      grad.addColorStop(0,'hsla('+p.hue+',65%,88%,0)');
      grad.addColorStop(1,'hsla('+p.hue+',65%,90%,'+p.alpha+')');
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
  text.textContent=remaining>50?'next in '+secs+'s':'updating...';
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