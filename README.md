# 🥦 FreshSense Pro
### AIoT-Based Food Spoilage Prediction System

FreshSense Pro is an AI-enabled IoT system that monitors and predicts food spoilage in real time using environmental and gas sensor data. It runs on a Seeed XIAO ESP32-S3, streams data to ThingSpeak Cloud, serves a live web dashboard, and feeds a Random Forest ML model for spoilage prediction.

---

## 📋 Table of Contents

- [Hardware Requirements](#-hardware-requirements)
- [Wiring & Pin Connections](#-wiring--pin-connections)
- [Software & Library Setup](#-software--library-setup)
- [ThingSpeak Channel Setup](#-thingspeak-channel-setup)
- [Configuration](#-configuration)
- [Upload to ESP32](#-upload-to-esp32)
- [First Boot & Calibration](#-first-boot--calibration)
- [Accessing the Dashboard](#-accessing-the-web-dashboard)
- [ThingSpeak Cloud Sync](#-thingspeak-cloud-sync)
- [Machine Learning Pipeline](#-machine-learning-pipeline)
- [LED Status Guide](#-led-status-guide)
- [Troubleshooting](#-troubleshooting)
- [Future Enhancements](#-future-enhancements)

---

## 🔧 Hardware Requirements

| Component | Model | Quantity |
|---|---|---|
| Microcontroller | Seeed XIAO ESP32-S3 | 1 |
| Gas Sensor | MQ-135 | 1 |
| Temp & Humidity Sensor | DHT11 | 1 |
| LED | Any 5mm LED | 1 |
| Resistor | 220Ω (for LED) | 1 |
| Load Resistor (MQ-135) | 10kΩ | 1 |
| Breadboard + Jumper Wires | — | As needed |

---

## 🔌 Wiring & Pin Connections

```
DHT11 Data     →  GPIO 2  (D1)
MQ-135 AOUT    →  GPIO 1  (A0)
LED (+)        →  GPIO 3  (D2)  →  220Ω  →  GND
```

### Full Wiring Table

| Sensor/Component | Sensor Pin | ESP32-S3 Pin | Notes |
|---|---|---|---|
| DHT11 | VCC | 3.3V | |
| DHT11 | GND | GND | |
| DHT11 | DATA | GPIO 2 (D1) | 10kΩ pull-up to 3.3V recommended |
| MQ-135 | VCC | 5V (VIN) | **Must be 5V**, not 3.3V |
| MQ-135 | GND | GND | |
| MQ-135 | AOUT | GPIO 1 (A0) | Analog output |
| LED | Anode (+) | GPIO 3 (D2) | Through 220Ω resistor |
| LED | Cathode (−) | GND | |

> ⚠️ **Important:** The MQ-135 heater requires 5V on VCC. Use the **VIN pin** on the XIAO ESP32-S3 when powered via USB — do not use 3.3V.

---

## 💻 Software & Library Setup

### Step 1 — Install Arduino IDE
Download from [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

### Step 2 — Add ESP32 Board Support
1. Go to **File → Preferences**
2. Paste this into "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search `esp32` → install **esp32 by Espressif Systems**

### Step 3 — Install Required Libraries
Go to **Sketch → Include Library → Manage Libraries** and install:

| Library | Author |
|---|---|
| `DHT sensor library` | Adafruit |
| `ArduinoJson` | Benoit Blanchon |
| `ThingSpeak` | MathWorks |

> **ESPAsyncWebServer + AsyncTCP** must be installed manually from GitHub (not available in Library Manager):
> - https://github.com/me-no-dev/ESPAsyncWebServer
> - https://github.com/me-no-dev/AsyncTCP
>
> Download each as ZIP → **Sketch → Include Library → Add .ZIP Library**

---

## ☁️ ThingSpeak Channel Setup

1. Sign up / log in at [https://thingspeak.com](https://thingspeak.com)
2. Click **New Channel** and set up fields:

| Field | Name |
|---|---|
| Field 1 | Temperature |
| Field 2 | Humidity |
| Field 3 | Gas Deviation |

3. Save the channel
4. Go to the **API Keys** tab → copy your **Write API Key**
5. Note your **Channel ID** from the channel URL or overview page

---

## ⚙️ Configuration

Open the `.ino` file and update the following at the top:

```cpp
// WiFi Credentials
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak
unsigned long myChannelNumber = YOUR_CHANNEL_ID;       // e.g. 3357764
const char*   myWriteAPIKey   = "YOUR_WRITE_API_KEY";  // e.g. "YID86KOKPB62YX7S"

// Calibration — adjust if needed
#define RL_VALUE             10.0  // 10kΩ load resistor on MQ-135 module
#define RO_CLEAN_AIR_FACTOR   3.6  // From MQ-135 datasheet
int baseGasProxy = 277;            // Baseline PPM proxy in clean air
```

> ⚠️ **Never push real credentials to a public repo.** Use a `secrets.h` file and add it to `.gitignore`.

---

## 📤 Upload to ESP32

1. Connect the XIAO ESP32-S3 to your PC via USB-C
2. In Arduino IDE, configure:

| Setting | Value |
|---|---|
| Board | `Seeed XIAO ESP32S3` |
| Port | Your COM port (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux) |
| USB CDC On Boot | **Enabled** ✅ |

3. Click **Upload** (→ arrow)
4. Open **Tools → Serial Monitor** → set baud rate to **115200**

---

## 🚀 First Boot & Calibration

Watch the Serial Monitor after upload:

```
--- FreshSense Pro Booting ---
Warming up MQ135...
Connecting to WiFi..........
Ready! Dashboard Link: http://192.168.x.x
```

### Boot sequence explained:
1. **MQ-135 warm-up** — Collects 50 ADC samples to compute baseline resistance `R0` in clean air (~5 seconds)
2. **DHT11 init** — Reads temperature & humidity for environmental compensation
3. **WiFi connection** — Connects to your network; IP is printed to Serial Monitor
4. **ThingSpeak init** — Cloud client starts
5. **Web server starts** — Dashboard live at the printed IP

### ⚠️ MQ-135 Burn-In (First Time Only)
A brand-new MQ-135 needs **24 hours of continuous power** to stabilize its internal heater coil before readings are reliable. Run the circuit powered-on for a full day before collecting real data.

---

## 🌐 Accessing the Web Dashboard

1. After boot, find the IP address in Serial Monitor:
   ```
   Ready! Dashboard Link: http://192.168.1.105
   ```
2. Open any browser on the **same WiFi network**
3. Go to `http://192.168.1.105` (your IP will differ)

### Dashboard Features

| Feature | Description |
|---|---|
| 🌡️ Temperature | Live °C from DHT11 |
| 💧 Humidity | Live % RH from DHT11 |
| ⚗️ Gas Emission Index | Compensated PPM proxy from MQ-135 |
| 🔢 Freshness Score | Dynamic 0–100 score |
| 🍎 Food Profile | Switch between Fruit / Dairy / Meat sensitivity modes |
| 🌙 Dark / Light Mode | Toggle top-right |
| 📡 Connection Status | Live / Connection Lost indicator |

The dashboard polls the `/data` endpoint every **2 seconds** automatically.

---

## ☁️ ThingSpeak Cloud Sync

Data is pushed every **20 seconds** automatically. Monitor in Serial Monitor:

```
Cloud Sync: Success           ← HTTP 200 OK
Cloud Sync: Failed (Error -301)  ← Check API key or WiFi
```

To view your data online:
1. Log into [https://thingspeak.com](https://thingspeak.com)
2. Open your channel → view live Field Charts
3. To export: **Export → Download CSV** for ML training

---

## 🤖 Machine Learning Pipeline

### Step 1 — Install Python Dependencies
```bash
pip install pandas scikit-learn matplotlib
```

### Step 2 — Export Data from ThingSpeak
Download your channel data as a CSV file from the ThingSpeak export page.

### Step 3 — Label the Data
Add a `label` column to your CSV:

| Label | When to apply |
|---|---|
| `Fresh` | Low gas deviation, normal temp/humidity |
| `Spoiling` | Moderate and rising gas deviation |
| `Spoiled` | High deviation, elevated temp or humidity |

### Step 4 — Train the Model
```python
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report

df = pd.read_csv("thingspeak_export.csv")
X = df[["Temperature", "Humidity", "GasDeviation"]]
y = df["label"]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

print(classification_report(y_test, model.predict(X_test)))
```

### Step 5 — Run Predictions
```python
sample = [[33.5, 40.2, 12.5]]  # [temp, humidity, gas_deviation]
print(model.predict(sample))   # → ['Fresh']
```

---

## 💡 LED Status Guide

| LED Behavior | Meaning | Freshness Score |
|---|---|---|
| OFF (solid) | Fresh — stable condition | 85–100 |
| Slow Blink (~1000ms) | Spoiling — consume soon | 50–84 |
| Fast Blink (~300ms) | Spoiled — unsafe | 10–49 |
| Very Fast Blink (~100ms) | Critical — extreme gas levels | 0–9 |

---

## 🛠️ Troubleshooting

| Problem | Likely Cause | Fix |
|---|---|---|
| Serial Monitor shows garbage text | Wrong baud rate | Set to **115200** |
| WiFi not connecting | Wrong credentials | Re-check `ssid` / `password` |
| DHT11 reads `nan` | Loose wire or missing pull-up | Check wiring; add 10kΩ pull-up on DATA pin |
| MQ-135 reads 0 or near-0 | Sensor powered at 3.3V | Move VCC to **VIN (5V)** |
| Dashboard not loading | Wrong IP or different network | Check Serial Monitor for IP; use same WiFi |
| ThingSpeak error `-301` | Wrong API key or Channel ID | Re-check `myWriteAPIKey` and `myChannelNumber` |
| Gas readings drift / unstable | Burn-in not complete | Power MQ-135 for 24 hours before use |
| R0 calibrated wrong | Calibrated in polluted air | Redo boot in a fresh-air / ventilated space |

---

## 📁 Project Structure

```
FreshSense-Pro/
├── FreshSense_Pro.ino         # Main Arduino sketch
├── README.md                  # This file
└── ml/
    ├── thingspeak_export.csv  # Exported sensor dataset
    └── train_model.py         # Random Forest training script
```

---

## 🔮 Future Enhancements

- [ ] Edge inference — deploy trained model on ESP32 via TensorFlow Lite
- [ ] SMS / Email alerts via Twilio or SMTP on spoilage detection
- [ ] Mobile app for remote monitoring
- [ ] Add MQ-3 / MQ-4 for specific gas detection (alcohol, methane)
- [ ] OTA (Over-the-Air) firmware updates
- [ ] Local data logging to SD card

---

## 📌 System Architecture

```
DHT11 + MQ-135
       ↓
 ESP32-S3 XIAO
(Calibration + Processing)
       ↓               ↓
 Web Dashboard    ThingSpeak Cloud
 (http://IP)            ↓
                  Export CSV Data
                        ↓
                  Python ML Model
                        ↓
               Spoilage Prediction
               (Fresh / Spoiling / Spoiled)
```

---

## 📄 License

MIT License — free to use, modify, and distribute with attribution.
