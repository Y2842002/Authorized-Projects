# ABM – Adaptive Belt Monitoring  
### AI-Driven Digital Twin for Tension Stability and Motor Health

ABM (Adaptive Belt Monitoring) is an **AI-powered digital twin system** designed to monitor **belt tension stability and motor health** in industrial environments.  
The system combines **machine learning models**, **real-time inference**, and **MQTT-based streaming** to provide live diagnostics visualized through a **Unity dashboard**.

---

## 🚀 Project Overview

Industrial belt-driven systems often suffer from hidden issues such as:
- Incorrect belt tension (loose / optimal / tight)
- Instability during operation
- Progressive motor health degradation

ABM addresses these challenges by:
- Analyzing vibration and energy-based features
- Classifying belt tension and stability in real time
- Streaming AI inference results to a digital twin dashboard
- Providing early warnings for unsafe or critical operating conditions

---

## 🧠 AI Models Used

The system uses **two machine learning models**:

### 1️⃣ Binary Stability Classification Model
- **Output:** Stable / Unstable
- **Purpose:** Detect operational instability in real time
- **Features:** Energy-based vibration metrics
- **Baseline tracking:** Drift detection for motor health

### 2️⃣ Multi-Class Tension Classification Model
- **Output:** Loose / Optimal / Tight
- **Purpose:** Identify belt tension condition
- **Features:** Extracted vibration features + scaling
- **Integration:** Combined with stability results for final health assessment

---

## ⚙️ Inference Engine & Streaming

- Implemented in **Python**
- Real-time inference pipeline
- MQTT-based communication
- Designed for **live integration with Unity**
- Supports scenario-based logic for machine state handling

Key components:
- `inference/` → Scenario handling & decision logic
- `mqtt/` → MQTT client & message publishing
- `models/` → Trained ML models & configurations
- `config/` → MQTT and system configuration files

---

## 🎮 Unity Digital Twin Integration

- Unity acts as the **HMI / dashboard**
- Receives AI inference results via **MQTT**
- Visualizes:
  - Belt tension state
  - Stability status
  - Motor health warnings
- Enables interactive monitoring in real time

> ⚠️ The Unity project is maintained separately and connected through MQTT topics defined in `unity_config.json`.

---

## 📊 Dataset Information

The dataset used in this project is **not proprietary**.

- Source: **Fraunhofer Institute (Germany)**
- Dataset link:  
  https://fordatis.fraunhofer.de/handle/fordatis/347

The dataset was used strictly for **research and educational purposes**.

---

## 🎬 Demo

A demo video demonstrating:
- Live inference
- Scenario changes
- Unity dashboard visualization

📁 Available in: demo/ABM Demo Video.mp4

Team Members
- Sara Hassan Mohamed — AI Engineer
- Dalia Abdelmonem — AI Engineer
- Mohamed Magdy — AI Engineer
- Ramy Elhosary — Integration Engineer
- Ahmed Mostafa — Unity Developer
- Youssef Osama — Automation Engineer

License
This project is shared for educational and research purposes. 