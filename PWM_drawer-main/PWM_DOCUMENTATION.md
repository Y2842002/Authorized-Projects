# PWM Drawer
## Project Documentation

> **Author:** Yousef Osama Mohamed
> **Repository:** `Y2842002/Authorized-Projects` → `PWM_drawer-main/`
> **Platform:** ATmega32 (AVR) — C firmware
> **Created:** May 2024 | **Last Updated:** April 2026

---

## Table of Contents

1. [What Is PWM Drawer?](#1-what-is-pwm-drawer)
2. [How It Works — End-to-End Flow](#2-how-it-works--end-to-end-flow)
3. [Hardware Architecture (AVR Layered Design)](#3-hardware-architecture-avr-layered-design)
4. [Module Reference](#4-module-reference)
5. [PWM Signal Modes](#5-pwm-signal-modes)
6. [State Machine: Signal Measurement](#6-state-machine-signal-measurement)
7. [Step-by-Step Setup & Build](#7-step-by-step-setup--build)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. What Is PWM Drawer?

PWM Drawer is a **bare-metal C firmware** for the **ATmega32 AVR microcontroller** that performs two simultaneous operations:

1. **Generates** PWM signals at selectable frequencies and duty cycles, controlled by 4 physical switches on PORTC.
2. **Measures** an incoming external PWM signal using hardware interrupts and a timer, then **calculates and displays** the signal's characteristics — frequency, period, duty cycle, T_on, and T_off — on a 16×2 LCD.

The firmware is written entirely in C using a strict layered AVR driver architecture (UTIL → MCAL → HAL → Application), without any Arduino libraries. Everything from pin toggling to LCD communication is implemented from scratch.

**What makes this project notable:**
- Pure register-level C — no Arduino, no HAL wrappers
- Interrupt-driven measurement using a 3-state machine on INT0 and Timer0
- Generation and measurement run on the same MCU simultaneously and independently

---

## 2. How It Works — End-to-End Flow

The system runs two fully independent data paths on a single microcontroller:

```
╔══════════════════════════════════════════════════════════╗
║              PATH A — PWM GENERATION                     ║
║                                                          ║
║  main.c reads PORTC switches (PC0–PC3)                   ║
║       │                                                  ║
║       ▼                                                  ║
║  Selects a (frequency, duty cycle) preset                ║
║       │                                                  ║
║       ▼                                                  ║
║  PWM_voidGenerate_PWM_Channel_1A(freq, duty)             ║
║       │                                                  ║
║       ▼                                                  ║
║  Timer1 in Fast PWM mode drives OC1A (PD5)              ║
║       │                                                  ║
║       ▼                                                  ║
║  PWM signal output on PD5 pin                           ║
╚══════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════╗
║              PATH B — PWM MEASUREMENT                    ║
║                                                          ║
║  External PWM signal arrives on PD2 (INT0)               ║
║       │                                                  ║
║       ▼                                                  ║
║  INT0 ISR fires on each edge transition                  ║
║  Timer0 counts elapsed ticks between edges              ║
║       │                                                  ║
║  State machine advances through States 0 → 1 → 2 → 3   ║
║    State 0: Wait for rising edge (idle)                  ║
║    State 1: Measure ON time (wait for falling edge)      ║
║    State 2: Measure full period (wait for rising edge)   ║
║    State 3: Signal main loop that data is ready          ║
║       │                                                  ║
║       ▼                                                  ║
║  main loop detects Global_state == 3                     ║
║  Calls calculation functions:                            ║
║    ├─ PWM_voidDutyCycleCalculations()                    ║
║    ├─ PWM_voidFrequencyCalculation()                     ║
║    ├─ PWM_voidPeriodicTimeCalculations()                 ║
║    ├─ PWM_voidOnTimeDuration()                           ║
║    └─ PWM_voidOffTimeDuration()                          ║
║       │                                                  ║
║       ▼                                                  ║
║  LCD_voidDisplayPWMCalculations()                        ║
║  LCD_voidDisplayPWMSignal()                              ║
║       │                                                  ║
║       ▼                                                  ║
║  Results displayed on 16×2 LCD                          ║
║  State resets to 0 → cycle repeats                      ║
╚══════════════════════════════════════════════════════════╝
```

---

## 3. Hardware Architecture (AVR Layered Design)

The project uses a strict four-layer architecture. Each layer depends only on layers below it — upper layers call downward, never upward.

```
┌──────────────────────────────────────────────────────┐
│                  Application Layer                   │
│                      main.c                          │
│                                                      │
│  • Reads PORTC switches each iteration               │
│  • Selects PWM generation preset                     │
│  • Polls Global_state for measurement completion     │
│  • Calls LCD display functions                       │
└────────────────────────┬─────────────────────────────┘
                         │ calls ↓
┌────────────────────────▼─────────────────────────────┐
│           HAL — Hardware Abstraction Layer           │
│                                                      │
│  LCD     — 16×2 character display, 4-bit mode        │
│  KPD     — 4×4 matrix keypad scanner                 │
│  SWITCH  — Push-button switch driver                 │
│  EEPROM  — ATmega32 internal EEPROM read/write       │
└────────────────────────┬─────────────────────────────┘
                         │ calls ↓
┌────────────────────────▼─────────────────────────────┐
│      MCAL — Microcontroller Abstraction Layer        │
│                                                      │
│  DIO    — Digital I/O (pin direction, read, write)   │
│  EXTI   — External interrupt (INT0 on PD2)           │
│  PWM    — PWM generation + measurement math          │
│  TIMER0 — 8-bit timer for tick counting              │
│  TWI    — I²C (Two-Wire Interface) driver            │
│  GI     — Global interrupt enable/disable            │
└────────────────────────┬─────────────────────────────┘
                         │ calls ↓
┌────────────────────────▼─────────────────────────────┐
│               UTIL — Utility Layer                   │
│                                                      │
│  STD_TYPES.h  — Portable type aliases (u8, f32, ...) │
│  BIT_MATH.h   — Bit manipulation macros              │
└──────────────────────────────────────────────────────┘
```

---

## 4. Module Reference

### UTIL Layer

#### `STD_TYPES.h` — Custom Type Aliases

Ensures consistent bit widths across AVR compilers and prevents the ambiguity of using `int` or `long` directly:

| Alias | Base Type | Width | Common Use |
|---|---|---|---|
| `u8` | `unsigned char` | 8-bit unsigned | Register values, byte data |
| `s8` | `signed char` | 8-bit signed | Small signed values |
| `u16` | `unsigned short int` | 16-bit unsigned | Timer register contents |
| `s16` | `signed short int` | 16-bit signed | |
| `u32` | `unsigned long int` | 32-bit unsigned | Tick accumulation across overflows |
| `s32` | `signed long int` | 32-bit signed | |
| `f32` | `float` | 32-bit float | Frequency, duty cycle, time results |
| `f64` | `double` | 64-bit float | High-precision intermediate calculations |
| `BOOL` | `enum {false, true}` | 1 logical bit | State flags |

#### `BIT_MATH.h` — Bit Manipulation Macros

Direct register-level bit operations — essential for AVR peripheral configuration without touching other bits:

| Macro | Operation | Example |
|---|---|---|
| `SET_BIT(reg, bit)` | Force a bit to 1 | `SET_BIT(PORTB, 3)` |
| `CLR_BIT(reg, bit)` | Force a bit to 0 | `CLR_BIT(PORTB, 3)` |
| `TOG_BIT(reg, bit)` | Flip a bit | `TOG_BIT(PORTB, 3)` |
| `GET_BIT(reg, bit)` | Read a bit's current value (0 or 1) | `if (GET_BIT(PINC, 0))` |

---

### MCAL Layer

#### `DIO` — Digital I/O Driver
Provides functions to configure ATmega32 port pins as input or output, and to read or write their state. Used as the foundation by all HAL drivers and the application.

#### `EXTI` — External Interrupt Driver
Configures **INT0 on PD2** for either rising or falling edge detection. This is the entry point for PWM measurement — every edge on PD2 fires the INT0 ISR, which advances the measurement state machine.

#### `TIMER0` — 8-bit Timer Driver
Timer0 runs continuously from startup. Its overflow ISR increments the `Global_ovfCounter` variable. Together with `TCNT0` (the live 8-bit count register), the overflow counter provides a high-resolution elapsed-tick count for measuring signal timing far beyond the 8-bit range.

**Why both TCNT0 and overflow counter?** An 8-bit timer can only count 0–255. For low-frequency signals, the ON time or period may span many full timer overflows. The combined value `(overflows × 256) + TCNT0` gives a much larger effective range.

#### `PWM` — PWM Generation and Measurement Math

| Function | Purpose |
|---|---|
| `PWM_voidInitChannel_1A()` | Initialises Timer1 Channel A in **Fast PWM mode** |
| `PWM_voidGenerate_PWM_Channel_1A(freq, duty)` | Generates a PWM signal at the specified frequency (Hz) and duty cycle (%) on **OC1A (PD5)** |
| `PWM_voidDutyCycleCalculations(...)` | Computes duty cycle (%) from captured ON ticks vs total period ticks |
| `PWM_voidFrequencyCalculation(...)` | Computes signal frequency (Hz) from total period ticks |
| `PWM_voidPeriodicTimeCalculations(...)` | Converts frequency to period in microseconds |
| `PWM_voidOnTimeDuration(...)` | Calculates T_on (µs) from period and duty cycle |
| `PWM_voidOffTimeDuration(...)` | Calculates T_off (µs) from period and T_on |

#### `GI` — Global Interrupt Enable/Disable
Sets or clears the `I` (global interrupt enable) bit in the AVR `SREG` register. Must be called before any ISR-driven feature (EXTI edges, Timer0 overflows) can fire.

#### `TWI` — I²C Driver
Two-Wire Interface driver included in the driver set for extensibility. Not used in the core PWM measurement path but available for attaching additional I²C peripherals.

---

### HAL Layer

#### `LCD` — 16×2 Character LCD Driver (4-bit mode)

Operates the LCD in 4-bit parallel mode, which requires only 4 data pins instead of 8, saving GPIO pins on the ATmega32. Pin mapping is defined in `LCD_config.h` — data pins use PORTB, control pins (RS, E) use PORTD.

| Function | Purpose |
|---|---|
| `LCD_voidInit()` | Initialises the LCD in 4-bit mode; clears the display |
| `LCD_voidDisplayString(str)` | Writes a null-terminated string at the current cursor position |
| `LCD_voidGoToSpecificPosition(row, col)` | Moves cursor to a specific row (0–1) and column (0–15) |
| `LCD_voidDisplayPWMCalculations(freq, period, duty, Ton, Toff)` | Displays five computed PWM values across both LCD rows |
| `LCD_voidDisplayPWMSignal(duty, freq, Ton, Toff)` | Draws a visual ASCII waveform representation of the measured signal |

#### `KPD` — 4×4 Matrix Keypad Driver
Column-scanning driver to detect key presses on a 4×4 matrix keypad. Not used in the main PWM measurement loop — available for future feature extensions.

#### `SWITCH` — Push-Button Switch Driver
Reads the state of push-button switches. Supports **forward-connection (active-high)** wiring — the pin reads HIGH when the switch is pressed.

#### `EEPROM` — Internal EEPROM Driver
Reads from and writes to the ATmega32's internal 512-byte EEPROM. Useful for persisting configuration or calibration data across power cycles.

---

## 5. PWM Signal Modes

The four switches on PORTC select PWM generation presets. Switch priority is SW1 > SW2 > SW3 > SW4. If no switch is pressed, the default preset is applied.

| Switch | Pin | Frequency | Duty Cycle | Characteristic |
|---|---|---|---|---|
| SW1 | PC0 | 25 Hz | 75% | Low frequency, high duty — long ON pulses |
| SW2 | PC1 | 50 Hz | 15% | Mid frequency, low duty — short ON pulses |
| SW3 | PC2 | 50 Hz | 85% | Mid frequency, very high duty — mostly ON |
| SW4 | PC3 | 165 Hz | 95% | High frequency, nearly always ON |
| *(none pressed)* | — | 25 Hz | 10% | Default — low frequency, very short ON pulses |

The generated PWM signal exits on **PD5 (Timer1 OC1A)**. Simultaneously, the system listens on **PD2 (INT0)** for an incoming external PWM signal to measure. These two paths are completely independent of each other.

---

## 6. State Machine: Signal Measurement

The PWM measurement logic is implemented as a **3-state interrupt-driven state machine** running on INT0 and Timer0. This is the most important part of the firmware to understand.

### State Diagram

```
         ┌────────────────────────────────────────────────────┐
         │                    reset to State 0                │
         │                                                    │
         ▼                                                    │
  ┌────────────┐                                              │
  │  STATE 0   │  Rising edge on PD2                         │
  │   (Idle)   │──────────────────────────────────────────▶  │
  └────────────┘  • Reset TCNT0 to 0                          │
                  • Reset Global_ovfCounter to 0              │
                  • Configure INT0 to trigger on FALLING edge  │
                  • Global_state = 1                           │
                                                              │
  ┌────────────┐  Falling edge on PD2                         │
  │  STATE 1   │  • Capture TCNT0 → Global_onTicks            │
  │ (ON timing)│  • Capture ovfCounter → Global_onCounter     │
  └─────┬──────┘  • Configure INT0 to trigger on RISING edge  │
        │         • Global_state = 2                           │
        │                                                      │
        │         Next rising edge on PD2                      │
  ┌─────▼──────┐  • Capture TCNT0 → Global_totalTicks         │
  │  STATE 2   │  • Capture ovfCounter → Global_totalCounter  │
  │  (Period)  │  • Global_state = 3                           │
  └─────┬──────┘                                              │
        │                                                      │
  ┌─────▼──────┐  main loop polls: if (Global_state == 3)     │
  │  STATE 3   │  • Compute duty, freq, period, Ton, Toff     │
  │ (Calculate)│  • Display results on LCD                    │
  └─────┬──────┘  • Global_state = 0                          │
        │                                                      │
        └──────────────────────────────────────────────────────┘
```

### Global Variables Used by the ISR

All of these are declared `volatile` in the source to prevent the compiler from caching them in CPU registers, since they are written by the ISR and read by the main loop.

| Variable | Type | Purpose |
|---|---|---|
| `Global_state` | `u8` | Current state (0–3); the main loop polls this |
| `Global_ovfCounter` | `u32` | Number of Timer0 overflows since last reset |
| `Global_onTicks` | `u8` | TCNT0 value captured at the falling edge |
| `Global_onCounter` | `u32` | Overflow count captured at the falling edge |
| `Global_totalTicks` | `u8` | TCNT0 value captured at the second rising edge |
| `Global_totalCounter` | `u32` | Overflow count captured at the second rising edge |

### ISR Vectors

| Vector | Triggered By | Action |
|---|---|---|
| `__vector_11` | Timer0 overflow | Increments `Global_ovfCounter` |
| `__vector_1` | INT0 edge on PD2 | Executes state transition; captures tick values |

### How Tick Values Convert to Time

The total number of ticks for the ON period:
```
ON_ticks_total = (Global_onCounter × 256) + Global_onTicks
```

The total number of ticks for the full period:
```
period_ticks_total = (Global_totalCounter × 256) + Global_totalTicks
```

These totals are passed to the `PWM_void*` calculation functions, which convert them to frequency, period (µs), duty cycle (%), T_on (µs), and T_off (µs) based on the known timer clock frequency.

---

## 7. Step-by-Step Setup & Build

### Prerequisites

- **Target MCU:** ATmega32 on a development board, or Proteus simulation (schematic included)
- **16 MHz crystal** oscillator connected to the ATmega32 (firmware assumes `F_CPU = 16000000UL`)
- **Microchip Studio (Atmel Studio 7)** installed on Windows — required to build from source
- **AVR programmer** (USBasp, AVRISP mkII, or similar) + AVRDUDE installed — for flashing to real hardware
- 16×2 LCD wired to PORTB (data) and PORTD (RS, E control) per `LCD_config.h`
- Switches wired to PC0–PC3 (active-high / forward-connected)

---

### Step 1 — Open the project in Microchip Studio

1. Launch **Microchip Studio (Atmel Studio 7)**.
2. Go to **File → Open → Project/Solution**.
3. Navigate to `PWM_drawer-main/PWM_drawer/` and open `PWM_drawer.atsln`.

The project tree in Solution Explorer should show all source files across `main.c`, `MCAL/`, `HAL/`, and `UTIL/`.

### Step 2 — Verify MCU and clock settings

1. Right-click the project in Solution Explorer → **Properties**.
2. Confirm **Device** is set to `ATmega32`.
3. Confirm `F_CPU` is defined as `16000000UL` (either in `main.c` or in the project's preprocessor symbol list).

### Step 3 — Build the project

Press **F7** or go to **Build → Build Solution**.

A successful build shows:
```
Build succeeded.
   0 Error(s)
   0 Warning(s)
```

If you want to skip building, pre-built binaries are already available in `Debug/`:
- `PWM_drawer.hex` — ready to flash to the MCU
- `PWM_drawer.elf` — ELF file for JTAG debugging

### Step 4 — Flash to the ATmega32

Connect your AVR programmer and run AVRDUDE:

```bash
avrdude -c usbasp -p m32 -U flash:w:PWM_drawer.hex
```

| AVRDUDE Flag | Meaning |
|---|---|
| `-c usbasp` | Programmer type — change to `avrispmkii`, `stk500v2`, etc. as needed |
| `-p m32` | Target device: ATmega32 |
| `-U flash:w:PWM_drawer.hex` | Write the hex file to the microcontroller's flash memory |

**Expected AVRDUDE output:**

```
avrdude: AVR device initialized and ready to accept instructions
avrdude: Device signature = 0x1e9502 (probably m32)
avrdude: writing flash (XXXX bytes):
Writing | ################################################## | 100%
avrdude: verifying flash memory against PWM_drawer.hex:
avrdude done.  Thank you.
```

### Step 5 — Simulate in Proteus (no hardware required)

If you don't have physical hardware:

1. Open **Proteus** and load `PWM_DESIGN.DSN` from the project root (`PWM_drawer-main/`).
2. Right-click the ATmega32 component → **Properties** → set the **Program File** to the path of `PWM_drawer.hex`.
3. Click **Run** (Play button). The LCD, switches, and PWM measurement input are pre-wired in the schematic.

### Step 6 — Verify hardware operation

After flashing and powering on:

1. **Press SW1 (PC0)** — LCD should display: frequency = 25 Hz, duty cycle = 75%
2. **Press SW2–SW4** — LCD should update to match the corresponding preset from the table in section 5
3. **Release all switches** — default preset (25 Hz, 10%) is applied
4. **Apply an external PWM signal to PD2** — after one complete period is captured, the LCD should update with the measured frequency, period, duty cycle, T_on, and T_off values

---

## 8. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| LCD shows nothing after flash | LCD wiring error or init failure | Check PORTB/PORTD connections against `LCD_config.h`; adjust contrast potentiometer |
| LCD shows garbled characters | Wrong 4-bit timing or incorrect pin mapping | Verify LCD pin mapping in `LCD_config.h`; check RS and E are on correct PORTD pins |
| PWM output not detected on PD5 | Timer1 not initialised | Confirm `PWM_voidInitChannel_1A()` is called before `PWM_voidGenerate_PWM_Channel_1A()` |
| Measurement never starts (state stays at 0) | INT0 not receiving the external signal | Check PD2 wiring; verify the external signal source is actually toggling |
| Frequency or duty cycle reads as zero or nonsense | Overflow counter not resetting between measurements | Confirm `Global_ovfCounter` is reset to 0 in State 0 of the ISR |
| AVRDUDE: cannot open device | Wrong port or USB driver issue | Specify the correct `-P COM3` or `-P /dev/ttyUSB0`; check USBasp USB driver installation |
| AVRDUDE: device signature does not match | Wrong MCU target flag | Confirm you are using `-p m32` for ATmega32; check ISP header orientation |
| Build error: undefined reference to function | Source file not added to project | Right-click project in Solution Explorer → Add Existing Item → add the missing `.c` file |
| Measurements are correct but LCD update is slow | State 3 not being polled fast enough | Ensure no blocking delays in the main loop between measurement capture and display |

---

## Project File Structure

```
PWM_drawer-main/
├── PWM_DESIGN.DSN                 ← Proteus circuit schematic
├── PWM_DESIGN.PWI                 ← Proteus workspace settings
│
└── PWM_drawer/
    ├── main.c                     ← Application entry point (switch reading, state polling)
    │
    ├── UTIL/                      ← Utility layer
    │   ├── STD_TYPES.h            ← Custom portable type aliases
    │   └── BIT_MATH.h             ← Bit manipulation macros
    │
    ├── MCAL/                      ← Microcontroller abstraction layer
    │   ├── DIO/                   ← Digital I/O driver (inc/ + src/)
    │   ├── PWM/                   ← PWM generation + measurement math (inc/ + src/)
    │   ├── TIMER0/                ← 8-bit timer driver (inc/ + src/)
    │   ├── EXTI/                  ← External interrupt driver, INT0 (inc/ + src/)
    │   ├── TWI/                   ← I²C driver (inc/ + src/)
    │   └── Global_Interrupt_Enable/ ← GI enable/disable (inc/ + src/)
    │
    ├── HAL/                       ← Hardware abstraction layer
    │   ├── LCD/                   ← 16×2 LCD driver, 4-bit mode (inc/ + src/)
    │   ├── KPD/                   ← 4×4 keypad driver (inc/ + src/)
    │   ├── SWITCH/                ← Push-button switch driver (inc/ + src/)
    │   └── EEPROM/                ← Internal EEPROM driver (inc/ + src/)
    │
    └── Debug/                     ← Pre-built binaries
        ├── PWM_drawer.hex         ← Flash directly with AVRDUDE
        └── PWM_drawer.elf         ← ELF file for JTAG debugging
```

---

*PWM Drawer Documentation — April 2026 | Yousef Osama Mohamed*
