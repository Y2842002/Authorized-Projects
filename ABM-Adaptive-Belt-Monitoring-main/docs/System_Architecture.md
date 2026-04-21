# System Architecture - ABM Inference Engine

## High-Level Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                        Unity Dashboard                       │
│  (Visualization, Real-time Monitoring, User Interface)      │
└────────────────────────┬───────────────────────────────────┘
                         │
                         │ MQTT Protocol
                         │ (Bidirectional)
                         │
┌────────────────────────▼───────────────────────────────────┐
│                    MQTT Broker (Mosquitto)                  │
│              Topics: abm/sensor/data ↔ abm/predictions      │
└────────────────────────┬───────────────────────────────────┘
                         │
                         │
┌────────────────────────▼───────────────────────────────────┐
│              AI Inference Engine (Python)                   │
│                                                              │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│  │ MQTT Client │→ │ Preprocessor │→ │   AI Models     │   │
│  │   Handler   │  │  & Validator │  │  (scikit-learn) │   │
│  └─────────────┘  └──────────────┘  └─────────────────┘   │
│                                               │              │
│                                               ▼              │
│                                      ┌─────────────────┐    │
│                                      │  Predictions    │    │
│                                      │  (JSON Output)  │    │
│                                      └─────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Component Breakdown

### 1. MQTT Communication Layer (`mqtt/`)

**Purpose**: Handles all communication with Unity

**Components**:
- `mqtt_client.py`: Manages MQTT connection, subscriptions, and message routing

**Responsibilities**:
- Connect to MQTT broker
- Subscribe to input topic (`abm/sensor/data`)
- Parse incoming JSON messages
- Publish predictions to output topic (`abm/predictions`)
- Handle connection errors and reconnection

---

### 2. Inference Layer (`inference/`)

**Purpose**: Core AI logic for predictions

#### 2.1 Model Loader (`model_loader.py`)
- Loads trained scikit-learn models from disk
- Loads preprocessing configurations (scalers, feature names)
- Loads baseline data for health monitoring

#### 2.2 Preprocessor (`preprocessor.py`)
- Validates incoming sensor data
- Transforms features using saved scaler
- Calculates energy-based health metrics
- Ensures data format matches training data

#### 2.3 Predictor (`predictor.py`)
- Executes model predictions
- Calculates confidence scores
- Aggregates results from multiple models
- Returns structured prediction objects

---

### 3. Configuration (`config/`)

**mqtt_config.json**: MQTT connection settings
- Broker address and port
- Topic names
- QoS and keepalive settings

---

## Data Flow

### Request Flow (Unity → AI)
```
1. Unity collects sensor data
   ↓
2. Formats as JSON with timestamp and request_id
   ↓
3. Publishes to MQTT topic: abm/sensor/data
   ↓
4. MQTT Broker routes message
   ↓
5. Inference Engine receives message
   ↓
6. Preprocessor validates and transforms data
   ↓
7. Predictor runs AI models
   ↓
8. Results formatted as JSON response
   ↓
9. Published to MQTT topic: abm/predictions
   ↓
10. Unity receives and displays prediction
```

### Processing Time Breakdown

| Step | Typical Time |
|------|--------------|
| MQTT transmission | <10ms |
| Data validation | <1ms |
| Feature preprocessing | <5ms |
| Model inference | 20-50ms |
| JSON serialization | <5ms |
| **Total** | **<100ms** |

---

## AI Models

### Model 1: Binary Stability Classifier

**Type**: Scikit-learn classifier (e.g., Random Forest, SVM, or XGBoost)

**Input Features** (3 physics-based features):
- `vib_intensity`: Vibration intensity magnitude
- `dom_freq`: Dominant frequency in Hz
- `spec_entropy`: Spectral entropy measure

**Output**:
- Class: "Stable" or "Unstable"
- Confidence: Probability score (0.0-1.0)

**Training Data**: 
- Collected from belt monitoring sensors
- Labeled by domain experts or threshold-based rules

---

### Health Monitoring: Energy Score

**Purpose**: Track motor/belt health over time

**Input**:
- `energy_score`: Derived from sensor readings

**Processing**:
- Compare against baseline (normal operation)
- Calculate deviation from expected range
- Classify as Normal/Warning/Critical

**Output**:
- Score: 0.0-1.0 (1.0 = perfect health)
- Status: "Normal", "Warning", or "Critical"

---

## Model Files

Required files in `models/` directory:

1. **`stability_model.pkl`**: Trained binary classifier
2. **`model_config.pkl`**: Contains:
   - Feature scaler (StandardScaler or similar)
   - Feature names and order
   - Model metadata
3. **`energy_baseline.npy`**: Baseline energy values from normal operation

---

## Error Handling

### Input Validation Errors
- Missing required features → Error response with details
- Invalid data types → Error response with expected format
- Out-of-range values → Log warning, attempt prediction

### Model Errors
- Model file not found → Exit at startup with clear message
- Prediction failure → Return error response, log details

### MQTT Errors
- Connection lost → Attempt reconnection with backoff
- Message parsing error → Return error response
- Broker unavailable → Retry connection, log status

