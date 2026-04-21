# Quick Start - Running the Inference Engine

## Prerequisites
1. Python 3.8+ installed
2. Virtual environment set up
3. All dependencies installed

## Steps to Run

### 1. Activate Virtual Environment
```bash
# Windows
venv\Scripts\activate

# Mac/Linux
source venv/bin/activate
```

### 2. Start the Inference Engine
```bash
python main.py
```

You should see:
```
==================================================
ABM Inference Engine - Starting...
==================================================
✓ Stability model loaded
✓ Model config loaded
✓ Energy baseline loaded
✓ Connected to MQTT Broker
✓ Subscribed to: abm/sensor/data

✓ Inference Engine Ready!
Listening on: abm/sensor/data
Publishing to: abm/predictions
--------------------------------------------------
```

### 3. Test It (Optional)
In a **new terminal window**:
```bash
# Activate venv again
venv\Scripts\activate

# Run test
python test_mqtt_sender.py
```

### 4. Keep It Running
- Leave the inference engine running while Unity is connected
- Press `Ctrl+C` to stop it gracefully

## Troubleshooting

**"Connection failed"**
- Make sure MQTT broker (Mosquitto) is running
- Check if port 1883 is available

**"Error loading models"**
- Verify files exist in `models/` folder:
  - `stability_model.pkl`
  - `model_config.pkl`
  - `energy_baseline.npy`

**"ModuleNotFoundError"**
- Make sure you activated the virtual environment
- Run: `pip install -r requirements.txt`
```

---

## What to Actually Send

### Option 1: Send via Email/Document Sharing

**Subject**: ABM Inference Engine - MQTT Integration Package

**Email Body**:
```
Hi [Name],

The AI inference engine is complete and ready for integration. 

Attached/Included:
1. MQTT_Integration_Guide.md - Complete technical specification
2. test_mqtt_sender.py - Python test script to verify connection
3. QUICK_START.md - Instructions to run the engine

Key Points:
- Communication: MQTT protocol on localhost:1883
- Input topic: abm/sensor/data
- Output topic: abm/predictions
- Response time: <100ms per prediction
- Currently supports Binary Stability Classification (Stable/Unstable)
- Phase 2 will add 3-Class Tension Classification

The engine is tested and working. Run test_mqtt_sender.py to see it in action.

Let me know when you're ready to test the integration from Unity's side.

Best regards,
[Your Name]
```

### Option 2: Share the Entire Folder

If you're using Git/GitHub or shared drive:

**What to share:**
```
BELTM.../
├── inference/              ← Your code
├── mqtt/                   ← Your code  
├── config/                 ← Configuration
├── models/                 ← AI models (DON'T SHARE if large)
├── main.py                 ← Entry point
├── test_mqtt_sender.py     ← Test script
├── MQTT_Integration_Guide.md    ← Documentation
├── QUICK_START.md          ← How to run
└── requirements.txt        ← Dependencies