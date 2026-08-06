#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <U8g2lib.h> 

#define ADC_PIN 34
#define PIXEL_PIN 23
#define LED_COUNT 1
#define BUILTIN_LED 2

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Adafruit_NeoPixel pixel(LED_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define ADC_SAMPLES 1000
#define LED_SETTLE_MS 80
#define LOOP_DELAY_MS 1000
#define FINGER_THRESHOLD 1200

#define MAX_DATA 200
int hbData[MAX_DATA];
int gluData[MAX_DATA];
String timeStamp[MAX_DATA];
int dataIndex = 0;

const char *ssid = "ESP32_Monitor";
const char *password = "12345678";

WebServer server(80);
int ambientBaseline = 0;
int skinToneBaseline = 0; 

float mapGlucose(int gluAbs) {
  return map(gluAbs, 0, 1000, 20, 500);
}

float mapHemoglobin(int hbAbs) {
  hbAbs = constrain(hbAbs, 400, 1800);
  return map(hbAbs, 400, 1800, 120, 175) / 10.0;
}

int readADC(int samples = ADC_SAMPLES) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(ADC_PIN);
    delay(2);
  }
  return sum / samples;
}

int readADCtest(int samples = 50) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(ADC_PIN);
    delay(2);
  }
  return sum / samples;
}

int readADCgreen(int samples = ADC_SAMPLES) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    int p= analogRead(ADC_PIN);
    sum += p;
    delay(2);
  }
  int avg = sum / samples;
  Serial.println(avg);
  int boosted = avg ;
  return boosted;
}

