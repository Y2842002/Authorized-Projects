import paho.mqtt.client as mqtt
import json
import time
import logging
from datetime import datetime
from pathlib import Path
from inference.scenario_handler import ScenarioHandler

# Load configuration
def load_config():
    config_path = Path("config.json")
    if config_path.exists():
        with open(config_path, 'r') as f:
            return json.load(f)
    else:
        # Default configuration if file doesn't exist
        return {
            "mqtt": {
                "broker": "broker.hivemq.com",
                "port": 1883,
                "topics": {
                    "command": "unity/motor/command",
                    "status": "unity/motor/status"
                },
                "client_id": "motor_inference_engine",
                "keepalive": 60
            },
            "system": {
                "log_level": "INFO"
            }
        }

CONFIG = load_config()

# Configure logging
logging.basicConfig(
    level=getattr(logging, CONFIG["system"].get("log_level", "INFO")),
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# MQTT Configuration from config.json
BROKER = CONFIG["mqtt"]["broker"]
PORT = CONFIG["mqtt"]["port"]
TOPIC_COMMAND = CONFIG["mqtt"]["topics"]["command"]
TOPIC_STATUS = CONFIG["mqtt"]["topics"]["status"]

TOPIC_MACHINE_ON = CONFIG["mqtt"]["topics"]["machine_on"]
TOPIC_MACHINE_OFF = CONFIG["mqtt"]["topics"]["machine_off"]

class MQTTInferenceEngine:
    def __init__(self):
        client_id = CONFIG["mqtt"].get("client_id", "motor_inference_engine")
        self.client = mqtt.Client(client_id=client_id)
        self.scenario_handler = ScenarioHandler()
        self.machine_on = True
        
        # Setup callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        
    def on_connect(self, client, userdata, flags, rc):
        
        if rc == 0:
            logger.info("✅ Connected to MQTT Broker!")

            client.subscribe(TOPIC_COMMAND)
            client.subscribe(TOPIC_MACHINE_ON)
            client.subscribe(TOPIC_MACHINE_OFF)

            logger.info(f"📥 Subscribed to: {TOPIC_COMMAND}")
            logger.info(f"📥 Subscribed to: {TOPIC_MACHINE_ON}")
            logger.info(f"📥 Subscribed to: {TOPIC_MACHINE_OFF}")

            self.publish_state("ON", "N/A", "N/A", "Ready")
        else:
            logger.error(f"❌ Failed to connect, return code {rc}")
    
    def on_disconnect(self, client, userdata, rc):
            if rc != 0:
                logger.warning(f"⚠️ Unexpected disconnection. Code: {rc}")
            else:
                logger.info("Disconnected from broker")
    
    def on_message(self, client, userdata, msg):
        try:
            topic = msg.topic
            payload = msg.payload.decode()

            # 1️⃣ MACHINE STATE EVENTS (authoritative)
            if topic == TOPIC_MACHINE_ON:
                self.handle_machine_on()
                return

            if topic == TOPIC_MACHINE_OFF:
                self.handle_machine_off()
                return

            # 2️⃣ UNITY COMMANDS (logical control)
            if topic == TOPIC_COMMAND:
                self.handle_unity_command(payload)
                return

            logger.warning(f"⚠️ Message from unknown topic: {topic}")

        except Exception as e:
            logger.error(f"❌ Error processing message: {e}")
    
    def publish_state(self, machine_state, tension, stability, health):
        """Publish current state to Unity"""
        message = {
            "timestamp": datetime.now().isoformat(),
            "machine_state": machine_state,
            "tension": tension,
            "stability": stability,
            "health": health
        }
        self.client.publish(TOPIC_STATUS, json.dumps(message))
        logger.info(f"📤 Published: {message}")
    
    def publish_result(self, result):
        """Publish scenario result to Unity"""
        message = {
            "timestamp": datetime.now().isoformat(),
            "machine_state": "ON",
            "tension": result["tension"],
            "stability": result["stability"],
            "health": result["health"],
            "scenario": result["scenario"],
            "approach": result["approach"]
        }
        self.client.publish(TOPIC_STATUS, json.dumps(message))
        logger.info(f"📤 Published Result: {message}")
    
    def connect(self):
        """Connect to MQTT broker"""
        try:
            keepalive = CONFIG["mqtt"].get("keepalive", 60)
            logger.info(f"🔌 Connecting to {BROKER}:{PORT}...")
            self.client.connect(BROKER, PORT, keepalive)
            return True
        except Exception as e:
            logger.error(f"❌ Connection failed: {e}")
            return False
    
    def start(self):
        """Start the MQTT loop"""
        logger.info("🚀 Starting MQTT Inference Engine...")
        if self.connect():
            self.client.loop_forever()
        else:
            logger.error("Failed to start engine")

    def handle_machine_on(self):
        if not self.machine_on:
            self.machine_on = True
            logger.info("🟢 Machine state received: ON")
            self.publish_state("ON", "N/A", "N/A", "Ready")


    def handle_machine_off(self):
        if self.machine_on:
            self.machine_on = False
            logger.info("🔴 Machine state received: OFF")
            self.publish_state("OFF", "N/A", "N/A", "Stopped")

    def handle_unity_command(self, payload):
        payload = json.loads(payload)
        command = payload.get("command", "").upper()
        scenario = payload.get("scenario", "")

        if not self.machine_on:
            logger.warning("⚠️ Cannot run scenario - Machine is OFF")
            self.publish_state("OFF", "N/A", "N/A", "Machine must be ON")
            return

        if command == "RUN_SCENARIO":
            result = self.scenario_handler.process_scenario(scenario)
            self.publish_result(result)
        else:
            logger.warning(f"⚠️ Unknown command: {command}")
def main():
    engine = MQTTInferenceEngine()
    try:
        engine.start()
    except KeyboardInterrupt:
        logger.info("\n⏹️ Shutting down...")
        engine.client.disconnect()

if __name__ == "__main__":
    main()