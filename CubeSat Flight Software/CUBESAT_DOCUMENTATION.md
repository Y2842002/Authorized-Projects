# CubeSat Flight Software
## Project Documentation

> **Author:** Youssef Osama
> **Repository:** `Y2842002/Authorized-Projects` → `CubeSat Flight Software/`
> **Platform:** ESP32 (Arduino)
> **Last Updated:** April 2026

---

## Table of Contents

1. [What Is the CubeSat Flight Software?](#1-what-is-the-cubesat-flight-software)
2. [How It Works — End-to-End Flow](#2-how-it-works--end-to-end-flow)
3. [Hardware & Sensors Reference](#3-hardware--sensors-reference)
4. [Code Structure](#4-code-structure)
5. [Serial Data Output Format](#5-serial-data-output-format)
6. [Step-by-Step Setup & Flashing](#6-step-by-step-setup--flashing)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. What Is the CubeSat Flight Software?

This is a **basic flight software implementation** for a CubeSat (small satellite) form-factor platform, running on an **ESP32 microcontroller**. The system collects environmental, inertial, positional, and radiation data from six onboard sensors and streams it as formatted telemetry over USB serial at a fixed 2-second interval.

**Purpose:** Serve as a functional, modular foundation for CubeSat subsystem integration and educational demonstration. Each sensor is independently initialised, read, and reported — making it straightforward to add, remove, or replace sensors.

**Key design characteristics:**

- **No RTOS** — runs on the Arduino bare-metal loop (`setup()` / `loop()`)
- **Blocking init** — if a sensor fails to initialise, the system halts (`while(1)`), preventing corrupted data from being logged silently
- **Compact serial format** — all sensor data is packed onto a single line per cycle for easy parsing or logging

---

## 2. How It Works — End-to-End Flow

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

      │  (if any sensor init fails → while(1) halt with error message)
      ▼
loop()  [runs indefinitely]
  ├─ Every 2,000 ms:
  │    └─ readAndDisplaySensors()
  │         ├─ Read DHT22   → temperature (°C), humidity (%)
  │         ├─ Read BMP280  → pressure (hPa), altitude (m)
  │         ├─ readMPU6050()→ accel X/Y/Z (raw int16), gyro X/Y/Z (raw int16)
  │         ├─ Read TSL2561 → light intensity (lux)
  │         ├─ Read ML8511  → UV intensity (mW/cm²), DUV index
  │         ├─ Read GPS     → lat, lon, alt, date, time
  │         └─ Print one compact line to Serial
  │
  └─ Continuously (every loop iteration):
       └─ Feed GPS NMEA bytes from Serial2 → TinyGPSPlus parser
```

> **Why the GPS runs every iteration (not just every 2 seconds):** GPS NMEA sentences arrive as a continuous byte stream on UART2. The `TinyGPSPlus.encode()` call must consume these bytes as they arrive to avoid buffer overflow. If it only ran every 2 seconds, bytes would pile up in the UART buffer and sentences would be missed or corrupted.

**Sensor initialisation behaviour:**

Each `initialize<X>()` function follows this exact pattern:

```
Call sensor.begin() or sensor.init()
  ├─ Success → print "Sensor X initialised OK"  → continue
  └─ Failure → print "Sensor X NOT FOUND"       → while(1) halt
```

This **blocking halt on failure** is intentional: it prevents the system from logging incomplete or misleading telemetry data and makes hardware wiring errors immediately obvious during development.

---

## 3. Hardware & Sensors Reference

### Sensor Pin Map

| Sensor | Model | Interface | GPIO Pins | Output |
|---|---|---|---|---|
| Temperature & Humidity | DHT22 | Digital 1-wire | GPIO 15 | Temp (°C), Humidity (%) |
| Barometric Pressure | BMP280 | Hardware SPI | GPIO 5 (CS), default MOSI/MISO/SCK | Pressure (hPa), Altitude (m) |
| IMU | MPU-6050 | I²C | SDA = GPIO 21, SCL = GPIO 22 | Accel X/Y/Z, Gyro X/Y/Z (raw int16) |
| GPS | Generic NMEA GPS | UART (Serial2) | RX = GPIO 16, TX = GPIO 17 | Lat, Lon, Alt, Date/Time |
| Light Intensity | TSL2561 | I²C | I²C address 0x39 | Lux (float) |
| UV Radiation | ML8511 | Analogue ADC | GPIO 34 (data), GPIO 14 (enable) | UV intensity (mW/cm²), DUV index |

### Critical Wiring Notes

- **BMP280** uses **hardware SPI** — do **not** wire it to I²C pins. Its CS pin must go to GPIO 5.
- **MPU-6050** uses **I²C** on GPIO 21 (SDA) and GPIO 22 (SCL) — these are the ESP32's default I²C bus. Both sensors sharing I²C (MPU-6050 and TSL2561) must have different addresses.
- **GPS module** operates at **9,600 baud** on UART2 — RX of the ESP32 (GPIO 16) connects to TX of the GPS module, and vice versa.
- **ML8511 UV sensor** uses the **12-bit ADC** on GPIO 34. GPIO 14 is the enable pin. The `setVoltsPerStep(3.3, 4095)` call in firmware must match your ESP32's actual ADC reference voltage.

### Library Dependencies

| Library | Sensor | Source in Repository |
|---|---|---|
| `Adafruit_BMP280` | BMP280 pressure | Via Library Manager |
| `DHT sensor library` | DHT22 temperature/humidity | `HAL/DHT_sensor_library-1.4.7.zip` |
| `TinyGPSPlus` | GPS NMEA parsing | `HAL/TinyGPSPlus-1.0.3.zip` |
| `Adafruit_TSL2561_Unified` | TSL2561 light sensor | `HAL/Adafruit_TSL2561-1.1.3.zip` |
| `ML8511` | UV sensor | `HAL/ML8511-0.2.1.zip` |
| `Wire` (I²C) | MPU-6050, TSL2561 | Arduino built-in |
| `Adafruit_GFX_Library` | OLED display support | `MCAL/Adafruit_GFX_Library-1.12.6.zip` |
| `Adafruit_SSD1306` | OLED display driver | `MCAL/Adafruit_SSD1306-2.5.16.zip` |
| `EspSoftwareSerial` | Software UART (if needed) | `MCAL/EspSoftwareSerial-8.1.0.zip` |

All library `.zip` files are pre-bundled in the repository's `HAL/` and `MCAL/` directories for **fully offline installation** — no internet connection required to install dependencies.

---

## 4. Code Structure

The entire application lives in a single file: `main.ino`. There are no separate `.cpp` / `.h` source files.

### `setup()` — Initialisation Phase

Runs once at boot. Performs the following in strict order:

1. Opens USB serial at 115,200 baud and waits for the port to become ready.
2. Calls each `initialize<Sensor>()` function sequentially — DHT22, BMP280, MPU-6050, TSL2561, ML8511, GPS.
3. Prints available heap memory as a startup health check.

### `loop()` — Runtime Phase

Runs continuously after `setup()`. Has two parallel responsibilities:

| Task | Frequency | Description |
|---|---|---|
| `readAndDisplaySensors()` | Every 2,000 ms | Reads all six sensors; prints a single formatted telemetry line to Serial |
| GPS feed | Every loop iteration | Feeds incoming UART2 bytes to `TinyGPSPlus.encode()` |

### `readAndDisplaySensors()` — Telemetry Output

Reads sensors sequentially in this order: DHT22 → BMP280 → MPU-6050 → TSL2561 → ML8511 → GPS. Then concatenates all readings into one compact line and prints it to Serial.

### `readMPU6050()` — Direct I²C Register Read

The MPU-6050 does not use a standard Adafruit driver. Instead, the sketch reads **14 raw bytes** directly from the sensor's I²C register bank, starting at register `0x3B` (`ACCEL_XOUT_H`):

```
Bytes 0–1:   Accel X  (int16, big-endian)
Bytes 2–3:   Accel Y
Bytes 4–5:   Accel Z
Bytes 6–7:   Temperature register (skipped — not used)
Bytes 8–9:   Gyro X
Bytes 10–11: Gyro Y
Bytes 12–13: Gyro Z
```

The raw 16-bit signed integers are printed directly. No scaling to m/s² or °/s is applied in the current firmware.

### Individual `initialize<X>()` functions

Each sensor has its own dedicated init function. They all follow the same pattern: attempt to start the sensor, halt with an error message if it cannot be found, print success if it can.

---

## 5. Serial Data Output Format

Serial output is printed at **115,200 baud**, one line per 2-second cycle.

### Format

```
DHT:<humidity>%,<temp>°C|BMP:<pressure>hPa,<alt>m@MPU:-x:<ax>,y:<ay>,z:<az>|gx:<gx>,gy:<gy>,gz:<gz>&LIGHT:<lux>lux,UV:<uvIntensity>mW/cm² (DUV: <duvIndex>),GPS:<lat>,<lng>,<alt>m | Date: MM/DD/YYYY HH:MM:SS
```

### Example Output Line

```
DHT:55.30%,23.10°C|BMP:1013.25hPa,12.50m@MPU:-x:320,y:-180,z:16200|gx:45,gy:-22,gz:8&LIGHT:312.50lux,UV:0.82mW/cm² (DUV: 0.03),GPS:30.123456,31.654321,45.2m | Date: 4/21/2026 10:00:00
```

### Field Delimiters

| Delimiter | Separates |
|---|---|
| `\|` | DHT22 from BMP280; MPU-6050 from LIGHT |
| `@` | BMP280 from MPU-6050 |
| `&` | MPU-6050 from LIGHT |
| `,` | Values within the same sensor block |

### Special Output Cases

| Condition | What Appears in Output |
|---|---|
| GPS has no satellite fix | `GPS:NoFix` |
| Light sensor saturated (too bright) | `LIGHT:OVF,` |

---

## 6. Step-by-Step Setup & Flashing

Follow these steps in order. Do not skip any step.

### Prerequisites

- Arduino IDE 2.x **or** PlatformIO installed
- ESP32 board package installed in Arduino IDE (see Step 1)
- USB cable connected to the ESP32 DevKit
- All sensors wired correctly per the pin map in section 3

---

### Step 1 — Install the ESP32 board package

1. Open Arduino IDE.
2. Go to **File → Preferences**.
3. In the "Additional Board Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**.
5. Search for `esp32` and install the package by **Espressif Systems**.

### Step 2 — Install sensor libraries from the bundled ZIP files

All required libraries are already included in the repository. No internet connection needed.

1. In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library...**
2. Navigate to `CubeSat Flight Software/HAL/` and install each ZIP one by one:
   - `DHT_sensor_library-1.4.7.zip`
   - `ML8511-0.2.1.zip`
   - `TinyGPSPlus-1.0.3.zip`
   - `Adafruit_TSL2561-1.1.3.zip`
3. Navigate to `CubeSat Flight Software/MCAL/` and install each ZIP:
   - `Adafruit_GFX_Library-1.12.6.zip`
   - `Adafruit_SSD1306-2.5.16.zip`
   - `EspSoftwareSerial-8.1.0.zip`
4. Install `Adafruit_BMP280` via the Library Manager:
   **Sketch → Include Library → Manage Libraries** → search `BMP280` → install the Adafruit version.

### Step 3 — Wire the hardware

Connect each sensor to the ESP32 according to the pin map in section 3. Refer to the critical wiring notes, especially:
- BMP280 → hardware SPI, CS = GPIO 5
- MPU-6050 → I²C, SDA = GPIO 21, SCL = GPIO 22
- GPS → UART2, RX = GPIO 16, TX = GPIO 17

### Step 4 — Open the sketch

In Arduino IDE: **File → Open** → navigate to `CubeSat Flight Software/main.ino` and open it.

### Step 5 — Select board and COM port

1. **Tools → Board → ESP32 Arduino** → select `ESP32 Dev Module` (or your exact board variant).
2. **Tools → Port** → select the COM port your ESP32 is connected to.
   - Windows: appears as `COM3`, `COM4`, etc.
   - Linux / macOS: appears as `/dev/ttyUSB0` or `/dev/tty.usbserial-*`

### Step 6 — Upload the firmware

Click the **Upload** button (→ arrow icon) or press `Ctrl+U`.

Wait for the IDE output to show:
```
Leaving...
Hard resetting via RTS pin...
```

This confirms the firmware was written and the ESP32 has restarted.

### Step 7 — Open Serial Monitor and verify telemetry

1. Go to **Tools → Serial Monitor** (or press `Ctrl+Shift+M`).
2. Set the baud rate dropdown to **115,200**.
3. The ESP32 will reboot and you should see:

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

A new telemetry line should appear every 2 seconds. If any sensor fails to initialise, the system halts and prints which sensor was not found.

---

## 7. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| System halts immediately after boot with "NOT FOUND" | One or more sensors not wired correctly | Check wiring for the sensor named in the error; verify power supply |
| `GPS:NoFix` in every line | GPS has no satellite lock | Move to a location with clear sky view; wait up to 90 seconds for cold fix |
| `LIGHT:OVF,` in output | TSL2561 saturation | Normal when facing a bright light source directly; move sensor or reduce exposure |
| Garbled / unreadable serial output | Wrong baud rate in Serial Monitor | Set Serial Monitor baud rate to exactly **115,200** |
| Upload fails with port error | Wrong COM port selected | Check Device Manager (Windows) or `ls /dev/tty*` (Linux/macOS) |
| MPU-6050 reads all zeros | I²C wiring issue or address conflict | Confirm SDA on GPIO 21, SCL on GPIO 22; verify no address clash with TSL2561 |
| UV readings seem incorrect or always zero | ADC voltage mismatch | Verify `setVoltsPerStep(3.3, 4095)` matches your board; confirm GPIO 14 enable is wired |
| BMP280 not found | Wired to I²C instead of SPI | BMP280 must use hardware SPI with CS on GPIO 5, not I²C pins |
| Sketch won't compile | Missing library | Confirm all ZIPs from `HAL/` and `MCAL/` are installed; check Adafruit_BMP280 via Library Manager |

---

## Project File Structure

```
CubeSat Flight Software/
├── main.ino                           ← Complete Arduino sketch (all logic in one file)
│
├── HAL/                               ← Sensor library ZIPs (offline installation)
│   ├── Adafruit_TSL2561-1.1.3.zip     ← Light intensity sensor
│   ├── DHT_sensor_library-1.4.7.zip   ← Temperature/humidity sensor
│   ├── ML8511-0.2.1.zip               ← UV radiation sensor
│   └── TinyGPSPlus-1.0.3.zip          ← GPS NMEA parser
│
└── MCAL/                              ← Display library ZIPs
    ├── Adafruit_GFX_Library-1.12.6.zip← Core GFX primitives
    ├── Adafruit_SSD1306-2.5.16.zip    ← OLED display driver
    └── EspSoftwareSerial-8.1.0.zip    ← Software UART
```

---

*CubeSat Flight Software Documentation — April 2026 | Youssef Osama*
