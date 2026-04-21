# ESP32 Smart Relay Controller — Documentation

> **Platform:** ESP32 (MicroPython)
> **Broker:** HiveMQ public MQTT broker
> **Simulation:** Wokwi-compatible

---

## Table of Contents

1. [What This Project Does](#1-what-this-project-does)
2. [How It Works — End-to-End Flow](#2-how-it-works--end-to-end-flow)
3. [Hardware Requirements & Wiring](#3-hardware-requirements--wiring)
4. [Software Requirements](#4-software-requirements)
5. [Configuration Reference](#5-configuration-reference)
6. [MQTT Topics & Commands](#6-mqtt-topics--commands)
7. [Control Modes](#7-control-modes)
8. [Step-by-Step Setup Guide](#8-step-by-step-setup-guide)
9. [Understanding the Code](#9-understanding-the-code)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. What This Project Does

This firmware turns an ESP32 into a **Wi-Fi-connected smart relay controller** with real-time temperature monitoring. It:

- Reads temperature from an **NTC thermistor** (10 kΩ) connected to the ADC
- Publishes temperature readings to an **MQTT broker** whenever the value changes meaningfully
- Subscribes to an MQTT topic to receive **remote commands** from any MQTT client (phone app, dashboard, Node-RED, etc.)
- Controls a **relay** in one of two modes:
  - **Manual** — you turn the relay ON/OFF by sending MQTT commands
  - **Automatic** — the relay turns ON when temperature exceeds a set threshold (default: 60 °C)

**Typical use case:** Monitoring a motor, heater, or industrial belt drive — automatically cutting power if it overheats, while also allowing manual override from a remote dashboard.

---

## 2. How It Works — End-to-End Flow

Understanding the flow before looking at code makes everything clearer.

```
 ┌─────────────────────────────────────────────────────────┐
 │                    BOOT / POWER-ON                      │
 └──────────────────────────┬──────────────────────────────┘
                            │
                            ▼
 ┌─────────────────────────────────────────────────────────┐
 │  1. Hardware Initialisation                             │
 │     • relay  = GPIO 21 (output, starts OFF)             │
 │     • adc    = GPIO 33 (12-bit ADC input)               │
 │     • Timer0 starts → calls temp_sample() every 50 ms  │
 └──────────────────────────┬──────────────────────────────┘
                            │
                            ▼
 ┌─────────────────────────────────────────────────────────┐
 │  2. Wi-Fi Connection                                    │
 │     • Connects to WIFI_SSID                             │
 │     • Blocks until connected                            │
 │     • Prints IP address                                 │
 └──────────────────────────┬──────────────────────────────┘
                            │
                            ▼
 ┌─────────────────────────────────────────────────────────┐
 │  3. MQTT Connection                                     │
 │     • Connects to broker.hivemq.com                     │
 │     • Retries automatically on failure                  │
 │     • Registers on_message() as the callback            │
 │     • Subscribes to "mode" topic                        │
 └──────────────────────────┬──────────────────────────────┘
                            │
                            ▼
 ┌─────────────────────────────────────────────────────────┐
 │  4. Main Loop (runs forever)                            │
 │                                                         │
 │   ┌─────────────────────────────────────────────────┐   │
 │   │  Every 50 ms (Timer ISR — background)           │   │
 │   │    Read ADC → compute temperature               │   │
 │   │    Store in current_temp (global)               │   │
 │   └─────────────────────────────────────────────────┘   │
 │                                                         │
 │   ┌─────────────────────────────────────────────────┐   │
 │   │  Every loop iteration (~100 ms)                 │   │
 │   │                                                 │   │
 │   │  Step A — Publish temperature?                  │   │
 │   │    If |current_temp - last_published| >= 0.5°C  │   │
 │   │    → publish to "readings" topic                │   │
 │   │                                                 │   │
 │   │  Step B — Automatic relay control?              │   │
 │   │    If mode == "automatic":                      │   │
 │   │      current_temp > 60°C  → relay ON            │   │
 │   │      current_temp <= 60°C → relay OFF           │   │
 │   │                                                 │   │
 │   │  Step C — Check for incoming messages           │   │
 │   │    client.check_msg()                           │   │
 │   │    → triggers on_message() if message waiting   │   │
 │   │                                                 │   │
 │   │  Step D — Connection dropped?                   │   │
 │   │    OSError caught → reconnect + re-subscribe    │   │
 │   └─────────────────────────────────────────────────┘   │
 └─────────────────────────────────────────────────────────┘
```

### How temperature is calculated

The thermistor is wired as a **voltage divider** with a fixed resistor. The ADC reads the divided voltage, which changes with the thermistor's resistance, which changes with temperature.

The Steinhart–Hart simplified (B-parameter) equation converts the ADC reading to °C:

```
ratio  = ADC_MAX / adc_value - 1          (R/R₀ from the voltage divider)
T (K)  = 1 / ( ln(ratio) / B + 1/T₀ )
T (°C) = T(K) - 273.15
```

Where:
- `B = 3950` — the Beta constant of the NTC thermistor
- `T₀ = 298.15 K` — reference temperature (25 °C)
- `ADC_MAX = 4095` — maximum 12-bit ADC value

---

## 3. Hardware Requirements & Wiring

### Components

| Component | Specification | Notes |
|---|---|---|
| Microcontroller | ESP32 DevKit (or compatible) | Must run MicroPython |
| Thermistor | NTC 10 kΩ, B = 3950 | Standard temperature sensing thermistor |
| Fixed resistor | 10 kΩ | Forms voltage divider with the thermistor |
| Relay module | 5V relay with opto-isolator | The opto-isolator protects the ESP32 from relay transients |
| Power supply | 3.3V / 5V depending on relay module | |

### Wiring Diagram

```
3.3V ──┬────────────────────────────────────
       │
      [R_fixed = 10kΩ]
       │
       ├──────────────────────────── GPIO 33 (ADC input)
       │
      [NTC Thermistor]
       │
GND ───┴────────────────────────────────────
```

```
GPIO 21 ──── IN pin of relay module
3.3V    ──── VCC of relay module  (or 5V depending on your module)
GND     ──── GND of relay module
```

> **Important:** The thermistor and fixed resistor form a voltage divider. As temperature rises, the NTC thermistor's resistance drops, which raises the voltage at GPIO 33. Make sure the fixed resistor has the same nominal resistance as the thermistor (10 kΩ) for best sensitivity at mid-range temperatures.

---

## 4. Software Requirements

### MicroPython firmware

Your ESP32 must be flashed with **MicroPython firmware**. Download from: https://micropython.org/download/esp32/

Flash with `esptool`:

```bash
# Erase existing flash
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash

# Flash MicroPython
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 esp32-YYYYMMDD-vX.X.X.bin
```

### Required MicroPython libraries

| Library | How to get it |
|---|---|
| `umqtt.simple` | Built into the standard MicroPython ESP32 firmware |
| `network` | Built-in |
| `machine` | Built-in |
| `math` | Built-in |

No additional library installation is needed if you use the standard ESP32 MicroPython firmware.

### Tools for uploading code

| Tool | Platform | Download |
|---|---|---|
| **Thonny IDE** | Windows / macOS / Linux | https://thonny.org |
| **mpremote** | Command line | `pip install mpremote` |
| **ampy** | Command line | `pip install adafruit-ampy` |
| **Wokwi** | Browser (simulation) | https://wokwi.com |

---

## 5. Configuration Reference

All configurable values are defined as **named constants at the top of `main.py`**. You should never need to edit anything below the constants section.

### Wi-Fi settings

```python
WIFI_SSID       = "Wokwi-GUEST"   # Your Wi-Fi network name
WIFI_PASSWORD   = ""               # Your Wi-Fi password (empty = open network)
```

### MQTT settings

```python
MQTT_BROKER     = "broker.hivemq.com"   # Broker hostname
MQTT_CLIENT_ID  = "esp32_client"        # Must be unique per device on the broker
TOPIC_PUB       = "readings"            # Topic where temperature is published
TOPIC_SUB       = "mode"                # Topic where commands are received
```

> **Multiple devices:** If you run more than one device on the same broker, each must have a **unique `MQTT_CLIENT_ID`**. Using the same ID on two devices causes them to disconnect each other repeatedly.

### Thermistor settings

```python
THERMISTOR_B    = 3950       # Beta constant — must match your thermistor's datasheet
THERMISTOR_T0   = 298.15     # Reference temperature in Kelvin (= 25 °C)
ADC_MAX         = 4095       # Do not change — fixed by 12-bit ADC resolution
```

> If you use a different NTC thermistor, update `THERMISTOR_B` with the value from its datasheet. Common B values: 3380, 3435, 3950, 4050.

### Threshold and sensitivity settings

```python
TEMP_THRESHOLD      = 60.0   # °C — relay turns ON above this in automatic mode
ADC_DELTA           = 0.1    # Minimum ADC change to trigger temperature recalculation
TEMP_PUBLISH_DELTA  = 0.5    # Minimum °C change to publish a new MQTT reading
```

Increasing `TEMP_PUBLISH_DELTA` reduces MQTT traffic. Decreasing it gives more granular data but publishes more frequently.

### Timing settings

```python
TIMER_PERIOD_MS     = 50     # How often the ADC is sampled (milliseconds)
MAIN_LOOP_DELAY_S   = 0.1    # Pause between main loop iterations (seconds)
MQTT_RETRY_DELAY_S  = 2      # Wait between MQTT reconnect attempts (seconds)
```

---

## 6. MQTT Topics & Commands

### Topics

| Topic | Direction | Content | Example |
|---|---|---|---|
| `readings` | ESP32 → Broker | Temperature string in °C | `"23.47"` |
| `mode` | Broker → ESP32 | Command string (see below) | `"automatic"` |

### Commands (publish to `mode`)

| Command | Effect | Mode Required |
|---|---|---|
| `manual` | Switch to manual relay control | Any |
| `automatic` | Switch to automatic temperature control | Any |
| `led_on` | Turn relay ON immediately | Manual only |
| `led_off` | Turn relay OFF immediately | Manual only |

> Commands are **case-insensitive** and **whitespace-tolerant** — `"  LED_ON  "`, `"Led_On"`, and `"led_on"` all work the same way.

> `led_on` and `led_off` commands are **silently ignored** (not rejected) when the mode is `automatic`. This prevents accidental override of automatic safety control.

### Testing with mosquitto CLI

```bash
# Listen for temperature readings
mosquitto_sub -h broker.hivemq.com -t readings

# Switch to automatic mode
mosquitto_pub -h broker.hivemq.com -t mode -m "automatic"

# Switch to manual mode and turn relay on
mosquitto_pub -h broker.hivemq.com -t mode -m "manual"
mosquitto_pub -h broker.hivemq.com -t mode -m "led_on"
```

---

## 7. Control Modes

### Manual Mode (default)

The relay is fully under remote command. The temperature is still published, but has no effect on the relay.

```
MQTT command "led_on"  → relay.value(1) → load powered
MQTT command "led_off" → relay.value(0) → load off
```

### Automatic Mode

The relay is controlled entirely by the measured temperature. Manual `led_on` / `led_off` commands are ignored.

```
current_temp > TEMP_THRESHOLD  → relay.value(1) → load powered (cooling/protection)
current_temp ≤ TEMP_THRESHOLD  → relay.value(0) → load off
```

**Switching between modes:**

```
Send "manual"    → mode = "manual"    (current relay state is preserved)
Send "automatic" → mode = "automatic" (relay immediately reflects temperature)
```

---

## 8. Step-by-Step Setup Guide

Follow these steps in order.

---

### Step 1 — Wire the hardware

Connect components according to the wiring diagram in section 3:
- Thermistor voltage divider → GPIO 33
- Relay module IN → GPIO 21
- Relay module VCC/GND → appropriate power rail

### Step 2 — Flash MicroPython to the ESP32

If your ESP32 does not already have MicroPython installed:

```bash
pip install esptool

# Erase
esptool.py --chip esp32 --port COM3 erase_flash        # Windows: COM3, Linux/macOS: /dev/ttyUSB0

# Flash
esptool.py --chip esp32 --port COM3 --baud 460800 write_flash -z 0x1000 esp32-firmware.bin
```

Verify by connecting in Thonny — you should see a `>>>` MicroPython REPL prompt.

### Step 3 — Edit configuration constants

Open `main.py` and update the constants section at the top:

```python
WIFI_SSID       = "YOUR_NETWORK_NAME"
WIFI_PASSWORD   = "YOUR_WIFI_PASSWORD"
MQTT_CLIENT_ID  = "esp32_bedroom_relay"  # unique name for this device
TEMP_THRESHOLD  = 60.0                   # adjust to your use case
```

### Step 4 — Upload `main.py` to the ESP32

**Option A — Thonny IDE:**
1. Open Thonny → connect to ESP32 (bottom-right corner: select MicroPython ESP32)
2. Open `main.py`
3. Go to **File → Save as...** → choose **MicroPython device** → save as `main.py`
4. The file is now on the device and will run automatically on boot

**Option B — mpremote (command line):**
```bash
mpremote connect COM3 cp main.py :main.py
```

**Option C — ampy:**
```bash
ampy --port COM3 put main.py
```

### Step 5 — Run and verify

**In Thonny:** Press the green **Run** button or press `F5`.

**Expected serial output on successful startup:**
```
[Wi-Fi] Connecting...
[Wi-Fi] Connected: ('192.168.1.45', '255.255.255.0', '192.168.1.1', '8.8.8.8')
[MQTT] Connected to broker.hivemq.com
[MQTT] Subscribed to 'mode'
[Publish #1] 23.47 °C
[Publish #2] 24.01 °C
```

### Step 6 — Send a test command

Open a second terminal (or use MQTT Explorer / MQTTX app):

```bash
# Test manual relay control
mosquitto_pub -h broker.hivemq.com -t mode -m "manual"
mosquitto_pub -h broker.hivemq.com -t mode -m "led_on"
```

**Expected serial output:**
```
[MQTT] Received on 'mode': 'manual'
[Mode] Switched to MANUAL
[MQTT] Received on 'mode': 'led_on'
[Relay] ON  (manual command)
```

### Step 7 — Test automatic mode

```bash
mosquitto_pub -h broker.hivemq.com -t mode -m "automatic"
```

The relay will now respond to the live temperature. Heat the thermistor with your fingers to test — you should see the relay state change as temperature crosses `TEMP_THRESHOLD`.

---

## 9. Understanding the Code

### Why a Timer ISR for temperature sampling?

The ADC sampling runs inside a hardware timer interrupt (`Timer(0)`) rather than in the main loop. This ensures temperature is always sampled at a consistent 50 ms interval **regardless of how busy the main loop is** (MQTT operations, reconnection delays, etc.).

The ISR stores the result in the global `current_temp`. The main loop reads that value whenever it needs it — the sampling and the publishing are fully decoupled.

### Why the ADC_DELTA check?

ADC readings have natural noise — even with no temperature change, consecutive reads may differ by ±1–2 counts. Without `ADC_DELTA`, the thermistor formula would run on every sample (every 50 ms) even when nothing is happening. The delta check prevents unnecessary floating-point computation in the ISR.

### Why TEMP_PUBLISH_DELTA for publishing?

Similarly, temperature computed from a noisy ADC will fluctuate by ±0.1–0.3 °C even in a stable environment. Without `TEMP_PUBLISH_DELTA`, the MQTT broker would receive hundreds of nearly-identical messages per minute. The 0.5 °C threshold ensures only meaningful changes are published.

### Why check_msg() instead of a blocking subscribe?

`client.check_msg()` is **non-blocking** — it checks if a message has arrived and returns immediately whether or not one has. This lets the main loop keep sampling temperature and controlling the relay without stalling. The alternative (`client.wait_msg()`) blocks until a message arrives, which would freeze the whole loop.

### MQTT reconnection logic

The `try/except OSError` block in `pub_and_sub()` catches network failures. When the broker disconnects or the network drops temporarily, instead of crashing the device, the code:
1. Calls `mqtt_connect()` to re-establish the connection (with automatic retries)
2. Re-registers the callback (`set_callback`)
3. Re-subscribes to the topic

This means the device **recovers from dropped connections without requiring a reboot**.

---

## 10. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| Stuck on `[Wi-Fi] Connecting...` | Wrong SSID/password, or network out of range | Double-check `WIFI_SSID` and `WIFI_PASSWORD`; move closer to router |
| `[MQTT] Connection failed` repeated | Broker unreachable, no internet | Confirm Wi-Fi connected first; try pinging `broker.hivemq.com` from another device |
| Temperature reads as `inf` or `nan` | Thermistor disconnected or open circuit | Check thermistor wiring; `adc_value` of 0 causes division by zero in formula |
| Temperature reads as very large negative number | Voltage divider wired backwards | Swap thermistor and fixed resistor positions |
| `led_on` command has no effect | Device is in automatic mode | Send `"manual"` first, then `"led_on"` |
| Temperature published too frequently | `TEMP_PUBLISH_DELTA` is too small | Increase to `1.0` or higher to reduce publish rate |
| Temperature never published | `TEMP_PUBLISH_DELTA` is too large | Decrease to `0.1`; or the temperature may genuinely be stable |
| Device keeps disconnecting from MQTT | Duplicate `MQTT_CLIENT_ID` on the broker | Give each device a unique `MQTT_CLIENT_ID` |
| `ImportError: no module named 'umqtt'` | MicroPython build missing umqtt | Flash a full standard ESP32 MicroPython firmware (not a minimal build) |
| Relay clicks rapidly / chatters | Temperature oscillating around threshold | Add hysteresis: turn ON at 62 °C, turn OFF at 58 °C (requires code change) |
| Nothing happens after upload | `main.py` not saved to device root | In Thonny, confirm save target is **MicroPython device**, not local computer |

---

*Documentation — April 2026*
