# ABM Inference Engine - MQTT Integration Guide

## Overview
The AI Inference Engine is ready and communicates via MQTT protocol for real-time predictions.

---

## MQTT Connection Details

### Broker Configuration
- **Broker Address**: `localhost` (or your server IP if hosted remotely)
- **Port**: `1883` (default MQTT port)
- **Protocol**: MQTT v3.1.1
- **QoS Level**: 1 (at least once delivery)

### Topics
- **Unity → AI (Input)**: `abm/sensor/data`
- **AI → Unity (Output)**: `abm/predictions`

---

## Message Format

### 1. Unity Sends to AI (Request)

**Topic**: `abm/sensor/data`

**JSON Structure**:
```json
{
  "timestamp": "2025-12-30T10:30:00Z",
  "sensor_data": {
    "vib_intensity": 0.45,
    "dom_freq": 32.5,
    "spec_entropy": 2.1,
    "energy_score": 0.75
  },
  "request_id": "req_12345"
}
```

**Field Descriptions**:
- `timestamp`: ISO 8601 format (YYYY-MM-DDTHH:MM:SSZ)
- `sensor_data`: Object containing the 3 required features + energy score
  - `vib_intensity`: Vibration intensity (float, range: 0-1)
  - `dom_freq`: Dominant frequency in Hz (float, typical range: 0-100)
  - `spec_entropy`: Spectral entropy (float, range: 0-5)
  - `energy_score`: Energy score for health monitoring (float, range: 0-1)
- `request_id`: Unique identifier for this request (string)

---

### 2. AI Sends to Unity (Response)

**Topic**: `abm/predictions`

**JSON Structure**:
```json
{
  "request_id": "req_12345",
  "timestamp": "2025-12-30T10:30:01Z",
  "predictions": {
    "stability": {
      "class": "Stable",
      "confidence": 0.92
    },
    "energy_health": {
      "score": 0.85,
      "status": "Normal"
    }
  }
}
```

**Field Descriptions**:
- `request_id`: Matches the request (for tracking)
- `timestamp`: When prediction was made (ISO 8601)
- `predictions.stability.class`: Either "Stable" or "Unstable"
- `predictions.stability.confidence`: Probability (0.0-1.0)
- `predictions.energy_health.score`: Health score (0.0-1.0)
- `predictions.energy_health.status`: "Normal", "Warning", or "Critical"

---

### 3. Error Response (if something goes wrong)
```json
{
  "request_id": "req_12345",
  "timestamp": "2025-12-30T10:30:01Z",
  "error": "Missing features: ['dom_freq']"
}
```

---

## Testing Instructions

### Test Tool Options
You can test the connection using:
1. **MQTT Explorer** (Recommended - GUI tool): http://mqtt-explorer.com/
2. **Mosquitto CLI** (Command line)
3. **Python test script** (provided below)

### Quick Test with MQTT Explorer
1. Download and install MQTT Explorer
2. Connect to `localhost:1883`
3. Publish to topic `abm/sensor/data` with the sample request JSON above
4. Check topic `abm/predictions` for the response

### Python Test Script
See attached `test_mqtt_sender.py` file

---

## Integration Checklist for Unity Team

- [ ] MQTT client library installed in Unity (e.g., M2MqttUnity)
- [ ] Connect to broker at `localhost:1883`
- [ ] Subscribe to topic: `abm/predictions`
- [ ] Publish sensor data to topic: `abm/sensor/data`
- [ ] Parse incoming JSON responses
- [ ] Handle errors gracefully (check for `error` field in response)
- [ ] Match `request_id` to track requests/responses
- [ ] Test with sample data before live integration

---

## Support & Contact

**AI Model Owner**: [Your Name]
**Email**: [Your Email]

For issues:
1. Check if the inference engine is running (`python main.py`)
2. Verify MQTT broker is running (Mosquitto or other)
3. Test with MQTT Explorer to isolate Unity vs AI issues
4. Check console output for error messages

---

## Notes
- The AI inference engine must be running before Unity connects
- Response time is typically < 100ms per prediction
- The engine can handle multiple simultaneous requests
- All timestamps should be in UTC

## Future Enhancements (Phase 2)
- Addition of 3-Class Tension Classifier (Loose/Optimal/Tight)
- Extended health metrics