---

## Scalability Considerations

### Current Design
- Single-threaded processing
- Handles requests sequentially
- Suitable for: 1-10 requests/second

### Future Scaling Options
- Multi-threading for parallel predictions
- Message queuing for high load
- Multiple inference engine instances
- Load balancing across engines

---

## Security Notes

**Current Implementation**:
- Local MQTT broker (localhost)
- No authentication required
- Suitable for development/testing

**Production Recommendations**:
- Enable MQTT authentication (username/password)
- Use TLS/SSL encryption
- Implement API rate limiting
- Add input sanitization
- Log all requests for audit

---

## Testing Strategy

### Unit Tests (Recommended)
- Test each component independently
- Mock MQTT messages
- Validate preprocessor logic
- Verify model loading

### Integration Tests
- End-to-end message flow
- Use `test_mqtt_sender.py`
- Verify JSON format compliance
- Test error scenarios

### Performance Tests
- Measure response time under load
- Test with burst requests
- Monitor memory usage
- Profile bottlenecks

---

## Future Enhancements (Phase 2)

### Model Addition
- **3-Class Tension Classifier**
  - Output: Loose, Optimal, or Tight
  - Same input features
  - Confidence scores for each class

### Architecture Updates
- Add tension model to `model_loader.py`
- Update `predictor.py` with `predict_tension()` method
- Expand JSON response structure

### Additional Features
- Historical data logging
- Prediction trend analysis
- Anomaly detection
- Model performance monitoring
```

---

## Step 3: How to Send Everything

### **Option A: Email with Attachments** (Recommended if small)

#### **Compress the package:**
1. Right-click on `ABM_Inference_Engine_Package` folder
2. Select "Send to" → "Compressed (zipped) folder" (Windows)
   - Or use 7-Zip, WinRAR, etc.
3. Name it: `ABM_Inference_Engine_v1.0.zip`

#### **Email Template:**

**To**: [Unity Developer / Integration Lead]  
**CC**: [Project Manager / Supervisor if applicable]  
**Subject**: ABM Inference Engine - Ready for Integration (AI Module Delivery)

**Email Body**:
```
Hi [Name],

I've completed the AI inference engine for the ABM project. The package is attached and ready for integration with Unity.

📦 What's Included:
- Complete inference engine code (Python)
- MQTT communication module
- Test scripts for validation
- Full technical documentation
- Integration guide with message format specifications

🔑 Key Information:
- Communication: MQTT protocol (localhost:1883)
- Response Time: <100ms per prediction
- Current Model: Binary Stability Classifier (Stable/Unstable)
- Status: Tested and working ✓

📋 Next Steps:
1. Extract the attached ZIP file
2. Follow QUICK_START.md to run the engine
3. Test with the included test_mqtt_sender.py script
4. Review MQTT_Integration_Guide.md for Unity integration details
5. Let me know when you're ready for integration testing

⚠️ Important Note:
The AI model files (stability_model.pkl, model_config.pkl, energy_baseline.npy) are NOT included in this package due to file size. I will provide them separately via [Google Drive / OneDrive / USB / your preferred method].

📁 Model Files Location:
[Specify how you'll deliver them - see options below]

📖 Documentation:
- README.md - Start here for overview
- docs/MQTT_Integration_Guide.md - Technical specifications
- docs/QUICK_START.md - Step-by-step running instructions
- docs/System_Architecture.md - System design details

🧪 Testing:
I've included a test script (test_mqtt_sender.py) that simulates Unity sending data. You can use this to verify the engine is working before integrating with Unity.

Let me know if you need any clarification or run into issues. I'm available for integration support and troubleshooting.

Best regards,
[Your Name]
[Your Email]
[Your Phone - optional]

---
Project: ABM - Adaptive Belt Monitoring
Module: AI Inference Engine
Version: 1.0.0
Date: December 30, 2025
```

---

### **Option B: Shared Cloud Storage** (Recommended if large files)

Use Google Drive, OneDrive, Dropbox, etc.

#### **Steps:**
1. Upload `ABM_Inference_Engine_Package` folder to cloud storage
2. Create a shareable link (anyone with link can view)
3. Send email with the link

#### **Email Template:**

**Subject**: ABM Inference Engine - Ready for Integration (Download Link)
```
Hi [Name],

The AI inference engine for ABM is complete and ready for integration.

📥 Download Package:
[Insert Google Drive / OneDrive link here]

Package Size: ~[X] MB
Includes: Complete code, documentation, test scripts

📁 Model Files (Large Files):
[Separate link for model files]
- stability_model.pkl
- model_config.pkl  
- energy_baseline.npy

Size: ~[X] MB

[Rest of email same as Option A]
```

---

### **Option C: GitHub Repository** (Best for version control)

If your team uses Git:

#### **Steps:**
1. Initialize Git in your package folder
2. Create `.gitignore` file:
```
# Don't commit these
venv/
__pycache__/
*.pyc
models/*.pkl
models/*.npy
.DS_Store