void setPixel(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void trainAmbient() {
  long sum = 0;
  for (int i = 0; i < 40; i++) {
    sum += readADCtest();
    delay(30);
  }
  ambientBaseline = sum / 40;
}

String webPage() {
return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>HealthMonitor Pro</title>

<style>
:root {
 --bg:#0d1117;
 --card:#161b22;
 --text:#e6edf3;
 --red:#ff4d4d;
 --green:#2ea043;
 --border:#30363d;
}
body{
font-family:Segoe UI,Arial;
background:var(--bg);
color:var(--text);
margin:0;
padding:20px;
display:flex;
justify-content:center;
}
.container{max-width:500px;width:100%;}
.header h1{margin:0;}
.grid{
display:grid;
grid-template-columns:1fr 1fr;
gap:15px;
margin-bottom:20px;
}
.card{
background:var(--card);
border:1px solid var(--border);
border-radius:16px;
padding:20px;
}
.value{font-size:32px;font-weight:700;}
.chart-box{
background:var(--card);
border:1px solid var(--border);
border-radius:16px;
padding:15px 10px 10px 10px;
margin-bottom: 15px;
}
.chart-title {
margin: 0 0 10px 10px;
font-size: 14px;
font-weight: bold;
}
canvas{width:100%;height:200px;}
.btn{
display:block;
padding:15px;
text-align:center;
background:#21262d;
border-radius:12px;
color:#58a6ff;
text-decoration:none;
}
</style>
</head>

<body>
<div class="container">

<div class="header">
<h1>Vital Signs</h1>
<p>Live Monitoring System</p>
</div>

<div class="grid">
<div class="card" style="border-top:4px solid var(--red)">
<p>Hemoglobin</p>
<div class="value" id="hb">--</div>
<p>g/dL</p>
</div>

<div class="card" style="border-top:4px solid var(--green)">
<p>Glucose</p>
<div class="value" id="glu">--</div>
<p>mg/dL</p>
</div>
</div>

<div class="chart-box">
<div class="chart-title" style="color:var(--red)">Hemoglobin Trend (g/dL)</div>
<canvas id="hbGraph"></canvas>
</div>

<div class="chart-box">
<div class="chart-title" style="color:var(--green)">Glucose Trend (mg/dL)</div>
<canvas id="gluGraph"></canvas>
</div>

<a class="btn" href="/download">Export CSV Report</a>

</div>

<script>
let hb=[], glu=[];
const canvasHb=document.getElementById("hbGraph");
const ctxHb=canvasHb.getContext("2d");
const canvasGlu=document.getElementById("gluGraph");
const ctxGlu=canvasGlu.getContext("2d");

const MAX_POINTS = 200;

canvasHb.width = canvasHb.offsetWidth;
canvasHb.height = 200;
canvasGlu.width = canvasGlu.offsetWidth;
canvasGlu.height = 200;

function drawChart(ctx, width, height, data, color, min, max) {
  ctx.clearRect(0,0,width,height);
  
  const padX = 30; 
  const padY = 20; 
  const graphW = width - padX;
  const graphH = height - padY;

  ctx.fillStyle = "#8b949e";
  ctx.font = "12px Arial";
  ctx.textAlign = "right";
  ctx.textBaseline = "middle";
  ctx.lineWidth = 1;

  for(let i=0; i<=4; i++) {
    let y = (i * (graphH / 4));
    let val = max - (i * ((max-min)/4));
    
    ctx.fillText(val.toFixed(0), padX - 5, y + 10);

    ctx.strokeStyle = "#30363d";
    ctx.beginPath();
    ctx.moveTo(padX, y + 10);
    ctx.lineTo(width, y + 10);
    ctx.stroke();
  }

  ctx.textAlign = "left";
  ctx.fillText("Oldest", padX, height - 2);
  ctx.textAlign = "right";
  ctx.fillText("Now", width, height - 2);

  if(!data || data.length === 0) return;

  ctx.strokeStyle = color;
  ctx.lineWidth = 3;
  ctx.lineCap = "round";
  ctx.lineJoin = "round";
  ctx.shadowBlur = 6;
  ctx.shadowColor = color;

  ctx.beginPath();
  data.forEach((v, i) => {
    let x = padX + (i * (graphW / (MAX_POINTS - 1)));
    let y = 10 + graphH - ((v - min) / (max - min)) * graphH;
    
    if(i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.stroke();
  
  ctx.shadowBlur = 0; 
  ctx.fillStyle = color;
  
  data.forEach((v, i) => {
    let x = padX + (i * (graphW / (MAX_POINTS - 1)));
    let y = 10 + graphH - ((v - min) / (max - min)) * graphH;
    
    ctx.beginPath();
    ctx.arc(x, y, 5, 0, 2 * Math.PI); 
    ctx.fill();
  });
}

function draw(){
  drawChart(ctxHb, canvasHb.width, canvasHb.height, hb.map(v=>v/10), "#ff4d4d", 8, 20);
  drawChart(ctxGlu, canvasGlu.width, canvasGlu.height, glu, "#2ea043", 60, 250);
}

async function update(){
  const r = await fetch("/data");
  const j = await r.json();

  hb = j.hb;
  glu = j.glu;

  if(hb.length){
    document.getElementById("hb").innerHTML = (hb[hb.length-1]/10).toFixed(1);
    document.getElementById("glu").innerHTML = glu[glu.length-1];
    draw();
  }
}

setInterval(update,1000);
</script>

</body>
</html>
)rawliteral";
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN,ADC_11db);

  pinMode(BUILTIN_LED, OUTPUT); 
  pinMode(16, OUTPUT);

  pixel.begin();
  pixel.setBrightness(90);

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); 
  u8g2.drawStr(0, 20, "HealthMonitor Pro");
  u8g2.drawStr(0, 40, "Starting WiFi...");
  u8g2.sendBuffer();

  WiFi.softAP(ssid,password);
  server.on("/",[](){server.send(200,"text/html",webPage());});

  server.on("/data",[](){
    String json="{\"hb\":[";
    for(int i=0;i<dataIndex;i++){
      json+=String(hbData[i]);
      if(i<dataIndex-1)json+=",";
    }
    json+="],\"glu\":[";
    for(int i=0;i<dataIndex;i++){
      json+=String(gluData[i]);
      if(i<dataIndex-1)json+=",";
    }
    json+="]}";
    server.send(200,"application/json",json);
  });

  server.on("/download",[](){
    String csv="Time,Hb,Glucose\n";
    for(int i=0;i<dataIndex;i++){
      csv+=String(i)+","+String(hbData[i])+","+String(gluData[i])+"\n";
    }
    server.send(200,"text/csv",csv);
  });

  server.begin();

  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "Calibrating Sensor...");
  u8g2.drawStr(0, 40, "Keep finger OFF!");
  u8g2.sendBuffer();

  digitalWrite(BUILTIN_LED, HIGH); 
  pixel.setBrightness(255);
  setPixel(0, 0, 255);             
  
  trainAmbient();

  setPixel(0, 0, 0);
  digitalWrite(BUILTIN_LED, LOW);
}

