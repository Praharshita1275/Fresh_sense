#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ThingSpeak.h>   // ✅ Correct

/* --- CONFIGURATION --- */
const char* ssid = "Viswamohan";
const char* password = "Geeta1981@";

/* --- THINGSPEAK CLOUD SETTINGS --- */
unsigned long myChannelNumber = 3357764;         // REPLACE WITH YOUR CHANNEL ID
const char * myWriteAPIKey = "YID86KOKPB62YX7S"; // REPLACE WITH YOUR API KEY

/* --- XIAO ESP32-S3 PINS --- */
#define DHTPIN 2       // D1 pin on XIAO
#define DHTTYPE DHT11
#define MQ135_PIN 1    // A0 pin on XIAO
#define LED_PIN 3      // D2 pin on XIAO

/* --- CALIBRATION CONSTANTS --- */
#define RL_VALUE 10.0 // 10kOhm load resistor
#define RO_CLEAN_AIR_FACTOR 3.6 

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);
WiFiClient client; // Needed for ThingSpeak

float R0 = 0; 
unsigned long startTime = 0;
int baseGasProxy = 277; 

// Non-Blocking Timers
unsigned long previousLedMillis = 0;
unsigned long previousThingSpeakMillis = 0;
int ledState = LOW;
int blinkInterval = 0; 

// Helper: Calculate Resistance from raw ADC
float getResistance(int rawADC) {
  if (rawADC <= 0) return 0;
  return RL_VALUE * (4095.0 - rawADC) / float(rawADC);
}

// Helper: Temp/Humidity compensation
float getCorrectionFactor(float t, float h) {
  return -0.015 * (t - 20.0) - 0.005 * (h - 33.0) + 1.0; 
}

