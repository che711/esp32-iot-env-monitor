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
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:20px}
.container{max-width:700px;margin:0 auto}
.header{background:rgba(255,255,255,.95);backdrop-filter:blur(10px);border-radius:20px;padding:30px;margin-bottom:20px;box-shadow:0 10px 30px rgba(0,0,0,.2);text-align:center}
.header h1{color:#333;font-size:clamp(24px,5vw,36px);margin-bottom:10px;font-weight:700}
.subtitle{color:#666;font-size:14px;margin-bottom:15px}
.status-container{display:flex;gap:10px;justify-content:center;flex-wrap:wrap}
.status{display:inline-flex;align-items:center;gap:8px;padding:8px 20px;border-radius:25px;font-size:13px;font-weight:600}
.status.online{background:#d4edda;color:#155724}
.status.offline{background:#f8d7da;color:#721c24}
.status-dot{width:8px;height:8px;border-radius:50%;animation:pulse 2s infinite}
.status.online .status-dot{background:#28a745}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
.card{background:rgba(255,255,255,.95);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:0 4px 16px rgba(0,0,0,.15);transition:.3s}
.card:hover{transform:translateY(-3px);box-shadow:0 10px 30px rgba(0,0,0,.2)}
.sensor-card{color:#fff;position:relative;overflow:hidden}
.sensor-card::before{content:'';position:absolute;top:-50%;right:-50%;width:200%;height:200%;background:radial-gradient(circle,rgba(255,255,255,.1) 0%,transparent 70%);pointer-events:none}
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
.info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px;margin-top:15px}
.info-item{background:#f8f9fa;padding:15px;border-radius:8px;border-left:4px solid #667eea}
.info-label{color:#666;font-size:11px;text-transform:uppercase;margin-bottom:8px}
.info-value{color:#333;font-weight:700;font-size:clamp(14px,3vw,18px)}
.chart-row{display:grid;grid-template-columns:1fr;gap:20px;margin-bottom:20px}
.chart-row.double{grid-template-columns:1fr 1fr}
.chart-card{background:rgba(255,255,255,.95);backdrop-filter:blur(10px);border-radius:12px;padding:25px;box-shadow:0 4px 16px rgba(0,0,0,.15);transition:.3s}
.chart-card:hover{transform:translateY(-3px);box-shadow:0 10px 30px rgba(0,0,0,.2)}
canvas{max-height:350px;min-height:250px;width:100%}
.buttons{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:12px;margin-top:15px}
.btn{padding:12px 20px;border:none;border-radius:8px;font-size:14px;cursor:pointer;font-weight:600;display:flex;align-items:center;justify-content:center;gap:8px;transition:.3s}
.btn-primary{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff}
.btn-success{background:linear-gradient(135deg,#28a745 0%,#218838 100%);color:#fff}
.btn-danger{background:linear-gradient(135deg,#dc3545 0%,#c82333 100%);color:#fff}
.btn:hover{transform:translateY(-2px)}
.temp-unit-toggle{margin-top:15px;text-align:center;padding:15px;background:rgba(255,255,255,.1);border-radius:8px}
.toggle-label{color:#fff;font-weight:600;font-size:13px}
.toggle-switch{position:relative;display:inline-block;width:60px;height:30px;margin:0 12px}
.toggle-switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:rgba(255,255,255,.3);transition:.4s;border-radius:30px}
.slider:before{position:absolute;content:"";height:24px;width:24px;left:3px;bottom:3px;background:#fff;transition:.4s;border-radius:50%}
input:checked+.slider{background:rgba(255,255,255,.4)}
input:checked+.slider:before{transform:translateX(30px)}
.update-time{text-align:center;color:#999;font-size:12px;margin-top:15px;padding:10px;background:#f8f9fa;border-radius:8px}
.comfort-indicator{margin-top:10px;padding:8px 12px;border-radius:8px;font-size:11px;font-weight:600;text-align:center;background:rgba(255,255,255,.2)}
.comfort-excellent{background:rgba(40,167,69,.3)}
.comfort-good{background:rgba(255,193,7,.3)}
.comfort-fair{background:rgba(255,152,0,.3)}
.comfort-poor{background:rgba(220,53,69,.3)}
.log-console{background:#1e1e1e;color:#d4d4d4;font-family:'Consolas','Monaco',monospace;font-size:12px;padding:15px;border-radius:8px;height:300px;overflow-y:auto;margin-top:15px}
.log-console::-webkit-scrollbar{width:8px}
.log-console::-webkit-scrollbar-track{background:#2d2d2d;border-radius:4px}
.log-console::-webkit-scrollbar-thumb{background:#555;border-radius:4px}
.log-console::-webkit-scrollbar-thumb:hover{background:#777}
.log-line{margin:2px 0;white-space:pre-wrap;word-break:break-all}
.log-error{color:#f48771}
.log-warning{color:#dcdcaa}
.log-info{color:#4ec9b0}
.log-success{color:#4fc1ff}
.ws-status{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:8px}
.ws-connected{background:#28a745}
.ws-disconnected{background:#dc3545}
@media(max-width:768px){
body{padding:10px}
.header{padding:20px}
.card{padding:20px}
.info-grid{grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px}
.buttons{grid-template-columns:1fr}
.chart-row.double{grid-template-columns:1fr}
}
</style>
</head>
<body>
<div class="container">
<div class="header">
<h1>🌡️ Environmental Statistics</h1>
<div class="subtitle">Temperature and humidity monitoring</div>
<div class="status-container">
<div id="statusBadge" class="status online"><div class="status-dot"></div><span>Connected</span></div>
<div class="status" style="background:#e3f2fd;color:#1976d2"><span id="lastUpdate">Загрузка...</span></div>
</div>
</div>

<div class="chart-row double">
<div class="card sensor-card temp-card">
<div class="sensor-header"><div class="sensor-label">🌡️ Temperature</div></div>
<div class="sensor-value"><span id="temperature">--</span><span class="sensor-unit" id="tempUnit">°C</span></div>
<div class="minmax">
<div><div style="font-size:10px">⬇️ Мин</div><div class="minmax-value"><span id="minTemp">--</span><span id="minTempUnit">°C</span></div></div>
<div class="avg-value">📊 <span id="avgTemp">--</span><span id="avgTempUnit">°C</span></div>
<div><div style="font-size:10px">⬆️ Макс</div><div class="minmax-value"><span id="maxTemp">--</span><span id="maxTempUnit">°C</span></div></div>
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
<div><div style="font-size:10px">⬇️ Мин</div><div class="minmax-value"><span id="minHumid">--</span>%</div></div>
<div class="avg-value">📊 <span id="avgHumid">--</span>%</div>
<div><div style="font-size:10px">⬆️ Макс</div><div class="minmax-value"><span id="maxHumid">--</span>%</div></div>
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

<div class="chart-row">
<div class="card">
<h3 style="margin-bottom:15px;color:#333">💻 System & Control</h3>
<div class="info-grid">
<div class="info-item"><div class="info-label">⏱️ Time</div><div class="info-value" id="uptime">--</div></div>
<div class="info-item"><div class="info-label">🧠 RAM</div><div class="info-value" id="freeHeap">--</div></div>
<div class="info-item"><div class="info-label">📊 CPU</div><div class="info-value" id="cpuUsage">--</div></div>
<div class="info-item"><div class="info-label">🔋 Battery</div><div class="info-value" id="batteryPercent">--</div></div>
<div class="info-item"><div class="info-label">⚡ Voltage</div><div class="info-value" id="batteryVoltage">--</div></div>
<div class="info-item"><div class="info-label">🔌 Power</div><div class="info-value" id="batterySource" style="font-size:12px">--</div></div>
<div class="info-item"><div class="info-label">📶 SSID</div><div class="info-value" id="ssid" style="font-size:13px">--</div></div>
<div class="info-item"><div class="info-label"><span id="wifiSignal">📶</span> RSSI</div><div class="info-value" id="rssi">--</div></div>
<div class="info-item"><div class="info-label">🌐 IP</div><div class="info-value" id="ipAddr" style="font-size:11px">--</div></div>
</div>
<div class="buttons">
<button class="btn btn-primary" onclick="exportCSV()">📥 CSV</button>
<button class="btn btn-success" onclick="exportJSON()">📋 JSON</button>
<button class="btn btn-success" onclick="resetMinMax()">🔄 Reset</button>
<button class="btn btn-danger" onclick="rebootDevice()">⚡ Reboot</button>
</div>
</div>
</div>

<div class="chart-row">
<div class="card">
<h3 style="margin-bottom:15px;color:#333">📟 Serial Monitor <span class="ws-status" id="wsStatus"></span><span id="wsStatusText" style="font-size:12px;color:#666">Подключение...</span></h3>
<div class="log-console" id="logConsole"></div>
<div class="buttons" style="margin-top:15px">
<button class="btn btn-primary" onclick="clearLogs()">🗑️ Clear</button>
<button class="btn btn-success" onclick="toggleAutoscroll()"><span id="autoscrollIcon">📌</span> Auto-scroll</button>
</div>
</div>
</div>

<div class="chart-row">
<div class="chart-card">
<h3 style="margin-bottom:15px;color:#333">🌡️ Temperature</h3>
<canvas id="tempChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeTemp">--</span></div>
</div>
</div>

<div class="chart-row">
<div class="chart-card">
<h3 style="margin-bottom:15px;color:#333">💧 Humidity</h3>
<canvas id="humidChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeHumid">--</span></div>
</div>
</div>

<div class="chart-row">
<div class="chart-card">
<h3 style="margin-bottom:15px;color:#333">🌡️ Heat Index</h3>
<canvas id="heatChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeHeat">--</span></div>
</div>
</div>

<div class="chart-row">
<div class="chart-card">
<h3 style="margin-bottom:15px;color:#333">💧 Dew point</h3>
<canvas id="dewChart"></canvas>
<div class="update-time">Updated: <span id="updateTimeDew">--</span></div>
</div>
</div>

</div>

<script>
let F=false,D={labels:[],temp:[],humid:[],heat:[],dew:[]},T,H,E,W,errCnt=0,iU,iS,iH;
let ws,autoscroll=true,maxLogs=500;
const O={responsive:!0,maintainAspectRatio:!1,interaction:{mode:'index',intersect:!1},plugins:{legend:{display:!1},tooltip:{backgroundColor:'rgba(0,0,0,.8)',padding:15,titleFont:{size:14,weight:'bold'},bodyFont:{size:14},borderWidth:2,callbacks:{title:c=>'Время: '+c[0].label,label:c=>c.dataset.label+': '+c.parsed.y.toFixed(1)}}},scales:{x:{grid:{color:'rgba(0,0,0,.05)',drawBorder:!1},ticks:{font:{size:11},maxRotation:0,autoSkip:!0,maxTicksLimit:10}},y:{grid:{drawBorder:!1},ticks:{font:{size:12}}}},animation:{duration:750,easing:'easeInOutQuart',delay:0}};

function initWebSocket(){
const proto=window.location.protocol==='https:'?'wss:':'ws:';
const wsUrl=`${proto}//${window.location.hostname}:81/`;
ws=new WebSocket(wsUrl);

ws.onopen=()=>{
document.getElementById('wsStatus').className='ws-status ws-connected';
document.getElementById('wsStatusText').textContent='Connected';
addLog('WebSocket connected','info');
};

ws.onclose=()=>{
document.getElementById('wsStatus').className='ws-status ws-disconnected';
document.getElementById('wsStatusText').textContent='Отключено';
addLog('WebSocket disconnected, reconnecting...','warning');
setTimeout(initWebSocket,3000);
};

ws.onerror=(e)=>{
addLog('WebSocket error','error');
};

ws.onmessage=(e)=>{
addLog(e.data,'info');
};
}

function addLog(msg,type='info'){
const console=document.getElementById('logConsole');
const line=document.createElement('div');
line.className=`log-line log-${type}`;
const time=new Date().toLocaleTimeString('ru-RU');
line.textContent=`[${time}] ${msg}`;
console.appendChild(line);

while(console.children.length>maxLogs){
console.removeChild(console.firstChild);
}

if(autoscroll){
console.scrollTop=console.scrollHeight;
}
}

function clearLogs(){
document.getElementById('logConsole').innerHTML='';
addLog('Logs cleared','success');
}

function toggleAutoscroll(){
autoscroll=!autoscroll;
document.getElementById('autoscrollIcon').textContent=autoscroll?'📌':'📍';
}

function initCharts(){
const tc=document.getElementById('tempChart').getContext('2d');
T=new Chart(tc,{type:'line',data:{labels:D.labels,datasets:[{label:'Temperature (°C)',data:D.temp,borderColor:'#667eea',backgroundColor:'rgba(102,126,234,.15)',tension:.4,fill:!0,borderWidth:5,pointRadius:4}]},options:{...O,scales:{...O.scales,y:{...O.scales.y,position:'right',title:{display:!0,text:'°C'}}}}});

const hc=document.getElementById('humidChart').getContext('2d');
H=new Chart(hc,{type:'line',data:{labels:D.labels,datasets:[{label:'Humidity (%)',data:D.humid,borderColor:'#4facfe',backgroundColor:'rgba(79,172,254,.15)',tension:.4,fill:!0,borderWidth:5,pointRadius:4}]},options:{...O,scales:{...O.scales,y:{...O.scales.y,position:'right',title:{display:!0,text:'%'}}}}});

const ec=document.getElementById('heatChart').getContext('2d');
E=new Chart(ec,{type:'line',data:{labels:D.labels,datasets:[{label:'Heat Index (°C)',data:D.heat,borderColor:'#fa709a',backgroundColor:'rgba(250,112,154,.15)',tension:.4,fill:!0,borderWidth:5,pointRadius:4}]},options:{...O,scales:{...O.scales,y:{...O.scales.y,position:'right',title:{display:!0,text:'°C'}}}}});

const dc=document.getElementById('dewChart').getContext('2d');
W=new Chart(dc,{type:'line',data:{labels:D.labels,datasets:[{label:'Dew point (°C)',data:D.dew,borderColor:'#f093fb',backgroundColor:'rgba(240,147,251,.15)',tension:.4,fill:!0,borderWidth:5,pointRadius:4}]},options:{...O,scales:{...O.scales,y:{...O.scales.y,position:'right',title:{display:!0,text:'°C'}}}}});
}

function c2f(c){return c*9/5+32}
function toggleUnit(){F=!F;updateDisplay()}

function updateDisplay(){
document.querySelectorAll('#tempUnit,#minTempUnit,#maxTempUnit,#avgTempUnit,#dewPointUnit,#heatIndexUnit').forEach(e=>e.textContent=F?'°F':'°C');
T.options.scales.y.title.text=F?'°F':'°C';
E.options.scales.y.title.text=F?'°F':'°C';
W.options.scales.y.title.text=F?'°F':'°C';
T.update();E.update();W.update();updateData();
}

function getComfort(v, isTemp) {
if (isTemp) {
if (v >= 20 && v <= 24) return { l: 'excellent', t: '✅ Optimal' };
if (v >= 18 && v <= 26) return { l: 'good', t: '👍 Comfortable' };
return { l: 'poor', t: '❌ Uncomfortable' };
} else {
// Улучшенная логика влажности
if (v >= 40 && v <= 60) return { l: 'excellent', t: '✅ Optimal' };
if (v >= 30 && v <= 70) return { l: 'good', t: '👍 Normal' };
return { l: 'poor', t: '❌ Uncomfortable' };
}
}

function updateData(){
fetch('/data').then(r=>r.json()).then(d=>{
const t=F?c2f(d.temperature):d.temperature,minT=F?c2f(d.minTemp):d.minTemp,maxT=F?c2f(d.maxTemp):d.maxTemp,avgT=F?c2f(d.avgTemp):d.avgTemp,dewP=F?c2f(d.dewPoint):d.dewPoint,heatI=F?c2f(d.heatIndex):d.heatIndex;
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
const tc=getComfort(d.temperature,!0),hc=getComfort(d.humidity,!1);
const te=document.getElementById('tempComfort');te.textContent=tc.t;te.className='comfort-indicator comfort-'+tc.l;
const he=document.getElementById('humidComfort');he.textContent=hc.t;he.className='comfort-indicator comfort-'+hc.l;
document.getElementById('lastUpdate').textContent='Updated: '+new Date().toLocaleTimeString('ru-RU');
errCnt=0;document.getElementById('statusBadge').className='status online';
document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Connected</span>';
}).catch(e=>{errCnt++;if(errCnt>2){document.getElementById('statusBadge').className='status offline';document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Disconnected</span>';}});
}

function updateStats(){
fetch('/stats').then(r=>r.json()).then(d=>{
document.getElementById('uptime').textContent=d.uptime;
document.getElementById('freeHeap').textContent=d.freeHeap;
document.getElementById('cpuUsage').textContent=d.cpuUsage+'%';
document.getElementById('ssid').textContent=d.ssid||'--';
const rssiVal=parseInt(d.rssi);let rssiIcon='📶';
if(rssiVal>=-60)rssiIcon='📶';else if(rssiVal>=-75)rssiIcon='📶';else rssiIcon='⚠️';
document.getElementById('wifiSignal').textContent=rssiIcon;
document.getElementById('rssi').textContent=d.rssi+' dBm';
document.getElementById('ipAddr').textContent=d.ip;

// ═══════════════════════════════════════════════════════
// Обновление данных батареи
// ═══════════════════════════════════════════════════════
if(d.battery){
const b=d.battery;
const pElem=document.getElementById('batteryPercent');
const vElem=document.getElementById('batteryVoltage');
const sElem=document.getElementById('batterySource');

// Процент с emoji
let pText=b.percent+'%';
if(b.isCritical)pText='‼️ '+pText;
else if(b.isLow)pText='⚠️ '+pText;
else if(b.percent>=80)pText='🟢 '+pText;
else if(b.percent>=40)pText='🟡 '+pText;
else pText='🟠 '+pText;
pElem.textContent=pText;

// Напряжение
vElem.textContent=b.voltage+'V';

// Источник питания со статусом
let sText=b.source;
if(b.isCharging)sText='⚡ '+sText+' (Charging)';
else if(b.status==='Fully charged')sText='✓ '+sText+' (Full)';
else sText='🔋 '+sText;
sElem.textContent=sText;

// Цвет индикатора
if(b.isCritical){
pElem.style.color='#dc3545';
vElem.style.color='#dc3545';
}else if(b.isLow){
pElem.style.color='#ff9800';
vElem.style.color='#ff9800';
}else if(b.isCharging){
pElem.style.color='#28a745';
vElem.style.color='#28a745';
}else{
pElem.style.color='#333';
vElem.style.color='#333';
}
}
}).catch(e=>console.error(e));
}

function updateHistory(){
fetch('/history').then(r=>r.json()).then(d=>{
const si=Math.max(0,d.labels.length-60);
D.labels=d.labels.slice(si);D.temp=d.temp.slice(si);D.humid=d.humid.slice(si);
// Use server-calculated values (Rothfusz formula) instead of client-side approximations
D.heat=d.heat?d.heat.slice(si):D.temp;
D.dew=d.dew?d.dew.slice(si):D.temp;
if(F){D.temp=D.temp.map(c2f);D.heat=D.heat.map(c2f);D.dew=D.dew.map(c2f);}
T.data.labels=D.labels;T.data.datasets[0].data=D.temp;T.update();
H.data.labels=D.labels;H.data.datasets[0].data=D.humid;H.update();
E.data.labels=D.labels;E.data.datasets[0].data=D.heat;E.update();
W.data.labels=D.labels;W.data.datasets[0].data=D.dew;W.update();
const ts=new Date().toLocaleTimeString();
['Temp','Humid','Heat','Dew'].forEach(id=>document.getElementById('updateTime'+id).textContent=ts);
}).catch(e=>console.error(e));
}

function resetMinMax(){if(confirm('Reset min/max?')){fetch('/reset').then(()=>updateData());}}
function rebootDevice(){if(confirm('Reboot?')){fetch('/reboot');}}

function exportCSV(){
let csv='Time,Temp,Humid\n';
for(let i=0;i<D.labels.length;i++)csv+=`${D.labels[i]},${D.temp[i]},${D.humid[i]}\n`;
const b=new Blob([csv],{type:'text/csv'});const a=document.createElement('a');a.href=URL.createObjectURL(b);a.download='weather.csv';a.click();
}

function exportJSON(){
const b=new Blob([JSON.stringify(D)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(b);a.download='weather.json';a.click();
}

document.addEventListener('DOMContentLoaded',()=>{
initCharts();initWebSocket();updateData();updateStats();updateHistory();
iU=setInterval(updateData,10000);iS=setInterval(updateStats,10000);iH=setInterval(updateHistory,15000);
});
</script>
</body>
</html>
)rawliteral";

#endif
