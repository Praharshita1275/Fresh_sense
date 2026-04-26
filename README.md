# 🥦 FreshSense Pro
### AIoT-Based Food Spoilage Prediction System

FreshSense Pro is an AI-enabled IoT system designed to monitor and predict food spoilage using environmental and gas sensor data. It collects real-time data from sensors, processes it on an ESP32 microcontroller, uploads it to ThingSpeak Cloud, and uses that data to train machine learning models for predictive analysis.

---

## 🏗️ System Architecture

```
Sensors → ESP32 (Processing & Calibration) → ThingSpeak Cloud → ML Model
```

**Data Flow:**
1. Sensors capture temperature, humidity, and gas concentration
2. ESP32 processes and calibrates the raw data
3. Data is transmitted to ThingSpeak every 20 seconds
4. Cloud data is exported and used to train machine learning models

---

## 🔧 Hardware Components

| Component | Description |
|---|---|
| ESP32-S3 (Seeed XIAO) | Main microcontroller |
| MQ-135 Gas Sensor | VOC / gas concentration detection |
| DHT11 | Temperature & humidity sensing |
| LED Indicator | Visual freshness status |

---

## 💻 Software Stack

- **Embedded:** C++ (Arduino Framework), ESPAsyncWebServer, ArduinoJson, ThingSpeak API
- **Machine Learning:** Python (scikit-learn — Random Forest Classifier)

---

## ⚙️ Core Functional Logic

### Sensor Data Acquisition

The system continuously reads:
- Temperature (°C)
- Humidity (%)
- Gas concentration (VOC levels)

### Gas Sensor Calibration

**Resistance Calculation:**
$$Rs = RL \times \left(\frac{4095}{ADC_{raw}} - 1\right)$$

**Environmental Compensation:**
$$Correction = -0.015(T - 20) - 0.005(H - 33) + 1$$

**Gas Concentration Proxy (PPM):**
$$PPM_{proxy} = \left(\frac{R0}{Rs_{compensated}}\right) \times 1000$$

### Baseline Calibration (R0)
- Collects 50 samples during initialization
- Computes baseline resistance under clean air conditions
- Ensures stable long-term measurements

---

## ☁️ ThingSpeak Cloud Integration

Data is uploaded every **20 seconds** to the following fields:

| Field | Data |
|---|---|
| Field 1 | Temperature |
| Field 2 | Humidity |
| Field 3 | Gas Deviation |

```cpp
ThingSpeak.setField(1, t);
ThingSpeak.setField(2, h);
ThingSpeak.setField(3, deviation);

ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
```

### Sample Dataset

| Temperature | Humidity | Gas Deviation |
|---|---|---|
| 33.3 | 40.1 | 6.81 |
| 33.8 | 40.6 | 18.78 |

---

## 🤖 Machine Learning Pipeline

**Model:** Random Forest Classifier

**Workflow:**
1. Export data from ThingSpeak
2. Label data — `Fresh`, `Spoiling`, or `Spoiled`
3. Train the model using Python
4. Evaluate performance
5. Use model for real-time prediction

**Why Random Forest?**
- Handles non-linear relationships effectively
- Robust to noise in sensor data
- Provides feature importance insights

---

## 🚀 Real-Time System Features

### Non-Blocking Architecture
Uses `millis()` instead of `delay()`, enabling concurrent execution of:
- Sensor readings
- Web server operations
- Cloud communication

### LED Status Indication

| LED State | Meaning |
|---|---|
| OFF | Fresh condition |
| Slow Blink | Spoiling detected |
| Fast Blink | Critical condition |

---

## 🌐 Embedded Web Dashboard

- Built with **Tailwind CSS** and **JavaScript**
- Hosted directly on the ESP32
- Displays: Temperature, Humidity, Gas levels, Freshness score

---

## 🛠️ Setup & Installation

### 1. Hardware Setup
- Connect sensors to the ESP32
- Ensure the MQ-135 undergoes a **24-hour burn-in** before use

### 2. Arduino IDE — Install Libraries
- `ESPAsyncWebServer`
- `AsyncTCP`
- `DHT sensor library`
- `ArduinoJson`
- `ThingSpeak`

### 3. Configuration

```cpp
const char* ssid     = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

unsigned long myChannelNumber = YOUR_CHANNEL_ID;
const char* myWriteAPIKey     = "YOUR_API_KEY";
```

### 4. Upload Instructions
- Select board: **Seeed XIAO ESP32S3**
- Enable **USB CDC On Boot**
- Upload the code
- Open Serial Monitor at **115200 baud**

---

## 🔮 Future Enhancements

- [ ] Deploy ML model on ESP32 (edge inference)
- [ ] Add mobile/web monitoring interface
- [ ] Integrate advanced gas sensors
- [ ] Implement alert systems (SMS / Email)

---

## 📌 Conclusion

FreshSense Pro demonstrates a complete AIoT pipeline combining sensor data acquisition, cloud integration, and machine learning. The system enables real-time monitoring and predictive analysis of food spoilage — providing a scalable solution for smart food safety applications.
