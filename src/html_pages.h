#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Метеостанция</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:20px}
.container{max-width:1400px;margin:0 auto}
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
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px;margin-bottom:20px}
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
.chart-card{grid-column:1/-1}
canvas{max-height:350px}
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
.chart-controls{display:flex;gap:10px;margin-bottom:20px;justify-content:center;flex-wrap:wrap}
.chart-btn{padding:10px 20px;border:2px solid #667eea;background:#fff;color:#667eea;border-radius:8px;font-size:13px;font-weight:600;cursor:pointer;transition:.3s}
.chart-btn:hover{background:#f0f0f0}
.chart-btn.active{background:#667eea;color:#fff;border-color:transparent}
.update-time{text-align:center;color:#999;font-size:12px;margin-top:15px;padding:10px;background:#f8f9fa;border-radius:8px}
.comfort-indicator{margin-top:10px;padding:8px 12px;border-radius:8px;font-size:11px;font-weight:600;text-align:center;background:rgba(255,255,255,.2)}
.comfort-excellent{background:rgba(40,167,69,.3)}
.comfort-good{background:rgba(255,193,7,.3)}
.comfort-fair{background:rgba(255,152,0,.3)}
.comfort-poor{background:rgba(220,53,69,.3)}
@media(max-width:768px){
body{padding:10px}
.header{padding:20px}
.grid{grid-template-columns:1fr;gap:15px}
.card{padding:20px}
.info-grid{grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px}
.buttons{grid-template-columns:1fr}
}
</style>
</head>
<body>
<div class="container">
<div class="header">
<h1>🌡️ ESP32 Метеостанция</h1>
<div class="subtitle">Мониторинг температуры и влажности</div>
<div class="status-container">
<div id="statusBadge" class="status online"><div class="status-dot"></div><span>Подключено</span></div>
<div class="status" style="background:#e3f2fd;color:#1976d2"><span id="lastUpdate">Загрузка...</span></div>
</div>
</div>
<div class="grid">
<div class="card sensor-card temp-card">
<div class="sensor-header"><div class="sensor-label">🌡️ Температура</div></div>
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
<div class="sensor-header"><div class="sensor-label">💧 Влажность</div></div>
<div class="sensor-value"><span id="humidity">--</span><span class="sensor-unit">%</span></div>
<div class="minmax">
<div><div style="font-size:10px">⬇️ Мин</div><div class="minmax-value"><span id="minHumid">--</span>%</div></div>
<div class="avg-value">📊 <span id="avgHumid">--</span>%</div>
<div><div style="font-size:10px">⬆️ Макс</div><div class="minmax-value"><span id="maxHumid">--</span>%</div></div>
</div>
<div id="humidComfort" class="comfort-indicator"></div>
</div>
<div class="card sensor-card dewpoint-card">
<div class="sensor-header"><div class="sensor-label">💧 Точка росы</div></div>
<div class="sensor-value"><span id="dewPoint">--</span><span class="sensor-unit" id="dewPointUnit">°C</span></div>
<div class="sensor-description">Температура конденсации водяного пара</div>
</div>
<div class="card sensor-card heatindex-card">
<div class="sensor-header"><div class="sensor-label">🌡️ Ощущаемая</div></div>
<div class="sensor-value"><span id="heatIndex">--</span><span class="sensor-unit" id="heatIndexUnit">°C</span></div>
<div class="sensor-description">Восприятие температуры с учётом влажности</div>
</div>
<div class="card">
<h3 style="margin-bottom:15px;color:#333">⚙️ Управление</h3>
<div class="buttons">
<button class="btn btn-primary" onclick="exportCSV()">📥 CSV</button>
<button class="btn btn-success" onclick="exportJSON()">📋 JSON</button>
<button class="btn btn-success" onclick="resetMinMax()">🔄 Сброс</button>
<button class="btn btn-danger" onclick="rebootDevice()">⚡ Перезагрузка</button>
</div>
</div>
<div class="card">
<h3 style="margin-bottom:15px;color:#333">💻 Система</h3>
<div class="info-grid">
<div class="info-item"><div class="info-label">⏱️ Время</div><div class="info-value" id="uptime">--</div></div>
<div class="info-item"><div class="info-label">🧠 RAM</div><div class="info-value" id="freeHeap">--</div></div>
<div class="info-item"><div class="info-label">📊 CPU</div><div class="info-value" id="cpuUsage">--</div></div>
<div class="info-item"><div class="info-label">📡 Канал</div><div class="info-value" id="wifiChannel">--</div></div>
<div class="info-item"><div class="info-label">📶 SSID</div><div class="info-value" id="ssid" style="font-size:13px">--</div></div>
<div class="info-item"><div class="info-label"><span id="wifiSignal">📶</span> RSSI</div><div class="info-value" id="rssi">--</div></div>
<div class="info-item"><div class="info-label">🌐 IP</div><div class="info-value" id="ipAddr" style="font-size:11px">--</div></div>
<div class="info-item"><div class="info-label">🔄 Переподкл.</div><div class="info-value" id="reconnects">--</div></div>
</div>
</div>
<div class="card chart-card">
<h3 style="margin-bottom:15px;color:#333">📈 История</h3>
</div>
<div class="card chart-card">
<h3 style="margin-bottom:15px;color:#333">🌡️ Температура</h3>
<canvas id="tempChart"></canvas>
<div class="update-time">Обновлено: <span id="updateTimeTemp">--</span></div>
</div>
<div class="card chart-card">
<h3 style="margin-bottom:15px;color:#333">💧 Влажность</h3>
<canvas id="humidChart"></canvas>
<div class="update-time">Обновлено: <span id="updateTimeHumid">--</span></div>
</div>
<div class="card chart-card">
<h3 style="margin-bottom:15px;color:#333">🔥 Ощущаемая</h3>
<canvas id="heatChart"></canvas>
<div class="update-time">Обновлено: <span id="updateTimeHeat">--</span></div>
</div>
</div>
</div>
<script>
let F=false,D={labels:[],temp:[],humid:[],heat:[]},T,H,E,P='3min',errCnt=0,iU,iS,iH;
const O={responsive:!0,maintainAspectRatio:!0,interaction:{mode:'index',intersect:!1},plugins:{legend:{display:!1},tooltip:{backgroundColor:'rgba(0,0,0,.8)',padding:15,titleFont:{size:14,weight:'bold'},bodyFont:{size:14},borderWidth:2,callbacks:{title:c=>'Время: '+c[0].label,label:c=>c.dataset.label+': '+c.parsed.y.toFixed(1)}}},scales:{x:{grid:{color:'rgba(0,0,0,.05)',drawBorder:!1},ticks:{font:{size:11},maxRotation:0,autoSkip:!0,maxTicksLimit:10}},y:{grid:{drawBorder:!1},ticks:{font:{size:12}}}},animation:{duration:750,easing:'easeInOutQuart'}};
function initCharts(){
const tc=document.getElementById('tempChart').getContext('2d');
T=new Chart(tc,{type:'line',data:{labels:D.labels,datasets:[{label:'Температура (°C)',data:D.temp,borderColor:'#667eea',backgroundColor:'rgba(102,126,234,.15)',tension:.4,fill:!0,borderWidth:3,pointRadius:0,pointHoverRadius:7,pointHoverBackgroundColor:'#667eea',pointHoverBorderColor:'white',pointHoverBorderWidth:3}]},options:{...O,plugins:{...O.plugins,tooltip:{...O.plugins.tooltip,borderColor:'#667eea'}},scales:{...O.scales,y:{...O.scales.y,title:{display:!0,text:'Температура (°C)',font:{size:13,weight:'bold'},color:'#667eea'},grid:{...O.scales.y.grid,color:'rgba(102,126,234,.1)'},ticks:{...O.scales.y.ticks,color:'#667eea'}}}}});
const hc=document.getElementById('humidChart').getContext('2d');
H=new Chart(hc,{type:'line',data:{labels:D.labels,datasets:[{label:'Влажность (%)',data:D.humid,borderColor:'#4facfe',backgroundColor:'rgba(79,172,254,.15)',tension:.4,fill:!0,borderWidth:3,pointRadius:0,pointHoverRadius:7,pointHoverBackgroundColor:'#4facfe',pointHoverBorderColor:'white',pointHoverBorderWidth:3}]},options:{...O,plugins:{...O.plugins,tooltip:{...O.plugins.tooltip,borderColor:'#4facfe'}},scales:{...O.scales,y:{...O.scales.y,title:{display:!0,text:'Влажность (%)',font:{size:13,weight:'bold'},color:'#4facfe'},grid:{...O.scales.y.grid,color:'rgba(79,172,254,.1)'},ticks:{...O.scales.y.ticks,color:'#4facfe'}}}}});
const ec=document.getElementById('heatChart').getContext('2d');
E=new Chart(ec,{type:'line',data:{labels:D.labels,datasets:[{label:'Ощущаемая (°C)',data:D.heat,borderColor:'#fa709a',backgroundColor:'rgba(250,112,154,.15)',tension:.4,fill:!0,borderWidth:3,pointRadius:0,pointHoverRadius:7,pointHoverBackgroundColor:'#fa709a',pointHoverBorderColor:'white',pointHoverBorderWidth:3}]},options:{...O,plugins:{...O.plugins,tooltip:{...O.plugins.tooltip,borderColor:'#fa709a'}},scales:{...O.scales,y:{...O.scales.y,title:{display:!0,text:'Ощущаемая (°C)',font:{size:13,weight:'bold'},color:'#fa709a'},grid:{...O.scales.y.grid,color:'rgba(250,112,154,.1)'},ticks:{...O.scales.y.ticks,color:'#fa709a'}}}}});
}
function c2f(c){return c*9/5+32}
function toggleUnit(){F=!F;updateDisplay()}
function updateDisplay(){
document.querySelectorAll('#tempUnit,#minTempUnit,#maxTempUnit,#avgTempUnit,#dewPointUnit,#heatIndexUnit').forEach(e=>e.textContent=F?'°F':'°C');
T.data.datasets[0].label=F?'Температура (°F)':'Температура (°C)';
T.options.scales.y.title.text=F?'Температура (°F)':'Температура (°C)';
E.data.datasets[0].label=F?'Ощущаемая (°F)':'Ощущаемая (°C)';
E.options.scales.y.title.text=F?'Ощущаемая (°F)':'Ощущаемая (°C)';
T.update('none');E.update('none');updateData();
}
function getComfort(v,isTemp){
if(isTemp){
if(v>=20&&v<=24)return{l:'excellent',t:'✅ Оптимально'};
if(v>=18&&v<=26)return{l:'good',t:'👍 Комфортно'};
if(v>=15&&v<=28)return{l:'fair',t:'⚠️ Допустимо'};
return{l:'poor',t:'❌ Некомфортно'};
}else{
if(v>=40&&v<=60)return{l:'excellent',t:'✅ Оптимально'};
if(v>=30&&v<=70)return{l:'good',t:'👍 Нормально'};
if(v>=25&&v<=75)return{l:'fair',t:'⚠️ Допустимо'};
return{l:'poor',t:'❌ Некомфортно'};
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
document.getElementById('lastUpdate').textContent='Обновлено: '+new Date().toLocaleTimeString('ru-RU');
errCnt=0;document.getElementById('statusBadge').className='status online';
document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Подключено</span>';
}).catch(e=>{console.error(e);errCnt++;if(errCnt>2){document.getElementById('statusBadge').className='status offline';document.getElementById('statusBadge').innerHTML='<div class="status-dot"></div><span>Нет связи</span>';}});
}
function updateStats(){
fetch('/stats').then(r=>r.json()).then(d=>{
document.getElementById('uptime').textContent=d.uptime;
document.getElementById('freeHeap').textContent=d.freeHeap;
document.getElementById('cpuUsage').textContent=d.cpuUsage+'%';
document.getElementById('wifiChannel').textContent=d.wifiChannel;
document.getElementById('ssid').textContent=d.ssid||'--';
document.getElementById('rssi').textContent=d.rssi+' dBm';
document.getElementById('ipAddr').textContent=d.ip;
document.getElementById('reconnects').textContent=d.reconnects;
const r=parseInt(d.rssi);document.getElementById('wifiSignal').textContent=r>-50?'📶':r>-60?'📶':r>-70?'📡':'📉';
}).catch(e=>console.error(e));
}
function updateHistory(){
fetch('/history').then(r=>r.json()).then(d=>{
let pts=60;
switch(P){case'3min':pts=60;break;case'10min':pts=200;break;case'30min':pts=600;break;case'60min':pts=1200;break;}
const si=Math.max(0,d.labels.length-pts);
D.labels=d.labels.slice(si);D.temp=d.temp.slice(si);D.humid=d.humid.slice(si);
if(d.heat){D.heat=d.heat.slice(si);}else{
D.heat=D.temp.map((t,i)=>{const h=D.humid[i];if(t<27)return t;
return -8.78469475556+1.61139411*t+2.33854883889*h+-0.14611605*t*h+-0.012308094*t*t+-0.0164248277778*h*h+0.002211732*t*t*h+0.00072546*t*h*h+-0.000003582*t*t*h*h;});
}
T.data.labels=D.labels;T.data.datasets[0].data=D.temp;T.update('none');
H.data.labels=D.labels;H.data.datasets[0].data=D.humid;H.update('none');
E.data.labels=D.labels;E.data.datasets[0].data=D.heat;E.update('none');
const ts=new Date().toLocaleTimeString('ru-RU');
document.getElementById('updateTimeTemp').textContent=ts;
document.getElementById('updateTimeHumid').textContent=ts;
document.getElementById('updateTimeHeat').textContent=ts;
}).catch(e=>console.error(e));
}
function changePeriod(p){P=p;document.querySelectorAll('.chart-btn').forEach(b=>b.classList.remove('active'));document.getElementById('btn-'+p).classList.add('active');updateHistory();}
function resetMinMax(){if(confirm('Сбросить min/max?')){fetch('/reset').then(r=>r.json()).then(d=>{alert(d.message||'Сброшено');updateData();}).catch(e=>alert('Ошибка'));}}
function rebootDevice(){if(confirm('⚠️ Перезагрузить устройство?')){fetch('/reboot').then(()=>{alert('Перезагрузка...');clearInterval(iU);clearInterval(iS);clearInterval(iH);setTimeout(()=>location.reload(),10000);}).catch(e=>console.error(e));}}
function exportCSV(){
let csv='Время,Температура,Влажность,Ощущаемая\n';
for(let i=0;i<D.labels.length;i++)csv+=`${D.labels[i]},${D.temp[i]},${D.humid[i]},${D.heat[i]}\n`;
const b=new Blob([csv],{type:'text/csv'});const u=URL.createObjectURL(b);const a=document.createElement('a');
a.href=u;a.download=`esp32_${new Date().toISOString().slice(0,10)}.csv`;a.click();URL.revokeObjectURL(u);
}
function exportJSON(){
const exp={timestamp:new Date().toISOString(),device:'ESP32+AHT10',data:{labels:D.labels,temperature:D.temp,humidity:D.humid,heatIndex:D.heat},current:{temperature:parseFloat(document.getElementById('temperature').textContent),humidity:parseFloat(document.getElementById('humidity').textContent),unit:F?'F':'C'}};
const b=new Blob([JSON.stringify(exp)],{type:'application/json'});const u=URL.createObjectURL(b);const a=document.createElement('a');
a.href=u;a.download=`esp32_${new Date().toISOString().slice(0,10)}.json`;a.click();URL.revokeObjectURL(u);
}
document.addEventListener('DOMContentLoaded',()=>{initCharts();updateData();updateStats();updateHistory();iU=setInterval(updateData,3000);iS=setInterval(updateStats,5000);iH=setInterval(updateHistory,3000);});
document.addEventListener('visibilitychange',()=>{if(document.hidden){clearInterval(iU);clearInterval(iS);clearInterval(iH);}else{updateData();updateStats();updateHistory();iU=setInterval(updateData,3000);iS=setInterval(updateStats,5000);iH=setInterval(updateHistory,3000);}});
</script>
</body>
</html>
)rawliteral";

#endif