int scanVitalStable(int totalSamples, const char* label, uint8_t r, uint8_t g, uint8_t b) {
  pixel.setBrightness(255);
  setPixel(r, g, b);
  delay(150); 

  const int numBlocks = 20;
  int samplesPerBlock = totalSamples / numBlocks;
  int blocks[numBlocks];

  for (int b_idx = 0; b_idx < numBlocks; b_idx++) {
    long blockSum = 0;
    for (int i = 0; i < samplesPerBlock; i++) {
      blockSum += analogRead(ADC_PIN);
      delay(2);
    }
    blocks[b_idx] = blockSum / samplesPerBlock;

    server.handleClient();
    int progress = map(b_idx + 1, 0, numBlocks, 0, 100);
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    
    int strWidth = u8g2.getStrWidth(label);
    u8g2.drawStr((128 - strWidth) / 2, 25, label); 
    
    u8g2.drawFrame(14, 40, 100, 10); 
    u8g2.drawBox(14, 40, progress, 10); 
    u8g2.sendBuffer();
  }

  setPixel(0, 0, 0); 

  for (int i = 0; i < numBlocks - 1; i++) {
    for (int j = i + 1; j < numBlocks; j++) {
      if (blocks[i] > blocks[j]) {
        int temp = blocks[i];
        blocks[i] = blocks[j];
        blocks[j] = temp;
      }
    }
  }

  long finalSum = 0;
  int count = 0;
  for (int i = numBlocks / 4; i < (numBlocks * 3) / 4; i++) {
    finalSum += blocks[i];
    count++;
  }

  return finalSum / count;
}


bool isFingerPresent() {
  setPixel(0, 0, 0);
  delay(50);
  int background = readADCtest(20);

  pixel.setBrightness(255);
  setPixel(0, 0, 255);
  delay(50);
  int activeLight = readADCtest(20);

  int difference = abs(activeLight - background);
  
  if (difference > 100) {
    return true; 
  }
  return false;
}


void loop() {
  server.handleClient();
  delay(100);
  
  if (!isFingerPresent()) { 
    delay(100); 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr); 
    u8g2.drawStr(0, 35, "Insert Finger...");
    u8g2.sendBuffer();
    return; 
  }
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  
  int w1 = u8g2.getStrWidth("Finger Detected!");
  u8g2.setCursor((128 - w1) / 2, 30);
  u8g2.print("Finger Detected!");

  int w2 = u8g2.getStrWidth("Please hold still...");
  u8g2.setCursor((128 - w2) / 2, 50);
  u8g2.print("Please hold still...");
  
  u8g2.sendBuffer();
  
  setPixel(0, 0, 0);
  digitalWrite(16, HIGH);
  delay(1500); 
  digitalWrite(16, LOW);

  skinToneBaseline = scanVitalStable(1000, "Skin Tone...", 255, 255, 255);
  
  Serial.print("Skin Tone Baseline: ");
  Serial.println(skinToneBaseline);

  int redVal = scanVitalStable(2000, "Analyzing Hb...", 255, 0, 0);
  int greenVal = scanVitalStable(2000, "Analyzing Glu...", 0, 255, 0);

  int hbAbs = redVal - ambientBaseline;
  int gluAbs = greenVal;
  
  if(hbAbs < 0) hbAbs = 0;
  if(gluAbs < 0) gluAbs = 0;

  float hb = mapHemoglobin(hbAbs);
  float glu = mapGlucose(gluAbs);

  if(dataIndex < MAX_DATA) {
    hbData[dataIndex] = hb * 10;
    gluData[dataIndex] = glu;
    dataIndex++;
  } else {
    for(int i = 1; i < MAX_DATA; i++) {
      hbData[i-1] = hbData[i];
      gluData[i-1] = gluData[i];
    }
    hbData[MAX_DATA-1] = hb * 10;
    gluData[MAX_DATA-1] = glu;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB12_tr); 
  
  u8g2.setCursor(0, 25);
  u8g2.print("Hb:  ");
  u8g2.print(hb, 1);
  
  u8g2.setCursor(0, 50); 
  u8g2.print("Glu: ");
  u8g2.print(glu, 0); 
  
  u8g2.sendBuffer();
  delay(4000); 

  String hbStatus = (hb < 12.0) ? "Anemic" : "Non-Anemic";
  String gluStatus = "";
  
  if (glu < 70) gluStatus = "Low";
  else if (glu <= 140) gluStatus = "Normal";
  else gluStatus = "High";

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr); 
  
  u8g2.setCursor(0, 25);
  u8g2.print("Hb: ");
  u8g2.print(hbStatus);
  
  u8g2.setCursor(0, 50); 
  u8g2.print("Glu: ");
  u8g2.print(gluStatus);
  
  u8g2.sendBuffer();
  delay(4000); 

  bool isHealthy = (hb >= 12.0 && hb <= 17.5 && glu >= 70 && glu <= 140);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr); 
  
  if (isHealthy) {
    int w = u8g2.getStrWidth("HEALTHY");
    u8g2.setCursor((128 - w) / 2, 35); 
    u8g2.print("HEALTHY");
  } else {
    int w = u8g2.getStrWidth("NOT HEALTHY");
    u8g2.setCursor((128 - w) / 2, 35); 
    u8g2.print("NOT HEALTHY");
  }
  
  u8g2.sendBuffer();
  delay(4000); 
}