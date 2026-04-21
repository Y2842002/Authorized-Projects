# Authorized Projects — Full Documentation

> **Author:** Youssef Osama  
> **Repository:** `Y2842002/Authorized-Projects`  
> **Last Updated:** April 2026

---

## Table of Contents

1. [Repository Overview](#1-repository-overview)
2. [Project 1 — ABM: Adaptive Belt Monitoring](#2-project-1--abm-adaptive-belt-monitoring)
   - [Overview](#21-overview)
   - [System Architecture](#22-system-architecture)
   - [AI Models](#23-ai-models)
   - [Module Reference](#24-module-reference)
   - [Configuration](#25-configuration)
   - [MQTT Topics](#26-mqtt-topics)
   - [Scenarios](#27-scenarios)
   - [Setup & Running](#28-setup--running)
3. [Project 2 — CubeSat Flight Software](#3-project-2--cubesat-flight-software)
   - [Overview](#31-overview)
   - [Hardware & Sensors](#32-hardware--sensors)
   - [Code Structure](#33-code-structure)
   - [Data Output Format](#34-data-output-format)
   - [Setup & Flashing](#35-setup--flashing)
4. [Project 3 — PWM Drawer](#4-project-3--pwm-drawer)
   - [Overview](#41-overview)
   - [Hardware Architecture (AVR Layered Design)](#42-hardware-architecture-avr-layered-design)
   - [Module Reference](#43-module-reference)
   - [PWM Signal Modes](#44-pwm-signal-modes)
   - [State Machine: Signal Reading](#45-state-machine-signal-reading)
   - [Setup & Build](#46-setup--build)
5. [Repository Structure](#5-repository-structure)

---

## 1. Repository Overview

This repository collects three independent embedded systems and AI engineering projects, each demonstrating different aspects of hardware-software integration:

| Project | Domain | Language/Platform | Key Technology |
|---|---|---|---|
| ABM – Adaptive Belt Monitoring | Industrial AI / IoT | Python + Unity | MQTT, scikit-learn, Digital Twin |
| CubeSat Flight Software | Space / Embedded | Arduino (ESP32) | Multi-sensor telemetry, GPS |
| PWM Drawer | Embedded Systems | C (AVR/ATmega) | PWM generation & measurement, LCD |

---

## 2. Project 1 — ABM: Adaptive Belt Monitoring

### 2.1 Overview

ABM is an AI-powered digital twin system designed for industrial belt-driven machinery. It monitors belt tension stability and motor health in real time using machine learning inference, and streams results to a Unity 3D dashboard over MQTT.

**The core problem it solves:** Industrial belt systems fail silently — loose or overtight belts and degrading motors are hard to detect until a breakdown occurs. ABM provides continuous early-warning diagnostics.

**Team:**
| Name | Role |
|---|---|
| Sara Hassan Mohamed | AI Engineer |
| Dalia Abdelmonem | AI Engineer |
| Mohamed Magdy | AI Engineer |
| Ramy Elhosary | Integration Engineer |
| Ahmed Mostafa | Unity Developer |
| Youssef Osama | Automation Engineer |

**Dataset:** Fraunhofer Institute open dataset — https://fordatis.fraunhofer.de/handle/fordatis/347

---

### 2.2 System Architecture

```
┌──────────────────────────────┐
│       Unity Dashboard        │  ← HMI / Visualization layer
│  (Belt tension, stability,   │
│   motor health display)      │
└────────────┬─────────────────┘
             │ MQTT (bidirectional)
             ▼
┌──────────────────────────────┐
│      MQTT Broker             │  ← HiveMQ public broker
│   broker.hivemq.com:1883     │     (or local Mosquitto)
└────────────┬─────────────────┘
             │
             ▼
┌──────────────────────────────────────────────────────┐
│              Python Inference Engine                  │
│                                                       │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────┐  │
│  │ MQTT Client  │ → │  Scenario    │ → │  AI      │  │
│  │  (main.py)   │   │  Handler     │   │  Models  │  │
│  └──────────────┘   └──────────────┘   └──────────┘  │
│                              │                        │
│                              ▼                        │
│                     ┌──────────────┐                  │
│                     │ JSON Result  │ → published back │
│                     └──────────────┘   to Unity       │
└──────────────────────────────────────────────────────┘
```

**Processing latency breakdown:**

| Step | Typical Time |
|---|---|
| MQTT transmission | < 10 ms |
| Data validation | < 1 ms |
| Feature preprocessing | < 5 ms |
| Model inference | 20–50 ms |
| JSON serialisation | < 5 ms |
| **Total round-trip** | **< 100 ms** |

---

### 2.3 AI Models

The system uses two scikit-learn models:

#### Model 1 — Binary Stability Classifier
- **File:** `stability_energy_model.pkl`
- **Output:** `Stable` / `Unstable`
- **Purpose:** Detect operational instability from energy-based vibration metrics
- **Input features:** Vibration intensity, dominant frequency, spectral entropy
- **Additional component:** `se_computer (1).pkl` — computes the energy score used for motor health drift detection

#### Model 2 — Multi-Class Tension Classifier
- **Files:** `feature_extractor(3) (1).pkl`, `feature_scaler(3) (1).pkl`, `tension_classifier(3) (1).pkl`
- **Output:** `Loose` / `Optimal` / `Tight`
- **Purpose:** Identify current belt tension condition
- **Pipeline:** Raw features → Feature extractor → Scaler → Classifier

> **Note:** Model `.pkl` files are not included in the repository due to file size. They must be placed in the `models/` directory before running.

---

### 2.4 Module Reference

#### `main.py` — Entry Point & MQTT Engine

The main orchestrator. Instantiates `MQTTInferenceEngine`, connects to the broker, and enters the MQTT event loop.

**Class: `MQTTInferenceEngine`**

| Method | Description |
|---|---|
| `__init__()` | Creates MQTT client, initialises `ScenarioHandler`, sets up callbacks |
| `on_connect()` | Fires on broker connection; subscribes to command, machine-on, and machine-off topics; publishes initial `"Ready"` state |
| `on_disconnect()` | Logs unexpected disconnections |
| `on_message()` | Routes incoming messages to the correct handler based on topic |
| `handle_machine_on()` | Sets `machine_on = True`, publishes `ON/Ready` state |
| `handle_machine_off()` | Sets `machine_on = False`, publishes `OFF/Stopped` state |
| `handle_unity_command()` | Parses `RUN_SCENARIO` commands from Unity; rejects if machine is OFF |
| `publish_state()` | Publishes a status JSON with `machine_state`, `tension`, `stability`, `health` |
| `publish_result()` | Publishes a full scenario result JSON including `scenario` and `approach` fields |
| `connect()` | Connects to the broker using settings from `config.json` |
| `start()` | Calls `connect()` then enters `loop_forever()` |

**Published JSON structure (status topic):**
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

#### `inference/scenario_handler.py` — Scenario Logic

Handles the mapping from scenario names to predefined inference results. On initialisation, loads all `.pkl` model files from the `models/` directory.

**Class: `ScenarioHandler`**

| Method | Description |
|---|---|
| `__init__()` | Loads models; registers all Approach 1 and Approach 2 scenario dictionaries |
| `load_models()` | Iterates over expected model filenames and loads each with `pickle.load()` |
| `process_scenario(name)` | Looks up scenario by name; returns a result dict; optionally calls `run_model_inference()` if models are loaded |
| `run_model_inference(name)` | Placeholder for live model inference — returns a note and list of loaded model keys |
| `get_error_result(name)` | Returns a structured error dict with a hint of the first 5 valid scenario names |
| `get_all_scenarios()` | Returns the full list of scenario keys |

**Approach 1 — Stability-Health Scenarios:**

| Scenario Key | Tension | Stability | Health |
|---|---|---|---|
| `loose_unstable_good` | Loose | Unstable | Good |
| `optimal_stable_good` | Optimal | Stable | Good |
| `optimal_unstable_warning` | Optimal | Unstable | Warning |
| `tight_stable_critical` | Tight | Stable | Critical |

**Approach 2 — Adjustment Scenarios:**

| Scenario Key | Tension | Health Recommendation |
|---|---|---|
| `adjustment_loose` | Loose | Requires Tightening |
| `adjustment_optimal` | Optimal | No Adjustment Needed |
| `adjustment_tight` | Tight | Requires Loosening |

---

#### `mqtt/mqtt_client.py` — Legacy MQTT Client

An alternative MQTT client (used in earlier iterations). Accepts sensor data directly from Unity, runs `predictor.predict_all()`, and returns predictions.

> This module is retained for reference but the current system uses `MQTTInferenceEngine` in `main.py` instead.

---

### 2.5 Configuration

**`config.json`** — Main configuration file:

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

**`config/mqtt_config.json`** — Supplementary MQTT settings (QoS, retain flags, topic aliases).

**`models/unity_config.json`** — MQTT topic definitions shared with the Unity client side.

---

### 2.6 MQTT Topics

| Topic | Direction | Purpose |
|---|---|---|
| `unity/motor/command` | Unity → Python | Send `RUN_SCENARIO` commands |
| `unity/motor/status` | Python → Unity | Publish inference results |
| `parallax/MACHINE_ON` | Hardware → Python | Signal machine power-on event |
| `parallax/MACHINE_OFF` | Hardware → Python | Signal machine power-off event |

---

### 2.7 Scenarios

To trigger a scenario from Unity, publish to `unity/motor/command`:

```json
{
  "command": "RUN_SCENARIO",
  "scenario": "optimal_stable_good"
}
```

The engine will respond on `unity/motor/status` with the full result. If the machine is currently `OFF`, the command is rejected and a `"Machine must be ON"` response is sent instead.

---

### 2.8 Setup & Running

**Requirements:** Python 3.8+

**Install dependencies:**
```bash
pip install -r requirements.txt
```

**Dependencies:**
```
numpy==1.24.3
scikit-learn==1.3.0
paho-mqtt==1.6.1
```

**Place model files** in the `models/` directory (not included in repo):
- `stability_energy_model.pkl`
- `se_computer (1).pkl`
- `feature_extractor(3) (1).pkl`
- `feature_scaler(3) (1).pkl`
- `tension_classifier(3) (1).pkl`

**Run the engine:**
```bash
python main.py
```

The engine will connect to the HiveMQ public broker and begin listening. On successful connection you should see:
```
✅ Connected to MQTT Broker!
📥 Subscribed to: unity/motor/command
```

> The system runs in POC mode by default — scenario results are returned from predefined mappings rather than live sensor inference.

---

## 3. Project 2 — CubeSat Flight Software

### 3.1 Overview

A basic flight software implementation for a CubeSat running on an **ESP32** microcontroller. The system collects data from six onboard sensors and streams telemetry over the serial port every 2 seconds. It is designed as a foundation for CubeSat subsystem integration and educational demonstration.

---

### 3.2 Hardware & Sensors

| Sensor | Model | Interface | GPIO Pins | Measurement |
|---|---|---|---|---|
| Temperature & Humidity | DHT22 | Digital | GPIO 15 | Temp (°C), Humidity (%) |
| Barometric Pressure | BMP280 | Hardware SPI | GPIO 5 (CS) | Pressure (hPa), Altitude (m) |
| IMU (Accel + Gyro) | MPU-6050 | I²C | SDA=GPIO21, SCL=GPIO22 | Acceleration (X/Y/Z), Gyro (X/Y/Z) |
| GPS | Generic NMEA | UART (Serial2) | RX=GPIO16, TX=GPIO17 | Latitude, Longitude, Altitude, Date/Time |
| Light Intensity | TSL2561 | I²C | 0x39 | Lux |
| UV Radiation | ML8511 | Analogue | GPIO 34 (data), GPIO 14 (enable) | UV intensity (mW/cm²), DUV index |

**Library dependencies:**
```
Adafruit_BMP280
DHT sensor library (DHT22 / AM2302)
TinyGPSPlus
Adafruit_TSL2561_Unified
ML8511
Wire (I²C, built-in)
```

All library ZIPs are included in the `HAL/` and `MCAL/` folders for offline installation.

---

### 3.3 Code Structure

**`setup()`** — Initialises serial (115,200 baud) and calls individual `initialize<Sensor>()` functions for all six sensors. Prints free heap memory on startup.

**`loop()`** — Runs two tasks:
1. Every 2,000 ms: calls `readAndDisplaySensors()` to read and print all sensor data.
2. Continuously: feeds GPS NMEA bytes from `Serial2` into the `TinyGPSPlus` parser.

**`readAndDisplaySensors()`** — Reads all sensors sequentially and prints a single compact line to Serial:
```
DHT:<hum>%,<temp>°C|BMP:<pressure>hPa,<alt>m@MPU:-x:<ax>,y:<ay>,z:<az>|gx:<gx>,gy:<gy>,gz:<gz>&LIGHT:<lux>lux,UV:<uvI>mW/cm² (DUV: <duv>),GPS:<lat>,<lng>,<alt>m | Date: MM/DD/YYYY HH:MM:SS
```

**`readMPU6050()`** — Directly reads 14 bytes from the MPU-6050 via I²C starting at register `0x3B` (ACCEL_XOUT_H). Skips the temperature registers (bytes 6–7) and reads the 6 gyroscope bytes.

**Individual `initialize<X>()` functions** — Each sensor has its own init function with a blocking `while(1)` halt if the sensor is not found (prevents running with missing hardware).

---

### 3.4 Data Output Format

Serial output example (115,200 baud):
```
DHT:55.30%,23.10°C|BMP:1013.25hPa,12.50m@MPU:-x:320,y:-180,z:16200|gx:45,gy:-22,gz:8&LIGHT:312.50lux,UV:0.82mW/cm² (DUV: 0.03),GPS:30.123456,31.654321,45.2m | Date: 4/21/2026 10:00:00
```

If GPS has no fix, the GPS field reads: `GPS:NoFix`

If the light sensor is overloaded: `LIGHT:OVF,`

---

### 3.5 Setup & Flashing

**Platform:** ESP32 (tested with ESP32 DevKit)

**IDE:** Arduino IDE or PlatformIO

**Steps:**
1. Open Arduino IDE and install the ESP32 board package.
2. Install the library ZIPs from `HAL/` and `MCAL/` via **Sketch → Include Library → Add .ZIP Library**.
3. Open `main.ino`.
4. Select your ESP32 board and COM port.
5. Upload and open Serial Monitor at **115,200 baud**.

**Wiring notes:**
- BMP280 uses **hardware SPI** (not I²C) — CS on GPIO5.
- MPU-6050 uses **I²C** on GPIO 21/22 (ESP32 default I²C pins).
- GPS uses **UART2** on GPIO 16 (RX) / 17 (TX) at 9,600 baud.
- ML8511 UV sensor uses **ADC on GPIO 34** — ensure 3.3V ADC calibration (`setVoltsPerStep(3.3, 4095)` matches your ESP32's 12-bit ADC).

---

## 4. Project 3 — PWM Drawer

### 4.1 Overview

A PWM signal generation and measurement tool implemented in C for the **ATmega32 (AVR)** microcontroller. The system generates PWM signals at different frequencies and duty cycles (controlled by 4 physical switches), measures incoming PWM signals using hardware interrupts and Timer0, and displays the computed results on a 16×2 LCD.

Authored by: **Yousef Osama Mohamed** — created May 2024, built with Microchip Studio (Atmel Studio).

---

### 4.2 Hardware Architecture (AVR Layered Design)

The project follows the standard AVR embedded driver architecture with three layers:

```
┌─────────────────────────────────────────┐
│          Application (main.c)           │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│           HAL (Hardware Abstraction)    │
│  LCD  |  Keypad (KPD)  |  EEPROM        │
│  Switch (SWITCH)                        │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      MCAL (Microcontroller Abstraction) │
│  DIO  |  EXTI  |  PWM  |  TIMER0        │
│  TWI  |  Global Interrupt (GI)          │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│         UTIL (Utility Layer)            │
│  STD_TYPES.h  |  BIT_MATH.h            │
└─────────────────────────────────────────┘
```

---

### 4.3 Module Reference

#### UTIL Layer

**`STD_TYPES.h`** — Custom type aliases for portability:

| Alias | Base Type | Size |
|---|---|---|
| `u8` | `unsigned char` | 8-bit unsigned |
| `s8` | `signed char` | 8-bit signed |
| `u16` | `unsigned short int` | 16-bit unsigned |
| `s16` | `signed short int` | 16-bit signed |
| `u32` | `unsigned long int` | 32-bit unsigned |
| `s32` | `signed long int` | 32-bit signed |
| `f32` | `float` | 32-bit float |
| `f64` | `double` | 64-bit float |
| `BOOL` | `enum {false, true}` | Boolean |

**`BIT_MATH.h`** — Macros for direct bit manipulation (SET_BIT, CLR_BIT, TOG_BIT, GET_BIT).

---

#### MCAL Layer

**`DIO`** — Digital I/O driver. Configures port pins as input or output and reads/writes them.

**`EXTI`** — External interrupt driver. Configures INT0 (on PD2) for rising or falling edge detection. Used to timestamp PWM signal edges.

**`TIMER0`** — 8-bit timer driver. Runs continuously to count elapsed time between PWM edges.

**`PWM`** — PWM generation and calculation module:

| Function | Description |
|---|---|
| `PWM_voidInitChannel_1A()` | Initialises Timer1 Channel A in Fast PWM mode |
| `PWM_voidGenerate_PWM_Channel_1A(freq, duty)` | Generates PWM at the given frequency (Hz) and duty cycle (%) |
| `PWM_voidDutyCycleCalculations(...)` | Computes duty cycle (%) from captured timer ticks |
| `PWM_voidFrequencyCalculation(...)` | Computes signal frequency (Hz) from total period ticks |
| `PWM_voidPeriodicTimeCalculations(...)` | Converts frequency to period in microseconds |
| `PWM_voidOnTimeDuration(...)` | Calculates T_on (µs) from period and duty cycle |
| `PWM_voidOffTimeDuration(...)` | Calculates T_off (µs) from period and T_on |

**`GI`** — Global interrupt enable/disable (sets/clears the AVR `I` bit in SREG).

**`TWI`** — I²C (Two-Wire Interface) driver for inter-device communication.

---

#### HAL Layer

**`LCD`** — 16×2 character LCD driver (4-bit mode). Key functions:
- `LCD_voidInit()` — Initialises the LCD in 4-bit mode
- `LCD_voidDisplayString(str)` — Writes a string to the current cursor position
- `LCD_voidGoToSpecificPosition(row, col)` — Moves cursor to a given position
- `LCD_voidDisplayPWMCalculations(freq, period, duty, Ton, Toff)` — Displays computed PWM values on the LCD
- `LCD_voidDisplayPWMSignal(duty, freq, Ton, Toff)` — Draws a visual waveform representation

**`KPD`** — 4×4 matrix keypad driver. Scans rows/columns to detect key presses.

**`SWITCH`** — Push-button switch driver. Reads switch state with support for forward-connection (active-high) wiring.

**`EEPROM`** — Reads/writes to the ATmega32 internal EEPROM for persistent data storage.

---

### 4.4 PWM Signal Modes

The four switches on PORTC select different PWM presets for generation:

| Switch | Pin | Frequency | Duty Cycle | Mode |
|---|---|---|---|---|
| SW1 | PC0 | 25 Hz | 75% | Low frequency, high duty |
| SW2 | PC1 | 50 Hz | 15% | Mid frequency, low duty |
| SW3 | PC2 | 50 Hz | 85% | Mid frequency, high duty |
| SW4 | PC3 | 165 Hz | 95% | High frequency, very high duty |
| None | — | 25 Hz | 10% | Default (low frequency, low duty) |

Simultaneously, the system listens on PD2 (INT0) for an external PWM input and measures it.

---

### 4.5 State Machine: Signal Reading

The measurement logic uses a 3-state interrupt-driven state machine on INT0 and Timer0:

```
State 0 (Idle)
  → Rising edge detected on PD2
  → Reset TCNT0 and overflow counter
  → Switch INT0 to FALLING edge
  → Move to State 1

State 1 (Measuring ON time)
  → Falling edge detected
  → Capture TCNT0 and overflow count as "ON ticks"
  → Switch INT0 to RISING edge
  → Move to State 2

State 2 (Measuring total period)
  → Next rising edge detected
  → Capture TCNT0 and overflow count as "total ticks"
  → Move to State 3

State 3 (Calculate & Display)
  → Main loop detects State == 3
  → Calls PWM calculation functions
  → Displays results on LCD
  → Resets back to State 0
```

**Global variables used by the ISR:**

| Variable | Purpose |
|---|---|
| `Global_state` | Current state (0–3) |
| `Global_ovfCounter` | Number of Timer0 overflows since last reset |
| `Global_onTicks` | Timer ticks at falling edge (end of ON period) |
| `Global_onCounter` | Overflow count at falling edge |
| `Global_totalTicks` | Timer ticks at second rising edge (end of full period) |
| `Global_totalCounter` | Overflow count at second rising edge |

**ISR vectors used:**
- `__vector_11` — Timer0 overflow ISR: increments `Global_ovfCounter`
- `__vector_1` — INT0 (External Interrupt 0) ISR: state transitions

---

### 4.6 Setup & Build

**Target MCU:** ATmega32 (16 MHz crystal — `F_CPU = 16000000UL`)

**IDE:** Microchip Studio (Atmel Studio 7)

**Build:** Open `PWM_drawer.atsln` in Microchip Studio and build the project. Pre-built binaries are available in `Debug/`:
- `PWM_drawer.hex` — Flash to microcontroller via AVR programmer (AVRDUDE / USBasp)
- `PWM_drawer.elf` — ELF file for debugging

**Flash with AVRDUDE (example):**
```bash
avrdude -c usbasp -p m32 -U flash:w:PWM_drawer.hex
```

**Hardware connections:**
- LCD: Connected to PORTB (data) and PORTD (control pins) — see `LCD_config.h` for exact mapping
- Switches: PC0–PC3 (forward-connected, active-high)
- PWM Output: PD5 (Timer1 OC1A)
- PWM Input (measurement): PD2 (INT0)

---

## 5. Repository Structure

```
Authorized-Projects-main/
│
├── README.md                          ← Top-level project overview
│
├── ABM-Adaptive-Belt-Monitoring-main/
│   ├── main.py                        ← Entry point: MQTT engine
│   ├── config.json                    ← MQTT & system configuration
│   ├── requirements.txt               ← Python dependencies
│   ├── inference/
│   │   ├── __init__.py
│   │   └── scenario_handler.py        ← Scenario logic & model loading
│   ├── mqtt/
│   │   ├── __init__.py
│   │   └── mqtt_client.py             ← Legacy MQTT client (reference)
│   ├── models/
│   │   └── unity_config.json          ← Unity-side MQTT topic config
│   ├── config/
│   │   └── mqtt_config.json           ← Supplementary MQTT settings
│   ├── docs/
│   │   ├── System_Architecture.md     ← Architecture diagrams & design
│   │   ├── MQTT_Integration_Guide.md  ← Unity integration guide
│   │   └── QUICK_START.md             ← Step-by-step run instructions
│   └── demo/
│       └── ABM Demo Video.mp4         ← Live inference demo
│
├── CubeSat Flight Software/
│   ├── main.ino                       ← ESP32 Arduino sketch
│   ├── HAL/                           ← Sensor library ZIPs
│   │   ├── Adafruit_TSL2561-1.1.3.zip
│   │   ├── DHT_sensor_library-1.4.7.zip
│   │   ├── ML8511-0.2.1.zip
│   │   └── TinyGPSPlus-1.0.3.zip
│   └── MCAL/                          ← Display library ZIPs
│       ├── Adafruit_GFX_Library-1.12.6.zip
│       ├── Adafruit_SSD1306-2.5.16.zip
│       └── EspSoftwareSerial-8.1.0.zip
│
└── PWM_drawer-main/
    ├── PWM_DESIGN.DSN                 ← Proteus circuit schematic
    ├── PWM_DESIGN.PWI                 ← Proteus workspace settings
    └── PWM_drawer/
        ├── main.c                     ← Application entry point
        ├── UTIL/
        │   ├── STD_TYPES.h            ← Custom type definitions
        │   └── BIT_MATH.h             ← Bit manipulation macros
        ├── HAL/
        │   ├── LCD/                   ← LCD driver (inc/ + src/)
        │   ├── KPD/                   ← Keypad driver (inc/ + src/)
        │   ├── SWITCH/                ← Switch driver (inc/ + src/)
        │   └── EEPROM/                ← EEPROM driver (inc/ + src/)
        ├── MCAL/
        │   ├── DIO/                   ← Digital I/O driver
        │   ├── PWM/                   ← PWM generation & measurement
        │   ├── TIMER0/                ← Timer0 driver
        │   ├── EXTI/                  ← External interrupt driver
        │   ├── TWI/                   ← I²C driver
        │   └── Global_Interrupt_Enable/ ← GI enable/disable
        └── Debug/                     ← Pre-built binaries (.hex, .elf)
```

---

*Documentation generated from source code analysis — April 2026*