/* --- THE ULTIMATE DASHBOARD (LIGHT/DARK) --- */
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en" class="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>FreshSense Intelligence</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <script>tailwind.config = { darkMode: 'class' }</script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;800&family=JetBrains+Mono:wght@500;800&display=swap');
        body { font-family: 'Inter', sans-serif; overflow-x: hidden; }
        .font-mono { font-family: 'JetBrains Mono', monospace; }
        .glass { backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px); transition: all 0.5s ease; }
        .score-ring-bg { fill: none; stroke-width: 8; transition: stroke 0.5s ease; }
        .dark .score-ring-bg { stroke: #1e293b; } 
        html:not(.dark) .score-ring-bg { stroke: #e2e8f0; } 
        .score-ring-fill { fill: none; stroke-width: 8; stroke-linecap: round; transition: stroke-dashoffset 1s ease-out, stroke 0.5s; }
        .metric-bar-bg { border-radius: 9999px; height: 8px; width: 100%; overflow: hidden; margin-top: 8px; transition: background-color 0.5s ease; }
        .dark .metric-bar-bg { background: #1e293b; }
        html:not(.dark) .metric-bar-bg { background: #e2e8f0; }
        .metric-bar-fill { height: 100%; border-radius: 9999px; transition: width 1s ease-out, background-color 0.5s; }
    </style>
</head>
<body class="bg-gradient-to-br from-slate-50 to-slate-200 dark:from-slate-900 dark:to-slate-950 text-slate-900 dark:text-slate-100 min-h-screen flex items-center justify-center p-4 md:p-8 transition-colors duration-500">
    
    <div class="w-full max-w-4xl grid grid-cols-1 md:grid-cols-3 gap-6 relative z-10">
        <div class="md:col-span-3 flex flex-col md:flex-row justify-between items-center gap-4 mb-2">
            <div>
                <h1 class="text-3xl font-extrabold tracking-tight flex items-center gap-3">
                    🌿 FreshSense <span class="text-emerald-500">OS</span>
                </h1>
                <div class="flex items-center gap-2 mt-1">
                    <span id="connDot" class="w-2.5 h-2.5 rounded-full bg-emerald-500 animate-pulse shadow-[0_0_8px_rgba(16,185,129,0.8)]"></span>
                    <span id="connText" class="text-xs text-slate-500 dark:text-slate-400 font-mono font-bold tracking-wider">LIVE / CLOUD SYNCED</span>
                </div>
            </div>
            
            <div class="flex items-end gap-4 w-full md:w-auto">
                <div class="flex-grow md:w-56">
                    <label class="text-[10px] text-slate-500 dark:text-slate-400 uppercase tracking-widest font-bold mb-1 block">Active Food Profile</label>
                    <select id="foodType" class="w-full bg-white/80 dark:bg-slate-800/80 backdrop-blur-md border border-slate-300 dark:border-slate-600 rounded-lg p-2.5 text-sm text-slate-900 dark:text-white focus:ring-2 focus:ring-emerald-500 outline-none cursor-pointer shadow-sm transition-colors">
                        <option value="fruit">🍎 Fruits (Ethylene)</option>
                        <option value="milk">🥛 Dairy (Ammonia)</option>
                        <option value="meat">🍗 Meat (Sulfur/Methane)</option>
                    </select>
                </div>
                <button onclick="toggleTheme()" class="h-[42px] w-[42px] flex-shrink-0 flex items-center justify-center rounded-lg bg-white/80 dark:bg-slate-800/80 border border-slate-300 dark:border-slate-600 shadow-sm hover:bg-slate-100 dark:hover:bg-slate-700 transition-all focus:outline-none">
                    <span id="themeIcon" class="text-lg">☀️</span>
                </button>
            </div>
        </div>

        <div class="glass bg-gradient-to-b from-white/90 to-slate-50/90 dark:from-slate-800/90 dark:to-slate-800/50 rounded-2xl p-6 flex flex-col items-center justify-center md:col-span-1 relative overflow-hidden border border-white/40 dark:border-slate-700/50 shadow-xl dark:shadow-2xl">
            <h2 class="text-[10px] text-slate-400 dark:text-slate-500 uppercase tracking-widest font-bold absolute top-6 left-6">System Status</h2>
            <div class="relative w-48 h-48 mt-8 mb-4">
                <svg viewBox="0 0 100 100" class="w-full h-full transform -rotate-90">
                    <circle class="score-ring-bg" cx="50" cy="50" r="42"></circle>
                    <circle id="scoreRing" class="score-ring-fill" cx="50" cy="50" r="42" stroke-dasharray="264" stroke-dashoffset="0" stroke="#10b981"></circle>
                </svg>
                <div class="absolute inset-0 flex flex-col items-center justify-center">
                    <span id="scoreText" class="text-5xl font-extrabold font-mono text-slate-800 dark:text-white">100</span>
                    <span class="text-[10px] text-slate-400 uppercase tracking-widest mt-1">Score</span>
                </div>
            </div>
            <h3 id="statusText" class="text-2xl font-bold text-emerald-500 mb-1">Optimal ✅</h3>
            <p id="predictionText" class="text-xs text-slate-500 dark:text-slate-400 text-center px-4 font-medium">Condition stable. Safe for storage.</p>
        </div>

        <div class="md:col-span-2 grid grid-cols-1 sm:grid-cols-2 gap-4">
            <div class="glass bg-gradient-to-br from-white/80 to-slate-100/80 dark:from-slate-800/80 dark:to-slate-800/40 border border-white/40 dark:border-slate-700/50 rounded-2xl p-5 flex flex-col justify-between shadow-lg">
                <div class="flex justify-between items-start mb-4">
                    <span class="text-xs text-slate-500 dark:text-slate-400 uppercase tracking-widest font-bold">Temperature</span>
                    <span class="text-xl drop-shadow-md">🌡️</span>
                </div>
                <div>
                    <div class="flex items-baseline gap-1">
                        <span id="temp" class="text-4xl font-extrabold font-mono text-slate-800 dark:text-white">0.0</span>
                        <span class="text-slate-400 font-mono text-sm">°C</span>
                    </div>
                    <div class="metric-bar-bg"><div id="tempBar" class="metric-bar-fill bg-blue-500 shadow-[0_0_10px_rgba(59,130,246,0.5)]" style="width: 0%"></div></div>
                </div>
            </div>

            <div class="glass bg-gradient-to-br from-white/80 to-slate-100/80 dark:from-slate-800/80 dark:to-slate-800/40 border border-white/40 dark:border-slate-700/50 rounded-2xl p-5 flex flex-col justify-between shadow-lg">
                <div class="flex justify-between items-start mb-4">
                    <span class="text-xs text-slate-500 dark:text-slate-400 uppercase tracking-widest font-bold">Relative Humidity</span>
                    <span class="text-xl drop-shadow-md">💧</span>
                </div>
                <div>
                    <div class="flex items-baseline gap-1">
                        <span id="hum" class="text-4xl font-extrabold font-mono text-slate-800 dark:text-white">0</span>
                        <span class="text-slate-400 font-mono text-sm">%</span>
                    </div>
                    <div class="metric-bar-bg"><div id="humBar" class="metric-bar-fill bg-cyan-500 shadow-[0_0_10px_rgba(6,182,212,0.5)]" style="width: 0%"></div></div>
                </div>
            </div>

            <div class="glass bg-gradient-to-br from-white/80 to-slate-100/80 dark:from-slate-800/80 dark:to-slate-800/40 border border-white/40 dark:border-slate-700/50 rounded-2xl p-5 flex flex-col justify-between sm:col-span-2 shadow-lg">
                <div class="flex justify-between items-start mb-2">
                    <span class="text-xs text-slate-500 dark:text-slate-400 uppercase tracking-widest font-bold">Gas Emission Index (MQ-135)</span>
                    <span id="ledStatus" class="text-[10px] font-mono font-bold text-slate-500 bg-slate-200/50 dark:bg-slate-900/50 border border-slate-300 dark:border-slate-700 px-2 py-1 rounded transition-colors">LED: OFF</span>
                </div>
                <div class="flex items-end justify-between">
                    <div>
                        <div class="flex items-baseline gap-2">
                            <span class="text-slate-400 dark:text-slate-500 font-mono text-lg">+</span>
                            <span id="gasDev" class="text-5xl font-extrabold font-mono text-emerald-500 transition-colors">0</span>
                            <span class="text-slate-400 dark:text-slate-500 font-mono text-xs ml-2 uppercase tracking-widest">PPM Proxy</span>
                        </div>
                    </div>
                    <div class="text-right">
                        <p class="text-[10px] text-slate-400 dark:text-slate-500 uppercase tracking-widest">Base R0</p>
                        <p id="r0val" class="text-sm font-mono font-bold text-slate-500 dark:text-slate-400">--</p>
                    </div>
                </div>
                <div class="metric-bar-bg mt-4"><div id="gasBar" class="metric-bar-fill bg-emerald-500 shadow-[0_0_10px_rgba(16,185,129,0.5)] transition-colors" style="width: 0%"></div></div>
            </div>
        </div>
    </div>

    <div class="fixed top-[-10%] left-[-10%] w-96 h-96 bg-emerald-500/10 dark:bg-emerald-500/5 rounded-full blur-3xl pointer-events-none"></div>
    <div class="fixed bottom-[-10%] right-[-10%] w-96 h-96 bg-blue-500/10 dark:bg-blue-500/5 rounded-full blur-3xl pointer-events-none"></div>

    <script>
        function toggleTheme() {
            const html = document.documentElement;
            html.classList.toggle('dark');
            document.getElementById('themeIcon').innerText = html.classList.contains('dark') ? '☀️' : '🌙';
        }

        const profiles = {
            fruit: { maxDev: 1500, timeDecay: 0.5 }, milk: { maxDev: 800, timeDecay: 1.0 }, meat: { maxDev: 400, timeDecay: 2.0 } 
        };
        let currentLedInterval = -1;

        function updateLogic() {
            fetch('/data').then(res => res.json()).then(data => {
                document.getElementById('connDot').className = "w-2.5 h-2.5 rounded-full bg-emerald-500 animate-pulse shadow-[0_0_8px_rgba(16,185,129,0.8)]";
                document.getElementById('connText').innerText = "LIVE / CLOUD SYNCED";

                const food = document.getElementById('foodType').value;
                const profile = profiles[food];
                
                let deviation = data.gasDev - data.baseGas;
                if (deviation < 0) deviation = 0;
                
                let gasImpact = (deviation / profile.maxDev) * 100;
                let timeHours = data.uptime / 3600;
                let score = Math.round(Math.max(0, Math.min(100, 100 - gasImpact - (timeHours * profile.timeDecay))));

                document.getElementById('temp').innerText = parseFloat(data.temp).toFixed(1);
                document.getElementById('hum').innerText = Math.round(data.hum);
                document.getElementById('gasDev').innerText = Math.round(deviation);
                document.getElementById('r0val').innerText = data.r0;
                document.getElementById('scoreText').innerText = score;

                // UI Status Update...
                const statusEl = document.getElementById('statusText'), predEl = document.getElementById('predictionText');
                const ring = document.getElementById('scoreRing'), ledDisplay = document.getElementById('ledStatus');
                const gasValText = document.getElementById('gasDev');
                
                ring.style.strokeDashoffset = 264 - (score / 100 * 264);
                let targetInterval = 0; 

                if (score >= 85) {
                    statusEl.innerText = "Optimal ✅"; statusEl.className = "text-2xl font-bold text-emerald-500 mb-1";
                    gasValText.className = "text-5xl font-extrabold font-mono text-emerald-500 transition-colors";
                    ring.style.stroke = "#10b981"; predEl.innerText = "Condition stable. Safe for storage.";
                    targetInterval = 0; ledDisplay.innerText = "LED: OFF (Fresh)";
                    ledDisplay.className = "text-[10px] font-mono font-bold text-emerald-600 dark:text-emerald-400 bg-emerald-100/50 dark:bg-emerald-900/30 border border-emerald-200 dark:border-emerald-800 px-2 py-1 rounded transition-colors";
                } else if (score >= 50) {
                    statusEl.innerText = "Warning ⚠️"; statusEl.className = "text-2xl font-bold text-amber-500 mb-1";
                    gasValText.className = "text-5xl font-extrabold font-mono text-amber-500 transition-colors";
                    ring.style.stroke = "#f59e0b"; 
                    let hoursLeft = Math.max(1, Math.round((score - 50) / (profile.timeDecay * 2 + (deviation/100))));
                    predEl.innerText = `Spoilage accelerating. Consume within ~${hoursLeft} hrs.`;
                    targetInterval = 1000; ledDisplay.innerText = "LED: SLOW BLINK (Spoiling)";
                    ledDisplay.className = "text-[10px] font-mono font-bold text-amber-600 dark:text-amber-400 bg-amber-100/50 dark:bg-amber-900/30 border border-amber-200 dark:border-amber-800 px-2 py-1 rounded transition-colors";
                } else {
                    statusEl.innerText = "Critical ❌"; statusEl.className = "text-2xl font-bold text-red-500 mb-1";
                    gasValText.className = "text-5xl font-extrabold font-mono text-red-500 transition-colors";
                    ring.style.stroke = "#ef4444"; 
                    if ((deviation / profile.maxDev) * 100 > 95) {
                        predEl.innerText = "CRITICAL: Extreme gas levels detected!"; targetInterval = 100; ledDisplay.innerText = "LED: VERY FAST (Critical)";
                    } else {
                        predEl.innerText = "Unsafe gas limits exceeded. Discard."; targetInterval = 300; ledDisplay.innerText = "LED: FAST BLINK (Spoiled)";
                    }
                    ledDisplay.className = "text-[10px] font-mono font-bold text-red-600 dark:text-red-400 bg-red-100/50 dark:bg-red-900/30 border border-red-200 dark:border-red-800 px-2 py-1 rounded animate-pulse transition-colors";
                }

                if(targetInterval !== currentLedInterval) {
                    currentLedInterval = targetInterval;
                    fetch(`/led?interval=${currentLedInterval}`);
                }
            }).catch(e => {
                document.getElementById('connDot').className = "w-2.5 h-2.5 rounded-full bg-red-500 shadow-[0_0_8px_rgba(239,68,68,0.8)]";
                document.getElementById('connText').innerText = "CONNECTION LOST";
            });
        }
        setInterval(updateLogic, 2000); setTimeout(updateLogic, 500); 
    </script>
</body>
</html>
)rawliteral";

/* --- ESP32 LOGIC --- */
void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(2000); 

  dht.begin();
  analogReadResolution(12);

  Serial.println("\n--- FreshSense Pro Booting ---");
  Serial.println("Warming up MQ135...");
  
  float rsSum = 0;
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if(isnan(t)) { t = 25.0; h = 50.0; } 
  float correction = getCorrectionFactor(t, h);

  for(int i = 0; i < 50; i++) {
    int raw = analogRead(MQ135_PIN);
    float rs = getResistance(raw);
    rsSum += (rs / correction); 
    delay(100);
  }
  
  R0 = (rsSum / 50.0) / RO_CLEAN_AIR_FACTOR;
  startTime = millis();
  
  // --- WiFi Setup ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nReady! Dashboard Link: http://" + WiFi.localIP().toString());

  // --- Initialize ThingSpeak ---
  ThingSpeak.begin(client);

  // --- Web Server Routes ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", DASHBOARD_HTML);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int currentGasRaw = analogRead(MQ135_PIN);

    if(isnan(t)) { t = 25.0; h = 50.0; } 

    float correction = getCorrectionFactor(t, h);
    float Rs = getResistance(currentGasRaw) / correction;
    float gasPPM_proxy = (Rs > 0) ? (R0 / Rs) * 1000 : 0;

    StaticJsonDocument<256> doc;
    doc["temp"] = String(t, 1);
    doc["hum"] = String(h, 0);
    doc["gasDev"] = gasPPM_proxy;  
    doc["baseGas"] = baseGasProxy; 
    doc["r0"] = String(R0, 2);
    doc["uptime"] = (millis() - startTime) / 1000;

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("interval")){
      blinkInterval = request->getParam("interval")->value().toInt();
      if(blinkInterval == 0) {
        ledState = LOW;
        digitalWrite(LED_PIN, ledState);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Non-Blocking LED Blink Logic
  if (blinkInterval > 0) {
    if (currentMillis - previousLedMillis >= blinkInterval) {
      previousLedMillis = currentMillis; 
      ledState = (ledState == LOW) ? HIGH : LOW;
      digitalWrite(LED_PIN, ledState);
    }
  }

  // 2. Non-Blocking ThingSpeak Cloud Sync (Every 20 Seconds)
  if (currentMillis - previousThingSpeakMillis >= 20000) {
    previousThingSpeakMillis = currentMillis;

    // Read Data for Cloud
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int raw = analogRead(MQ135_PIN);
    
    if(!isnan(t)) {
      float correction = getCorrectionFactor(t, h);
      float Rs = getResistance(raw) / correction;
      float gasPPM_proxy = (Rs > 0) ? (R0 / Rs) * 1000 : 0;
      float deviation = gasPPM_proxy - baseGasProxy;
      if(deviation < 0) deviation = 0;

      // Map to ThingSpeak Fields
      ThingSpeak.setField(1, t);
      ThingSpeak.setField(2, h);
      ThingSpeak.setField(3, deviation);

      // Send to Cloud
      int responseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      
      if(responseCode == 200){
        Serial.println("Cloud Sync: Success");
      } else {
        Serial.print("Cloud Sync: Failed (Error ");
        Serial.print(responseCode);
        Serial.println(")");
      }
    }
  }
}