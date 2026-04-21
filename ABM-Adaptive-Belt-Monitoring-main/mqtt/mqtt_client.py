import paho.mqtt.client as mqtt
import json
from datetime import datetime

class MQTTClient:
    """Handles MQTT communication with Unity"""
    
    def __init__(self, config, predictor):
        self.config = config
        self.predictor = predictor
        self.client = mqtt.Client()
        
        # Set up callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
    
    def on_connect(self, client, userdata, flags, rc):
        """Callback when connected to broker"""
        if rc == 0:
            print(f"✓ Connected to MQTT Broker")
            # Subscribe to input topic
            topic = self.config['topics']['input']
            client.subscribe(topic, qos=self.config['qos'])
            print(f"✓ Subscribed to: {topic}")
        else:
            print(f"✗ Connection failed with code {rc}")
    
    def on_disconnect(self, client, userdata, rc):
        """Callback when disconnected"""
        if rc != 0:
            print(f"⚠ Unexpected disconnection. Code: {rc}")
    
    def on_message(self, client, userdata, msg):
        """Callback when message received from Unity"""
        try:
            # Parse incoming JSON
            payload = json.loads(msg.payload.decode())
            print(f"\n→ Received: {payload.get('request_id', 'unknown')}")
            
            # Extract sensor data
            sensor_data = payload.get('sensor_data', {})
            request_id = payload.get('request_id', 'unknown')
            
            # Run prediction
            predictions = self.predictor.predict_all(sensor_data)
            
            # Prepare response
            response = {
                "request_id": request_id,
                "timestamp": datetime.now().isoformat(),
                "predictions": predictions
            }
            
            # Send back to Unity
            self.publish_prediction(response)
            print(f"← Sent: {request_id} | {predictions['stability']['class']}")
            
        except Exception as e:
            print(f"✗ Error processing message: {e}")
            # Send error response
            error_response = {
                "request_id": payload.get('request_id', 'unknown'),
                "error": str(e),
                "timestamp": datetime.now().isoformat()
            }
            self.publish_prediction(error_response)
    
    def publish_prediction(self, result):
        """Send prediction back to Unity"""
        topic = self.config['topics']['output']
        payload = json.dumps(result)
        self.client.publish(topic, payload, qos=self.config['qos'])
    
    def connect(self):
        """Connect to MQTT broker"""
        try:
            self.client.connect(
                self.config['broker'],
                self.config['port'],
                self.config['keepalive']
            )
            return True
        except Exception as e:
            print(f"✗ Connection error: {e}")
            return False
    
    def start(self):
        """Start the MQTT loop (blocking)"""
        print("Starting MQTT loop...")
        self.client.loop_forever()
    
    def stop(self):
        """Stop and disconnect"""
        self.client.loop_stop()
        self.client.disconnect()