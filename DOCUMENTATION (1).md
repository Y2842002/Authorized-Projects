# Authorized Projects — Full Technical Documentation

> **Author:** Youssef Osama
> **Repository:** `Y2842002/Authorized-Projects`
> **Last Updated:** April 2026
> **Version:** 2.0

---

## Table of Contents

1. [Repository Overview](#1-repository-overview)
2. [Project 1 — ABM: Adaptive Belt Monitoring](#2-project-1--abm-adaptive-belt-monitoring)
   - [2.1 What Is ABM?](#21-what-is-abm)
   - [2.2 How It Works — End-to-End Flow](#22-how-it-works--end-to-end-flow)
   - [2.3 System Architecture](#23-system-architecture)
   - [2.4 AI Models Deep Dive](#24-ai-models-deep-dive)
   - [2.5 Module Reference](#25-module-reference)
   - [2.6 Configuration Files](#26-configuration-files)
   - [2.7 MQTT Topics Reference](#27-mqtt-topics-reference)
   - [2.8 Scenarios Reference](#28-scenarios-reference)
   - [2.9 Step-by-Step Setup & Running](#29-step-by-step-setup--running)
   - [2.10 Troubleshooting](#210-troubleshooting)
3. [Project 2 — CubeSat Flight Software](#3-project-2--cubesat-flight-software)
   - [3.1 What Is the CubeSat Flight Software?](#31-what-is-the-cubesat-flight-software)
   - [3.2 How It Works — End-to-End Flow](#32-how-it-works--end-to-end-flow)
   - [3.3 Hardware & Sensors Reference](#33-hardware--sensors-reference)
   - [3.4 Code Structure](#34-code-structure)
   - [3.5 Serial Data Output Format](#35-serial-data-output-format)
   - [3.6 Step-by-Step Setup & Flashing](#36-step-by-step-setup--flashing)
   - [3.7 Troubleshooting](#37-troubleshooting)
4. [Project 3 — PWM Drawer](#4-project-3--pwm-drawer)
   - [4.1 What Is PWM Drawer?](#41-what-is-pwm-drawer)
   - [4.2 How It Works — End-to-End Flow](#42-how-it-works--end-to-end-flow)
   - [4.3 Hardware Architecture (AVR Layered Design)](#43-hardware-architecture-avr-layered-design)
   - [4.4 Module Reference](#44-module-reference)
   - [4.5 PWM Signal Modes](#45-pwm-signal-modes)
   - [4.6 State Machine: Signal Measurement](#46-state-machine-signal-measurement)
   - [4.7 Step-by-Step Setup & Build](#47-step-by-step-setup--build)
   - [4.8 Troubleshooting](#48-troubleshooting)
5. [Repository Structure](#5-repository-structure)

---

## 1. Repository Overview

This repository contains three independent embedded systems and AI engineering projects. Each project demonstrates a different domain of hardware-software integration — from industrial AI and IoT to aerospace telemetry and low-level AVR firmware.

| # | Project | Domain | Language / Platform | Key Technology |
|---|---|---|---|---|
| 1 | **ABM – Adaptive Belt Monitoring** | Industrial AI / IoT | Python + Unity 3D | MQTT, scikit-learn, Digital Twin |
| 2 | **CubeSat Flight Software** | Space / Embedded | Arduino (ESP32) | Multi-sensor telemetry, GPS, UART |
| 3 | **PWM Drawer** | Embedded Systems | C (AVR / ATmega32) | PWM generation & measurement, LCD |

Each project is fully self-contained with its own dependencies, hardware requirements, and build process. You do **not** need to set up all three to work on any one of them.

---

## 2. Project 1 — ABM: Adaptive Belt Monitoring

### 2.1 What Is ABM?

ABM (Adaptive Belt Monitoring) is an **AI-powered industrial monitoring system** that uses machine learning to diagnose the health of a belt-driven machine in real time. It connects to a **Unity 3D digital twin dashboard** over MQTT and reports:

- **Belt tension state** — Is the belt Loose, Optimal, or Tight?
- **System stability** — Is the machine running Stable or Unstable?
- **Motor health** — Is the motor in Good, Warning, or Critical condition?

**The core problem it solves:** Industrial belt systems fail silently. A loose or over-tight belt, or a degrading motor, are extremely difficult to detect before a breakdown actually occurs. ABM provides continuous, automated, early-warning diagnostics using vibration-based AI inference.

**Team:**

| Name | Role |
|---|---|
| Sara Hassan Mohamed | AI Engineer |
| Dalia Abdelmonem | AI Engineer |
| Mohamed Magdy | AI Engineer |
| Ramy Elhosary | Integration Engineer |
| Ahmed Mostafa | Unity Developer |
| Youssef Osama | Automation Engineer |

**Dataset:** Fraunhofer Institute open dataset
`https://fordatis.fraunhofer.de/handle/fordatis/347`

---

### 2.2 How It Works — End-to-End Flow

Understanding the system at a high level before diving into code is critical. Here is the complete lifecycle of a single diagnostic cycle:

```
Step 1 — Machine Power Signal
  Hardware/Unity publishes to MQTT topic: parallax/MACHINE_ON
  └─→ Python engine sets machine_on = True
      └─→ Publishes state: { machine_state: "ON", tension: "–", stability: "–", health: "–" }

Step 2 — Unity Sends a Scenario Command
  Unity publishes to MQTT topic: unity/motor/command
  Message: { "command": "RUN_SCENARIO", "scenario": "optimal_stable_good" }
  └─→ Python engine receives the message in on_message()
      └─→ Routes to handle_unity_command()

Step 3 — Validation
  handle_unity_command() checks:
    ├─ Is machine_on == True?  → proceed
    └─ Is machine_on == False? → reject, publish "Machine must be ON" error

Step 4 — Scenario Processing
  ScenarioHandler.process_scenario("optimal_stable_good") is called
  └─→ Looks up the scenario key in predefined scenario dictionaries
      └─→ Returns: { tension: "Optimal", stability: "Stable", health: "Good", ... }

Step 5 — AI Model Inference (optional, if .pkl files are present)
  ScenarioHandler.run_model_inference() is called alongside the lookup
  └─→ Loads models from models/ directory
      └─→ Returns model keys and inference note

Step 6 — Result Published Back to Unity
  MQTTInferenceEngine.publish_result() serialises the result to JSON
  └─→ Publishes to: unity/motor/status
      └─→ Unity dashboard reads and visualises the result in 3D

Step 7 — Machine Power Off
  Hardware/Unity publishes to: parallax/MACHINE_OFF
  └─→ Python engine sets machine_on = False
      └─→ Publishes state: { machine_state: "OFF", ... }
```

**Processing latency per cycle:**

| Step | Typical Duration |
|---|---|
| MQTT transmission | < 10 ms |
| Data validation | < 1 ms |
| Feature preprocessing | < 5 ms |
| Model inference | 20–50 ms |
| JSON serialisation | < 5 ms |
| **Total round-trip** | **< 100 ms** |

---

### 2.3 System Architecture

The system is structured into three distinct layers connected by MQTT:

```
┌──────────────────────────────────────────────┐
│             Unity 3D Dashboard               │
│                                              │
│  • Displays belt tension, stability, health  │
│  • Sends RUN_SCENARIO commands               │
│  • Sends MACHINE_ON / MACHINE_OFF signals    │
└──────────────────┬───────────────────────────┘
                   │  MQTT (bidirectional)
                   │  Port: 1883
                   ▼
┌──────────────────────────────────────────────┐
│           MQTT Broker                        │
│                                              │
│  Primary:  broker.hivemq.com:1883  (public)  │
│  Fallback: Local Mosquitto broker            │
└──────────────────┬───────────────────────────┘
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│               Python Inference Engine                    │
│                                                          │
│  ┌────────────────┐    ┌─────────────────┐               │
│  │  main.py       │───▶│ scenario_handler│               │
│  │  (MQTT engine, │    │ (scenario lookup │               │
│  │   callbacks,   │    │  + model load)  │               │
│  │   pub/sub)     │    └────────┬────────┘               │
│  └────────────────┘             │                        │
│                                 ▼                        │
│                        ┌────────────────┐                │
│                        │  models/       │                │
│                        │  *.pkl files   │                │
│                        │  (scikit-learn)│                │
│                        └────────────────┘                │
└──────────────────────────────────────────────────────────┘
```

> **Note:** The Python engine is the "brain." Unity is purely a visualisation and command interface. The MQTT broker is the message bus between them.

---

### 2.4 AI Models Deep Dive

The system uses two scikit-learn model pipelines, each targeting a different diagnostic question.

#### Model 1 — Binary Stability Classifier

| Property | Value |
|---|---|
| **File** | `stability_energy_model.pkl` |
| **Type** | Binary classifier |
| **Output** | `Stable` or `Unstable` |
| **Purpose** | Detects operational instability from energy-based vibration metrics |
| **Input features** | Vibration intensity, dominant frequency, spectral entropy |

**Supporting component — `se_computer (1).pkl`:**
This is not a classifier but a **pre-processing estimator** that computes the energy score from raw vibration signals. This energy score feeds into motor health drift detection (separate from the stability output).

#### Model 2 — Multi-Class Tension Classifier

| Property | Value |
|---|---|
| **Files** | `feature_extractor(3) (1).pkl` → `feature_scaler(3) (1).pkl` → `tension_classifier(3) (1).pkl` |
| **Type** | Multi-class classifier (3 classes) |
| **Output** | `Loose`, `Optimal`, or `Tight` |
| **Purpose** | Identifies the current belt tension condition |

**Inference pipeline (in order):**

```
Raw vibration features
       │
       ▼
feature_extractor(3) (1).pkl   ← Extracts meaningful features from raw input
       │
       ▼
feature_scaler(3) (1).pkl      ← Normalises features to a standard scale
       │
       ▼
tension_classifier(3) (1).pkl  ← Classifies into Loose / Optimal / Tight
       │
       ▼
Tension result (string)
```

> **Important:** Model `.pkl` files are **not** included in the repository due to file size constraints. You must obtain them separately and place them in the `models/` directory before running the engine. Without them, the system falls back to predefined scenario mappings (POC mode).

---

### 2.5 Module Reference

#### `main.py` — Entry Point & MQTT Engine

This is the **orchestrator** of the entire system. It creates the MQTT client, sets up all callbacks, and enters the MQTT event loop. It never terminates unless interrupted.

**Class: `MQTTInferenceEngine`**

| Method | Trigger | What It Does |
|---|---|---|
| `__init__()` | On instantiation | Creates MQTT client, initialises `ScenarioHandler`, registers all callbacks |
| `on_connect()` | Broker connection established | Subscribes to command/machine topics; publishes initial `"Ready"` state to Unity |
| `on_disconnect()` | Broker connection lost | Logs the unexpected disconnection for debugging |
| `on_message()` | Any MQTT message arrives | Reads the topic and routes to the correct handler method |
| `handle_machine_on()` | `parallax/MACHINE_ON` received | Sets `machine_on = True`; publishes `ON/Ready` status |
| `handle_machine_off()` | `parallax/MACHINE_OFF` received | Sets `machine_on = False`; publishes `OFF/Stopped` status |
| `handle_unity_command()` | `unity/motor/command` received | Parses `RUN_SCENARIO`; rejects if machine is OFF; calls `ScenarioHandler` |
| `publish_state()` | After any state change | Publishes a status JSON (machine state, tension, stability, health) |
| `publish_result()` | After scenario execution | Publishes full result JSON including `scenario` and `approach` fields |
| `connect()` | Called by `start()` | Reads `config.json`; connects to the broker |
| `start()` | Called from `__main__` | Calls `connect()` then enters `loop_forever()` — blocks indefinitely |

**JSON published on `unity/motor/status`:**

```json
{
  "timestamp": "2026-04-21T10:00:00.000",
  "machine_state": "ON",
  "tension": "Optimal",
  "stability": "Stable",
  "health": "Good",
  "scenario": "optimal_stable_good",
  "approach": "Approach 1"
}
```

---

#### `inference/scenario_handler.py` — Scenario Logic & Model Loader

This module is responsible for two things:
1. **Loading all `.pkl` model files** from the `models/` directory at startup.
2. **Mapping scenario keys** to predefined result dictionaries.

**Class: `ScenarioHandler`**

| Method | What It Does |
|---|---|
| `__init__()` | Calls `load_models()`; registers all Approach 1 and Approach 2 scenario dicts |
| `load_models()` | Iterates over expected filenames; loads each with `pickle.load()`; stores in internal dict |
| `process_scenario(name)` | Looks up `name` in the scenario registry; returns a result dict; optionally calls `run_model_inference()` |
| `run_model_inference(name)` | Placeholder for live sensor inference — currently returns loaded model keys and a status note |
| `get_error_result(name)` | Returns a structured error dict including a hint of the first 5 valid scenario names |
| `get_all_scenarios()` | Returns the full list of registered scenario keys |

**Approach 1 — Stability-Health Diagnostic Scenarios:**

These scenarios simulate combined belt + motor health readings.

| Scenario Key | Tension | Stability | Health | Interpretation |
|---|---|---|---|---|
| `loose_unstable_good` | Loose | Unstable | Good | Belt is loose and causing instability, but motor is fine |
| `optimal_stable_good` | Optimal | Stable | Good | Nominal operation — everything is healthy |
| `optimal_unstable_warning` | Optimal | Unstable | Warning | Belt tension is fine but vibration anomaly detected |
| `tight_stable_critical` | Tight | Stable | Critical | Belt too tight — motor stress is critical despite stable vibration |

**Approach 2 — Tension Adjustment Scenarios:**

These scenarios focus specifically on belt adjustment recommendations.

| Scenario Key | Tension | Health Recommendation |
|---|---|---|
| `adjustment_loose` | Loose | Requires Tightening |
| `adjustment_optimal` | Optimal | No Adjustment Needed |
| `adjustment_tight` | Tight | Requires Loosening |

---

#### `mqtt/mqtt_client.py` — Legacy MQTT Client (Reference Only)

An earlier-iteration MQTT client that accepted sensor data directly from Unity, ran `predictor.predict_all()`, and returned predictions. This module is **no longer active** — the current system uses `MQTTInferenceEngine` in `main.py` instead. It is retained in the repository as a reference for alternative integration patterns.

---

### 2.6 Configuration Files

#### `config.json` — Main System Configuration

```json
{
  "mqtt": {
    "broker": "broker.hivemq.com",
    "port": 1883,
    "client_id": "motor_inference_engine",
    "keepalive": 60
  },
  "system": {
    "name": "Belt Monitoring Inference Engine",
    "version": "1.0.0",
    "mode": "POC",
    "log_level": "INFO"
  }
}
```

| Field | Description |
|---|---|
| `broker` | MQTT broker hostname. Replace with `localhost` to use a local Mosquitto broker |
| `port` | Standard unencrypted MQTT port. Use `8883` for TLS |
| `client_id` | Unique identifier for this MQTT client on the broker |
| `keepalive` | Seconds between keepalive pings to the broker |
| `mode` | `"POC"` = predefined scenario mappings only. Change when live inference is integrated |
| `log_level` | Python logging level: `DEBUG`, `INFO`, `WARNING`, `ERROR` |

#### `config/mqtt_config.json`
Supplementary MQTT settings covering QoS levels, retain flags, and topic aliases.

#### `models/unity_config.json`
MQTT topic definitions shared with the Unity client. Unity reads this file to know which topics to subscribe to and publish on — keep this in sync with the Python engine's topic constants.

---

### 2.7 MQTT Topics Reference

| Topic | Direction | Message Type | Purpose |
|---|---|---|---|
| `unity/motor/command` | Unity → Python | JSON | Unity sends `RUN_SCENARIO` commands here |
| `unity/motor/status` | Python → Unity | JSON | Python publishes inference results here |
| `parallax/MACHINE_ON` | Hardware/Unity → Python | String | Signals machine power-on; enables scenario processing |
| `parallax/MACHINE_OFF` | Hardware/Unity → Python | String | Signals machine power-off; blocks scenario processing |

**Command message format (Unity → Python):**

```json
{
  "command": "RUN_SCENARIO",
  "scenario": "optimal_stable_good"
}
```

**State message format (Python → Unity — on power events):**

```json
{
  "timestamp": "2026-04-21T10:00:00.000",
  "machine_state": "OFF",
  "tension": "–",
  "stability": "–",
  "health": "–"
}
```

---

### 2.8 Scenarios Reference

All currently registered scenario keys are listed below. Use these exact strings in the `"scenario"` field of a `RUN_SCENARIO` command.

**Approach 1 scenarios:**
```
loose_unstable_good
optimal_stable_good
optimal_unstable_warning
tight_stable_critical
```

**Approach 2 scenarios:**
```
adjustment_loose
adjustment_optimal
adjustment_tight
```

If you send an unrecognised scenario key, the engine responds with a structured error that includes a hint of the first 5 valid scenario names.

---

### 2.9 Step-by-Step Setup & Running

Follow these steps in order. Do not skip steps — each one is a prerequisite for the next.

#### Prerequisites

- Python 3.8 or higher installed
- Internet connection (to reach HiveMQ public broker), or a local Mosquitto broker running
- The `.pkl` model files (if you want live AI inference rather than POC mode)

---

#### Step 1 — Clone or download the repository

```bash
git clone https://github.com/Y2842002/Authorized-Projects.git
cd Authorized-Projects/ABM-Adaptive-Belt-Monitoring-main
```

#### Step 2 — Create and activate a virtual environment (recommended)

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# macOS / Linux
python3 -m venv venv
source venv/bin/activate
```

> Using a virtual environment prevents version conflicts with other Python projects on your machine.

#### Step 3 — Install Python dependencies

```bash
pip install -r requirements.txt
```

The `requirements.txt` installs:

| Package | Version | Purpose |
|---|---|---|
| `numpy` | 1.24.3 | Numerical array operations for feature processing |
| `scikit-learn` | 1.3.0 | Loading and running `.pkl` model files |
| `paho-mqtt` | 1.6.1 | MQTT client library for broker communication |

#### Step 4 — Place model files

Obtain the five `.pkl` model files and copy them into the `models/` directory:

```
models/
├── stability_energy_model.pkl
├── se_computer (1).pkl
├── feature_extractor(3) (1).pkl
├── feature_scaler(3) (1).pkl
└── tension_classifier(3) (1).pkl
```

> **Without these files:** The system still runs in POC mode — scenarios are served from predefined dictionaries. The `.pkl` files are only required for live sensor inference.

#### Step 5 — Review configuration

Open `config.json` and confirm:
- `"broker"` is set to `"broker.hivemq.com"` (public) or `"localhost"` (local Mosquitto)
- `"port"` is `1883`
- `"mode"` is `"POC"` for predefined scenarios

#### Step 6 — Run the inference engine

```bash
python main.py
```

**Expected startup output:**

```
✅ Connected to MQTT Broker!
📥 Subscribed to: unity/motor/command
📥 Subscribed to: parallax/MACHINE_ON
📥 Subscribed to: parallax/MACHINE_OFF
🟢 System ready — waiting for commands...
```

The engine is now listening. It will block in the MQTT event loop until you press `Ctrl+C`.

#### Step 7 — Test with an MQTT client (without Unity)

You can test the engine using any MQTT client (e.g., **MQTTX**, **MQTT Explorer**, or `mosquitto_pub`).

**Power on the machine:**

```bash
mosquitto_pub -h broker.hivemq.com -t parallax/MACHINE_ON -m "ON"
```

**Run a scenario:**

```bash
mosquitto_pub -h broker.hivemq.com -t unity/motor/command \
  -m '{"command": "RUN_SCENARIO", "scenario": "optimal_stable_good"}'
```

**Listen for results:**

```bash
mosquitto_sub -h broker.hivemq.com -t unity/motor/status
```

**Expected result on `unity/motor/status`:**

```json
{
  "timestamp": "2026-04-21T10:05:30.112",
  "machine_state": "ON",
  "tension": "Optimal",
  "stability": "Stable",
  "health": "Good",
  "scenario": "optimal_stable_good",
  "approach": "Approach 1"
}
```

#### Step 8 — Connect Unity dashboard

Open the Unity project, configure it with the topic settings from `models/unity_config.json`, and run the scene. Unity will subscribe to `unity/motor/status` and publish commands to `unity/motor/command` automatically.

---

### 2.10 Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `Connection refused` on startup | Broker unreachable | Check internet connection; try switching to a local Mosquitto broker |
| Engine receives command but gives no response | Machine is OFF | Publish to `parallax/MACHINE_ON` first |
| `Unknown scenario` error | Typo in scenario key | Use `get_all_scenarios()` or refer to section 2.8 for exact keys |
| `ModuleNotFoundError: paho` | Dependencies not installed | Run `pip install -r requirements.txt` |
| Model files not loading | Files missing or misnamed | Check exact filenames match those listed in section 2.9 Step 4 |
| No output on `unity/motor/status` | Wrong topic subscription | Confirm you are subscribing to `unity/motor/status` not a variant |

---

## 3. Project 2 — CubeSat Flight Software

### 3.1 What Is the CubeSat Flight Software?

This is a **basic flight software implementation** for a CubeSat (small satellite) form-factor platform, running on an **ESP32 microcontroller**. The system collects environmental, inertial, positional, and radiation data from six onboard sensors and streams it as formatted telemetry over USB serial at a fixed 2-second interval.

**Purpose:** Serve as a functional, modular foundation for CubeSat subsystem integration and educational demonstration. Each sensor is independently initialised, read, and reported — making it straightforward to add, remove, or replace sensors.

**Key characteristics:**
- **No RTOS** — runs on the Arduino bare-metal loop (`setup()` / `loop()`)
- **Blocking init** — if a sensor fails to initialise, the system halts (`while(1)`), preventing corrupted data from being logged
- **Compact serial format** — all sensor data is packed onto a single line per cycle for easy parsing or logging

---

### 3.2 How It Works — End-to-End Flow

```
Power-on / Reset
      │
      ▼
setup()
  ├─ Serial.begin(115200)        ← Open USB serial at 115,200 baud
  ├─ initializeDHT22()           ← Temp/humidity sensor
  ├─ initializeBMP280()          ← Barometric pressure sensor
  ├─ initializeMPU6050()         ← IMU (accelerometer + gyroscope)
  ├─ initializeTSL2561()         ← Light intensity sensor
  ├─ initializeML8511()          ← UV radiation sensor
  ├─ initializeGPS()             ← GPS (UART2)
  └─ Print free heap memory      ← Diagnostic info at startup

      │  (if any sensor fails → while(1) halt)
      ▼
loop()  [runs indefinitely]
  ├─ Every 2,000 ms:
  │    └─ readAndDisplaySensors()
  │         ├─ Read DHT22   → temperature (°C), humidity (%)
  │         ├─ Read BMP280  → pressure (hPa), altitude (m)
  │         ├─ readMPU6050()→ accel X/Y/Z (raw), gyro X/Y/Z (raw)
  │         ├─ Read TSL2561 → light intensity (lux)
  │         ├─ Read ML8511  → UV intensity (mW/cm²), DUV index
  │         ├─ Read GPS     → lat, lon, alt, date, time
  │         └─ Print one compact line to Serial
  │
  └─ Continuously (every loop iteration):
       └─ Feed GPS NMEA bytes from Serial2 → TinyGPSPlus parser
```

> The GPS parser runs continuously (not just every 2 seconds) because GPS NMEA sentences arrive as a byte stream and must be consumed in real time to avoid buffer overflow.

---

### 3.3 Hardware & Sensors Reference

#### Sensor Pin Map

| Sensor | Model | Interface | GPIO Pins | Output |
|---|---|---|---|---|
| Temperature & Humidity | DHT22 | Digital 1-wire | GPIO 15 | Temp (°C), Humidity (%) |
| Barometric Pressure | BMP280 | Hardware SPI | GPIO 5 (CS), default MOSI/MISO/SCK | Pressure (hPa), Altitude (m) |
| IMU | MPU-6050 | I²C | SDA = GPIO 21, SCL = GPIO 22 | Accel X/Y/Z, Gyro X/Y/Z (raw int16) |
| GPS | Generic NMEA GPS | UART (Serial2) | RX = GPIO 16, TX = GPIO 17 | Lat, Lon, Alt, Date/Time |
| Light Intensity | TSL2561 | I²C | I²C address 0x39 | Lux (float) |
| UV Radiation | ML8511 | Analogue ADC | GPIO 34 (data), GPIO 14 (enable) | UV intensity (mW/cm²), DUV index |

#### Library Dependencies

| Library | Sensor | Source |
|---|---|---|
| `Adafruit_BMP280` | BMP280 pressure sensor | Bundled in `MCAL/` |
| `DHT sensor library` | DHT22 temperature/humidity | Bundled in `HAL/` |
| `TinyGPSPlus` | GPS NMEA parsing | Bundled in `HAL/` |
| `Adafruit_TSL2561_Unified` | TSL2561 light sensor | Bundled in `HAL/` |
| `ML8511` | UV sensor | Bundled in `HAL/` |
| `Wire` (I²C) | MPU-6050, TSL2561 | Arduino built-in |
| `Adafruit_GFX_Library` | OLED display support | Bundled in `MCAL/` |
| `Adafruit_SSD1306` | OLED display driver | Bundled in `MCAL/` |
| `EspSoftwareSerial` | Soft UART (if needed) | Bundled in `MCAL/` |

All library `.zip` files are pre-bundled in the `HAL/` and `MCAL/` directories for fully offline installation.

---

### 3.4 Code Structure

The entire application lives in `main.ino`. There are no separate `.cpp` / `.h` source files — all logic is contained in a single Arduino sketch.

#### `setup()` — Initialisation Phase

Runs once at boot. Performs the following in order:

1. Opens USB serial at 115,200 baud and waits for the port to be ready.
2. Calls each `initialize<Sensor>()` function sequentially.
3. Prints available heap memory as a startup health check.

Each `initialize<X>()` function follows this pattern:

```
Call sensor.begin() or sensor.init()
  ├─ Success → print "Sensor X initialised OK"
  └─ Failure → print "Sensor X NOT FOUND" → while(1) halt
```

This **blocking halt on failure** is intentional: it prevents the system from operating with incomplete sensor data and makes hardware wiring errors immediately obvious.

#### `loop()` — Runtime Phase

Runs continuously after `setup()`. Has two concurrent responsibilities:

| Task | Frequency | What it does |
|---|---|---|
| `readAndDisplaySensors()` | Every 2,000 ms | Reads all six sensors; prints a single formatted telemetry line |
| GPS feed | Every loop iteration | Feeds incoming bytes from `Serial2` to `TinyGPSPlus.encode()` |

#### `readAndDisplaySensors()` — Telemetry Output

Reads sensors sequentially in this order: DHT22 → BMP280 → MPU-6050 → TSL2561 → ML8511 → GPS. Then concatenates all readings into one compact serial line.

#### `readMPU6050()` — Direct I²C Register Read

The MPU-6050 does not use the standard Adafruit driver. Instead, the sketch reads **14 raw bytes** directly from the sensor's I²C register bank, starting at register `0x3B` (ACCEL_XOUT_H):

```
Bytes 0–1:  Accel X (int16, big-endian)
Bytes 2–3:  Accel Y
Bytes 4–5:  Accel Z
Bytes 6–7:  Temperature (skipped — not used)
Bytes 8–9:  Gyro X
Bytes 10–11: Gyro Y
Bytes 12–13: Gyro Z
```

The raw 16-bit signed integers are printed directly. No scaling to m/s² or °/s is applied in the current firmware.

---

### 3.5 Serial Data Output Format

Serial output is printed at **115,200 baud**, one line per 2-second cycle.

**Format:**

```
DHT:<humidity>%,<temp>°C|BMP:<pressure>hPa,<alt>m@MPU:-x:<ax>,y:<ay>,z:<az>|gx:<gx>,gy:<gy>,gz:<gz>&LIGHT:<lux>lux,UV:<uvIntensity>mW/cm² (DUV: <duvIndex>),GPS:<lat>,<lng>,<alt>m | Date: MM/DD/YYYY HH:MM:SS
```

**Example line:**

```
DHT:55.30%,23.10°C|BMP:1013.25hPa,12.50m@MPU:-x:320,y:-180,z:16200|gx:45,gy:-22,gz:8&LIGHT:312.50lux,UV:0.82mW/cm² (DUV: 0.03),GPS:30.123456,31.654321,45.2m | Date: 4/21/2026 10:00:00
```

**Field delimiters:**

| Delimiter | Separates |
|---|---|
| `\|` | DHT22 block from BMP280 block; MPU from LIGHT block |
| `@` | BMP280 block from MPU-6050 block |
| `&` | MPU-6050 block from LIGHT block |
| `,` | Values within the same sensor block |

**Special cases:**

| Condition | Output |
|---|---|
| GPS has no satellite fix | `GPS:NoFix` |
| Light sensor overloaded (too bright) | `LIGHT:OVF,` |

---

### 3.6 Step-by-Step Setup & Flashing

Follow these steps in order.

#### Prerequisites

- Arduino IDE 2.x **or** PlatformIO installed
- ESP32 board package installed in Arduino IDE
- USB cable connected to the ESP32 DevKit
- All sensors wired correctly (refer to section 3.3)

---

#### Step 1 — Install the ESP32 board package (Arduino IDE)

1. Open Arduino IDE.
2. Go to **File → Preferences**.
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**.
5. Search for `esp32` and install the package by Espressif Systems.

#### Step 2 — Install library dependencies from ZIP files

All required library ZIPs are already included in the repository.

1. In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library...**.
2. Navigate to `CubeSat Flight Software/HAL/` and install each ZIP:
   - `DHT_sensor_library-1.4.7.zip`
   - `ML8511-0.2.1.zip`
   - `TinyGPSPlus-1.0.3.zip`
   - `Adafruit_TSL2561-1.1.3.zip`
3. Navigate to `CubeSat Flight Software/MCAL/` and install each ZIP:
   - `Adafruit_GFX_Library-1.12.6.zip`
   - `Adafruit_SSD1306-2.5.16.zip`
   - `EspSoftwareSerial-8.1.0.zip`
4. Install `Adafruit_BMP280` via the Library Manager (**Sketch → Include Library → Manage Libraries** → search `BMP280`).

#### Step 3 — Wire the hardware

Connect sensors to the ESP32 according to the pin map in section 3.3. Key wiring notes:

- **BMP280** uses **hardware SPI** — do not wire it to I²C pins. CS pin goes to GPIO 5.
- **MPU-6050** uses **I²C** on GPIO 21 (SDA) and GPIO 22 (SCL) — these are the ESP32 default I²C pins.
- **GPS module** uses **UART2** at 9,600 baud — RX goes to GPIO 16, TX to GPIO 17.
- **ML8511 UV sensor** uses the **12-bit ADC** on GPIO 34. Ensure your ESP32 runs at 3.3V logic and that the `setVoltsPerStep(3.3, 4095)` call in the sketch matches your ADC reference.

#### Step 4 — Open the sketch

In Arduino IDE: **File → Open** → navigate to `CubeSat Flight Software/main.ino`.

#### Step 5 — Select board and port

1. Go to **Tools → Board → ESP32 Arduino** → select `ESP32 Dev Module` (or your exact variant).
2. Go to **Tools → Port** → select the COM port your ESP32 is connected to.

#### Step 6 — Upload the firmware

Click the **Upload** button (right arrow icon) or press `Ctrl+U`.

Wait for the output to show:
```
Leaving...
Hard resetting via RTS pin...
```

#### Step 7 — Open Serial Monitor and verify output

1. Go to **Tools → Serial Monitor** (or press `Ctrl+Shift+M`).
2. Set baud rate to **115,200**.
3. You should see startup messages followed by a telemetry line every 2 seconds.

**Expected startup output:**

```
ESP32 CubeSat Flight Software — Boot
DHT22 initialised OK
BMP280 initialised OK
MPU-6050 initialised OK
TSL2561 initialised OK
ML8511 initialised OK
GPS (Serial2) initialised OK
Free heap: 287432 bytes
--- Telemetry stream starting ---
DHT:55.30%,23.10°C|BMP:1013.25hPa,12.50m@MPU:-x:320,y:-180,z:16200|...
```

---

### 3.7 Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| System halts immediately after boot | One or more sensors not found | Check wiring; verify power supply to sensors; confirm I²C addresses |
| `GPS:NoFix` in every line | GPS has no satellite lock | Ensure GPS module has clear sky view; wait up to 90 seconds for cold fix |
| `LIGHT:OVF,` in output | TSL2561 saturation | Normal if directly facing a bright light source; move sensor |
| Garbled serial output | Wrong baud rate | Set Serial Monitor to exactly 115,200 |
| Upload fails with port error | Wrong COM port selected | Check Device Manager (Windows) or `ls /dev/tty*` (Linux/macOS) |
| MPU-6050 reads all zeros | I²C address conflict or wiring issue | Confirm SDA/SCL are on GPIO 21/22; check for address 0x68 conflict |
| UV readings seem incorrect | ADC voltage mismatch | Verify `setVoltsPerStep(3.3, 4095)` matches your board's actual ADC reference |

---

## 4. Project 3 — PWM Drawer

### 4.1 What Is PWM Drawer?

PWM Drawer is a **bare-metal C firmware** for the **ATmega32 AVR microcontroller** that does two things simultaneously:

1. **Generates** PWM signals at different frequencies and duty cycles, selectable via 4 physical switches.
2. **Measures** an incoming external PWM signal using hardware interrupts and a timer, then **displays** the computed values (frequency, period, duty cycle, T_on, T_off) on a 16×2 LCD.

The firmware is written entirely in C using a layered AVR driver architecture (UTIL → MCAL → HAL → Application), without any Arduino libraries or HAL wrappers. Everything from pin toggling to LCD communication is implemented from scratch.

**Author:** Yousef Osama Mohamed
**Created:** May 2024
**IDE:** Microchip Studio (Atmel Studio 7)

---

### 4.2 How It Works — End-to-End Flow

The system runs two parallel data paths on a single microcontroller:

```
┌────────────────────────────────────────────────────────┐
│              PATH A — PWM GENERATION                   │
│                                                        │
│  main.c reads PORTC switches (PC0–PC3)                 │
│       │                                                │
│       ▼                                                │
│  Selects a (frequency, duty) preset                    │
│       │                                                │
│       ▼                                                │
│  PWM_voidGenerate_PWM_Channel_1A(freq, duty)           │
│       │                                                │
│       ▼                                                │
│  Timer1 (Fast PWM mode) drives OC1A (PD5)             │
│       │                                                │
│       ▼                                                │
│  PWM signal output on PD5 pin                         │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│              PATH B — PWM MEASUREMENT                  │
│                                                        │
│  External PWM signal arrives on PD2 (INT0)             │
│       │                                                │
│       ▼                                                │
│  INT0 ISR fires on edge transitions                    │
│  Timer0 counts elapsed ticks between edges             │
│       │                                                │
│  State machine (States 0→1→2→3)                        │
│    State 0: Wait for rising edge                       │
│    State 1: Measure ON time (falling edge)             │
│    State 2: Measure period (next rising edge)          │
│    State 3: Trigger calculation in main loop           │
│       │                                                │
│       ▼                                                │
│  main loop detects State == 3                          │
│  Calls PWM calculation functions:                      │
│    ├─ PWM_voidDutyCycleCalculations()                  │
│    ├─ PWM_voidFrequencyCalculation()                   │
│    ├─ PWM_voidPeriodicTimeCalculations()               │
│    ├─ PWM_voidOnTimeDuration()                         │
│    └─ PWM_voidOffTimeDuration()                        │
│       │                                                │
│       ▼                                                │
│  LCD_voidDisplayPWMCalculations()                      │
│  LCD_voidDisplayPWMSignal()                            │
│       │                                                │
│       ▼                                                │
│  Results displayed on 16×2 LCD                        │
│  State resets to 0                                     │
└────────────────────────────────────────────────────────┘
```

---

### 4.3 Hardware Architecture (AVR Layered Design)

The project follows the standard AVR embedded driver architecture. Dependencies flow strictly downward — upper layers call lower layers but never vice versa.

```
┌──────────────────────────────────────────────────┐
│              Application Layer                   │
│                  main.c                          │
│  • Switch reading loop                           │
│  • PWM preset selection                          │
│  • State machine polling                         │
│  • LCD display calls                             │
└────────────────────┬─────────────────────────────┘
                     │ calls
┌────────────────────▼─────────────────────────────┐
│         HAL — Hardware Abstraction Layer         │
│                                                  │
│  LCD     — 16×2 display, 4-bit mode              │
│  KPD     — 4×4 matrix keypad                     │
│  SWITCH  — Push-button switch driver             │
│  EEPROM  — Internal ATmega32 EEPROM              │
└────────────────────┬─────────────────────────────┘
                     │ calls
┌────────────────────▼─────────────────────────────┐
│      MCAL — Microcontroller Abstraction Layer    │
│                                                  │
│  DIO    — Digital I/O (port pin config/read/write)│
│  EXTI   — External interrupt (INT0 on PD2)       │
│  PWM    — PWM generation + measurement math      │
│  TIMER0 — 8-bit timer (tick counting)            │
│  TWI    — I²C (Two-Wire Interface) driver        │
│  GI     — Global interrupt enable/disable        │
└────────────────────┬─────────────────────────────┘
                     │ calls
┌────────────────────▼─────────────────────────────┐
│              UTIL — Utility Layer                │
│                                                  │
│  STD_TYPES.h  — Custom portable type aliases     │
│  BIT_MATH.h   — Bit manipulation macros          │
└──────────────────────────────────────────────────┘
```

---

### 4.4 Module Reference

#### UTIL Layer

**`STD_TYPES.h`** — Custom type aliases that ensure consistent bit widths across compilers and AVR targets:

| Alias | Base Type | Width | Notes |
|---|---|---|---|
| `u8` | `unsigned char` | 8-bit unsigned | General-purpose byte |
| `s8` | `signed char` | 8-bit signed | |
| `u16` | `unsigned short int` | 16-bit unsigned | Timer registers |
| `s16` | `signed short int` | 16-bit signed | |
| `u32` | `unsigned long int` | 32-bit unsigned | Tick accumulation |
| `s32` | `signed long int` | 32-bit signed | |
| `f32` | `float` | 32-bit float | PWM calculation results |
| `f64` | `double` | 64-bit float | High-precision math |
| `BOOL` | `enum {false, true}` | 1 logical bit | State flags |

**`BIT_MATH.h`** — Macros for direct register-level bit manipulation:

| Macro | Operation | Example |
|---|---|---|
| `SET_BIT(reg, bit)` | Set a bit to 1 | `SET_BIT(PORTB, 3)` |
| `CLR_BIT(reg, bit)` | Clear a bit to 0 | `CLR_BIT(PORTB, 3)` |
| `TOG_BIT(reg, bit)` | Toggle a bit | `TOG_BIT(PORTB, 3)` |
| `GET_BIT(reg, bit)` | Read a bit's value | `if (GET_BIT(PINC, 0))` |

---

#### MCAL Layer

**`DIO`** — Digital I/O driver. Provides functions to configure port pins as input or output and to read or write their state. Used by all HAL drivers and the application.

**`EXTI`** — External interrupt driver. Configures **INT0 on PD2** for rising or falling edge detection. The INT0 ISR is the entry point for PWM measurement — each edge triggers a state transition.

**`TIMER0`** — 8-bit timer driver. Timer0 runs continuously and its overflow ISR increments `Global_ovfCounter`. Together, TCNT0 (the 8-bit count) and the overflow counter give a high-resolution tick count for measuring signal timing.

**`PWM`** — PWM generation and measurement math module:

| Function | Purpose |
|---|---|
| `PWM_voidInitChannel_1A()` | Initialises Timer1 Channel A in **Fast PWM mode** |
| `PWM_voidGenerate_PWM_Channel_1A(freq, duty)` | Generates a PWM signal at the specified frequency (Hz) and duty cycle (%) on **OC1A (PD5)** |
| `PWM_voidDutyCycleCalculations(...)` | Computes duty cycle (%) from captured ON ticks vs total period ticks |
| `PWM_voidFrequencyCalculation(...)` | Computes signal frequency (Hz) from total period ticks |
| `PWM_voidPeriodicTimeCalculations(...)` | Converts frequency to period in microseconds |
| `PWM_voidOnTimeDuration(...)` | Calculates T_on (µs) from period and duty cycle |
| `PWM_voidOffTimeDuration(...)` | Calculates T_off (µs) from period and T_on |

**`GI`** — Global interrupt enable/disable. Sets or clears the `I` (interrupt enable) bit in the AVR SREG register. Must be called before any ISR-driven feature (EXTI, TIMER0 overflow) can fire.

**`TWI`** — I²C (Two-Wire Interface) driver for inter-device communication. Included in the driver set for extensibility; not directly used in the core PWM measurement path.

---

#### HAL Layer

**`LCD`** — 16×2 character LCD driver operating in **4-bit mode** (saves 4 GPIO pins vs 8-bit mode):

| Function | Purpose |
|---|---|
| `LCD_voidInit()` | Initialises the LCD in 4-bit parallel mode; clears display |
| `LCD_voidDisplayString(str)` | Writes a null-terminated string to the current cursor position |
| `LCD_voidGoToSpecificPosition(row, col)` | Moves the cursor to a specific row (0 or 1) and column (0–15) |
| `LCD_voidDisplayPWMCalculations(freq, period, duty, Ton, Toff)` | Displays five computed PWM values across both LCD rows |
| `LCD_voidDisplayPWMSignal(duty, freq, Ton, Toff)` | Draws a visual ASCII waveform representation of the measured PWM signal |

Pin mapping for LCD is defined in `LCD_config.h`. Data pins connect to PORTB; control pins (RS, E) connect to PORTD.

**`KPD`** — 4×4 matrix keypad driver. Uses column-scanning to detect which key (if any) is pressed. Not used in the main PWM measurement loop — available for extensions.

**`SWITCH`** — Push-button switch driver. Reads switch state with support for **forward-connection (active-high)** wiring — pin reads HIGH when switch is pressed.

**`EEPROM`** — Reads from and writes to the ATmega32 internal EEPROM (512 bytes). Useful for persisting configuration or calibration data across power cycles.

---

### 4.5 PWM Signal Modes

The four switches on PORTC select PWM generation presets. Only one switch is read at a time (priority: SW1 > SW2 > SW3 > SW4). If no switch is pressed, a default preset is applied.

| Switch | Pin | Frequency | Duty Cycle | Characteristic |
|---|---|---|---|---|
| SW1 | PC0 | 25 Hz | 75% | Low frequency, high duty — long ON pulses |
| SW2 | PC1 | 50 Hz | 15% | Mid frequency, low duty — short ON pulses |
| SW3 | PC2 | 50 Hz | 85% | Mid frequency, high duty — mostly ON |
| SW4 | PC3 | 165 Hz | 95% | High frequency, very high duty — nearly always ON |
| *(none)* | — | 25 Hz | 10% | Default — low frequency, very short ON pulses |

**Simultaneously**, the system listens for an incoming external PWM signal on **PD2 (INT0)** and measures it — the generation and measurement paths run independently.

---

### 4.6 State Machine: Signal Measurement

The PWM measurement logic is implemented as a **3-state interrupt-driven state machine** using INT0 and Timer0. Understanding this is key to understanding how the firmware measures an unknown PWM signal.

**State diagram:**

```
         ┌──────────────────────────────────────────────────┐
         │                                                  │
         ▼                                                  │
  ┌─────────────┐   Rising edge on PD2                      │
  │  STATE 0    │──────────────────────────────────────────▶│
  │   (Idle)    │   • Reset TCNT0 and overflow counter       │
  └─────────────┘   • Configure INT0 for FALLING edge        │
                    • Transition → State 1                   │
                                                             │
  ┌─────────────┐   Falling edge on PD2                      │
  │  STATE 1    │   • Capture TCNT0 + overflow as "ON ticks" │
  │ (ON timing) │   • Configure INT0 for RISING edge         │
  └──────┬──────┘   • Transition → State 2                   │
         │                                                   │
         │          Next rising edge on PD2                  │
  ┌──────▼──────┐   • Capture TCNT0 + overflow as "total"   │
  │  STATE 2    │   • Transition → State 3                   │
  │(Period meas)│                                            │
  └──────┬──────┘                                            │
         │                                                   │
  ┌──────▼──────┐   main loop polls Global_state == 3        │
  │  STATE 3    │   • Compute duty, freq, period, Ton, Toff  │
  │ (Calculate) │   • Display results on LCD                 │
  └──────┬──────┘   • Reset → State 0                       │
         │                                                   │
         └──────────────────────────────────────────────────┘
```

**Global variables used by the ISR (shared between ISR and main loop):**

| Variable | Type | Purpose |
|---|---|---|
| `Global_state` | `u8` | Current state (0–3); main loop polls this |
| `Global_ovfCounter` | `u32` | Number of Timer0 overflows since last reset |
| `Global_onTicks` | `u8` | TCNT0 value captured at falling edge (end of ON period) |
| `Global_onCounter` | `u32` | Overflow count at falling edge |
| `Global_totalTicks` | `u8` | TCNT0 value at second rising edge (end of full period) |
| `Global_totalCounter` | `u32` | Overflow count at second rising edge |

> All ISR-shared variables must be declared `volatile` to prevent the compiler from caching them in registers.

**ISR vectors:**

| Vector | Source | Action |
|---|---|---|
| `__vector_11` | Timer0 overflow | Increments `Global_ovfCounter` |
| `__vector_1` | INT0 (PD2 edge) | Executes state transitions; captures tick values |

---

### 4.7 Step-by-Step Setup & Build

#### Prerequisites

- **ATmega32** microcontroller on a target board (or Proteus simulation — schematic included as `PWM_DESIGN.DSN`)
- **16 MHz crystal** oscillator connected to the ATmega32
- **AVR programmer** (USBasp, AVRISP mkII, or similar) and AVRDUDE installed — if flashing physical hardware
- **Microchip Studio (Atmel Studio 7)** installed on Windows — for building from source
- 16×2 LCD wired to PORTB (data) + PORTD (RS, E) per `LCD_config.h`
- Switches wired to PC0–PC3 (active-high / forward-connected)

---

#### Step 1 — Open the project in Microchip Studio

1. Launch **Microchip Studio (Atmel Studio 7)**.
2. Go to **File → Open → Project/Solution**.
3. Navigate to `PWM_drawer-main/PWM_drawer/` and open `PWM_drawer.atsln`.

#### Step 2 — Verify target MCU settings

1. Go to **Project → Properties** (or right-click the project in Solution Explorer).
2. Confirm **Device** is set to `ATmega32`.
3. Confirm **F_CPU** = `16000000UL` in `main.c` or project preprocessor symbols.

#### Step 3 — Build the project

Press **F7** or go to **Build → Build Solution**.

A successful build outputs:
```
Build succeeded.
   0 Error(s)
   0 Warning(s)
```

Pre-built binaries are available in `Debug/` if you want to skip the build:
- `PWM_drawer.hex` — Ready to flash to the MCU
- `PWM_drawer.elf` — For debugging with a JTAG probe

#### Step 4 — Flash to the ATmega32 via AVRDUDE

Connect your AVR programmer and run:

```bash
avrdude -c usbasp -p m32 -U flash:w:PWM_drawer.hex
```

| Flag | Meaning |
|---|---|
| `-c usbasp` | Programmer type — change to `avrispmkii`, `stk500v2`, etc. as needed |
| `-p m32` | Target MCU: ATmega32 |
| `-U flash:w:PWM_drawer.hex` | Write the hex file to program flash |

**Expected AVRDUDE output:**

```
avrdude: AVR device initialized and ready to accept instructions
avrdude: Device signature = 0x1e9502 (probably m32)
avrdude: reading input file "PWM_drawer.hex"
avrdude: writing flash (XXXX bytes):
Writing | ################################################## | 100% 
avrdude: verifying flash memory against PWM_drawer.hex:
avrdude done.  Thank you.
```

#### Step 5 — Simulate in Proteus (optional, no hardware required)

1. Open **Proteus** and load `PWM_DESIGN.DSN` from the project root.
2. Set the MCU's Program File property to the path of `PWM_drawer.hex`.
3. Run the simulation. The LCD and switches are pre-wired in the schematic.

#### Step 6 — Verify hardware operation

After flashing:

1. Press **SW1 (PC0)** — LCD should show: `25Hz 75%`
2. Apply an external PWM signal to **PD2** — the LCD should update with measured frequency, period, duty cycle, T_on, and T_off after one complete period is captured.
3. Press **SW2–SW4** to cycle through other presets.
4. Release all switches — the default 25Hz / 10% preset is applied.

---

### 4.8 Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| LCD shows nothing after flash | LCD init failed or wrong wiring | Check PORTB/PORTD connections against `LCD_config.h` |
| LCD shows garbled characters | Wrong 4-bit timing or contrast too low | Adjust contrast potentiometer; verify LCD init sequence |
| PWM output not detected on PD5 | Timer1 not running | Verify `PWM_voidInitChannel_1A()` is called before `generate` |
| Measurement never triggers (State stays 0) | INT0 not receiving signal | Check PD2 wiring; verify external PWM source is toggling |
| Frequency reads as zero or garbage | Overflow counter not resetting between measurements | Confirm `Global_ovfCounter` is reset in State 0 |
| AVRDUDE: cannot open device | Wrong programmer or COM port | Specify correct `-P` port; check USB driver for USBasp |
| AVRDUDE: device signature mismatch | Wrong MCU target | Confirm `-p m32` for ATmega32; check ISP header orientation |
| Build error: undefined reference | Missing source file in project | Ensure all `.c` driver files are included in the Microchip Studio project |

---

## 5. Repository Structure

```
Authorized-Projects-main/
│
├── README.md                              ← Top-level project overview
│
├── ABM-Adaptive-Belt-Monitoring-main/     ← Project 1
│   ├── main.py                            ← Entry point: MQTT engine + MQTTInferenceEngine
│   ├── config.json                        ← MQTT broker settings + system config
│   ├── requirements.txt                   ← Python dependencies (numpy, scikit-learn, paho-mqtt)
│   │
│   ├── inference/
│   │   ├── __init__.py
│   │   └── scenario_handler.py            ← Scenario registry + model loading + inference routing
│   │
│   ├── mqtt/
│   │   ├── __init__.py
│   │   └── mqtt_client.py                 ← Legacy MQTT client (reference only — not active)
│   │
│   ├── models/
│   │   └── unity_config.json              ← MQTT topic definitions shared with Unity
│   │   (+ *.pkl files — not in repo, must be added manually)
│   │
│   ├── config/
│   │   └── mqtt_config.json               ← Supplementary MQTT settings (QoS, retain, aliases)
│   │
│   ├── docs/
│   │   ├── System_Architecture.md         ← Architecture diagrams and design notes
│   │   ├── MQTT_Integration_Guide.md      ← Unity integration guide
│   │   └── QUICK_START.md                 ← Condensed run instructions
│   │
│   └── demo/
│       └── ABM Demo Video.mp4             ← Live inference demonstration recording
│
├── CubeSat Flight Software/               ← Project 2
│   ├── main.ino                           ← Complete Arduino sketch (all logic in one file)
│   │
│   ├── HAL/                               ← Sensor library ZIPs (offline installation)
│   │   ├── Adafruit_TSL2561-1.1.3.zip     ← Light intensity sensor
│   │   ├── DHT_sensor_library-1.4.7.zip   ← Temperature/humidity sensor
│   │   ├── ML8511-0.2.1.zip               ← UV radiation sensor
│   │   └── TinyGPSPlus-1.0.3.zip          ← GPS NMEA parser
│   │
│   └── MCAL/                              ← Display library ZIPs
│       ├── Adafruit_GFX_Library-1.12.6.zip← Core GFX primitives
│       ├── Adafruit_SSD1306-2.5.16.zip    ← OLED display driver
│       └── EspSoftwareSerial-8.1.0.zip    ← Software UART (if needed)
│
└── PWM_drawer-main/                       ← Project 3
    ├── PWM_DESIGN.DSN                     ← Proteus circuit schematic
    ├── PWM_DESIGN.PWI                     ← Proteus workspace settings
    │
    └── PWM_drawer/
        ├── main.c                         ← Application entry point (switch reading, state polling)
        │
        ├── UTIL/                          ← Utility layer
        │   ├── STD_TYPES.h                ← Custom portable type aliases (u8, u16, f32, ...)
        │   └── BIT_MATH.h                 ← Bit manipulation macros (SET_BIT, CLR_BIT, ...)
        │
        ├── MCAL/                          ← Microcontroller abstraction layer
        │   ├── DIO/                       ← Digital I/O driver (inc/ + src/)
        │   ├── PWM/                       ← PWM generation + measurement math (inc/ + src/)
        │   ├── TIMER0/                    ← 8-bit timer driver (inc/ + src/)
        │   ├── EXTI/                      ← External interrupt driver, INT0 (inc/ + src/)
        │   ├── TWI/                       ← I²C driver (inc/ + src/)
        │   └── Global_Interrupt_Enable/   ← GI enable/disable (inc/ + src/)
        │
        ├── HAL/                           ← Hardware abstraction layer
        │   ├── LCD/                       ← 16×2 LCD driver, 4-bit mode (inc/ + src/)
        │   ├── KPD/                       ← 4×4 keypad driver (inc/ + src/)
        │   ├── SWITCH/                    ← Push-button switch driver (inc/ + src/)
        │   └── EEPROM/                    ← Internal EEPROM driver (inc/ + src/)
        │
        └── Debug/                         ← Pre-built binaries
            ├── PWM_drawer.hex             ← Flash directly with AVRDUDE
            └── PWM_drawer.elf             ← ELF file for JTAG debugging
```

---

*Documentation enhanced and expanded — April 2026 | Youssef Osama*
