# ABM — Adaptive Belt Monitoring
## Project Documentation

> **Author:** Youssef Osama
> **Repository:** `Y2842002/Authorized-Projects` → `ABM-Adaptive-Belt-Monitoring-main/`
> **Last Updated:** April 2026

---

## Table of Contents

1. [What Is ABM?](#1-what-is-abm)
2. [How It Works — End-to-End Flow](#2-how-it-works--end-to-end-flow)
3. [System Architecture](#3-system-architecture)
4. [AI Models Deep Dive](#4-ai-models-deep-dive)
5. [Module Reference](#5-module-reference)
6. [Configuration Files](#6-configuration-files)
7. [MQTT Topics Reference](#7-mqtt-topics-reference)
8. [Scenarios Reference](#8-scenarios-reference)
9. [Step-by-Step Setup & Running](#9-step-by-step-setup--running)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. What Is ABM?

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

## 2. How It Works — End-to-End Flow

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

## 3. System Architecture

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

## 4. AI Models Deep Dive

The system uses two scikit-learn model pipelines, each targeting a different diagnostic question.

### Model 1 — Binary Stability Classifier

| Property | Value |
|---|---|
| **File** | `stability_energy_model.pkl` |
| **Type** | Binary classifier |
| **Output** | `Stable` or `Unstable` |
| **Purpose** | Detects operational instability from energy-based vibration metrics |
| **Input features** | Vibration intensity, dominant frequency, spectral entropy |

**Supporting component — `se_computer (1).pkl`:**
This is not a classifier but a **pre-processing estimator** that computes the energy score from raw vibration signals. This energy score feeds into motor health drift detection (separate from the stability output).

### Model 2 — Multi-Class Tension Classifier

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

## 5. Module Reference

### `main.py` — Entry Point & MQTT Engine

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

### `inference/scenario_handler.py` — Scenario Logic & Model Loader

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

| Scenario Key | Tension | Stability | Health | Interpretation |
|---|---|---|---|---|
| `loose_unstable_good` | Loose | Unstable | Good | Belt is loose and causing instability, but motor is fine |
| `optimal_stable_good` | Optimal | Stable | Good | Nominal operation — everything is healthy |
| `optimal_unstable_warning` | Optimal | Unstable | Warning | Belt tension is fine but vibration anomaly detected |
| `tight_stable_critical` | Tight | Stable | Critical | Belt too tight — motor stress is critical despite stable vibration |

**Approach 2 — Tension Adjustment Scenarios:**

| Scenario Key | Tension | Health Recommendation |
|---|---|---|
| `adjustment_loose` | Loose | Requires Tightening |
| `adjustment_optimal` | Optimal | No Adjustment Needed |
| `adjustment_tight` | Tight | Requires Loosening |

---

### `mqtt/mqtt_client.py` — Legacy MQTT Client (Reference Only)

An earlier-iteration MQTT client that accepted sensor data directly from Unity, ran `predictor.predict_all()`, and returned predictions. This module is **no longer active** — the current system uses `MQTTInferenceEngine` in `main.py` instead. It is retained in the repository as a reference for alternative integration patterns.

---

## 6. Configuration Files

### `config.json` — Main System Configuration

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

### `config/mqtt_config.json`
Supplementary MQTT settings covering QoS levels, retain flags, and topic aliases.

### `models/unity_config.json`
MQTT topic definitions shared with the Unity client. Unity reads this file to know which topics to subscribe to and publish on — keep this in sync with the Python engine's topic constants.

---

## 7. MQTT Topics Reference

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

## 8. Scenarios Reference

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

## 9. Step-by-Step Setup & Running

Follow these steps in order. Do not skip steps — each one is a prerequisite for the next.

### Prerequisites

- Python 3.8 or higher installed
- Internet connection (to reach HiveMQ public broker), or a local Mosquitto broker running
- The `.pkl` model files (if you want live AI inference rather than POC mode)

---

### Step 1 — Clone or download the repository

```bash
git clone https://github.com/Y2842002/Authorized-Projects.git
cd Authorized-Projects/ABM-Adaptive-Belt-Monitoring-main
```

### Step 2 — Create and activate a virtual environment (recommended)

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# macOS / Linux
python3 -m venv venv
source venv/bin/activate
```

> Using a virtual environment prevents version conflicts with other Python projects on your machine.

### Step 3 — Install Python dependencies

```bash
pip install -r requirements.txt
```

| Package | Version | Purpose |
|---|---|---|
| `numpy` | 1.24.3 | Numerical array operations for feature processing |
| `scikit-learn` | 1.3.0 | Loading and running `.pkl` model files |
| `paho-mqtt` | 1.6.1 | MQTT client library for broker communication |

### Step 4 — Place model files

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

### Step 5 — Review configuration

Open `config.json` and confirm:
- `"broker"` is set to `"broker.hivemq.com"` (public) or `"localhost"` (local Mosquitto)
- `"port"` is `1883`
- `"mode"` is `"POC"` for predefined scenarios

### Step 6 — Run the inference engine

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

### Step 7 — Test without Unity (MQTT client)

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

### Step 8 — Connect Unity dashboard

Open the Unity project, configure it with the topic settings from `models/unity_config.json`, and run the scene. Unity will subscribe to `unity/motor/status` and publish commands to `unity/motor/command` automatically.

---

## 10. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `Connection refused` on startup | Broker unreachable | Check internet connection; try switching to a local Mosquitto broker |
| Engine receives command but gives no response | Machine is OFF | Publish to `parallax/MACHINE_ON` first |
| `Unknown scenario` error | Typo in scenario key | Use `get_all_scenarios()` or refer to section 8 for exact keys |
| `ModuleNotFoundError: paho` | Dependencies not installed | Run `pip install -r requirements.txt` |
| Model files not loading | Files missing or misnamed | Check exact filenames match those listed in section 9, Step 4 |
| No output on `unity/motor/status` | Wrong topic subscription | Confirm you are subscribing to `unity/motor/status` not a variant |

---

## Project File Structure

```
ABM-Adaptive-Belt-Monitoring-main/
├── main.py                        ← Entry point: MQTT engine + MQTTInferenceEngine
├── config.json                    ← MQTT broker settings + system config
├── requirements.txt               ← Python dependencies
│
├── inference/
│   ├── __init__.py
│   └── scenario_handler.py        ← Scenario registry + model loading + inference routing
│
├── mqtt/
│   ├── __init__.py
│   └── mqtt_client.py             ← Legacy MQTT client (reference only — not active)
│
├── models/
│   └── unity_config.json          ← MQTT topic definitions shared with Unity
│   (+ *.pkl files — not in repo, must be added manually)
│
├── config/
│   └── mqtt_config.json           ← Supplementary MQTT settings
│
├── docs/
│   ├── System_Architecture.md
│   ├── MQTT_Integration_Guide.md
│   └── QUICK_START.md
│
└── demo/
    └── ABM Demo Video.mp4         ← Live inference demonstration recording
```

---

*ABM Documentation — April 2026 | Youssef Osama*
