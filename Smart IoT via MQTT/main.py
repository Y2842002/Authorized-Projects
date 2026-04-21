# ================================================================
#  ESP32 Smart Relay Controller — MicroPython
#  Board:    ESP32 (Wokwi / physical DevKit)
#  Purpose:  Reads NTC thermistor temperature, publishes to MQTT,
#            and controls a relay in manual or automatic mode.
# ================================================================

# ──────────────────────── Libraries ─────────────────────────────
import math
import network
import time
from machine import Pin, ADC, Timer
from umqtt.simple import MQTTClient

# ──────────────────────── Constants ─────────────────────────────
# Wi-Fi
WIFI_SSID           = "Wokwi-GUEST"
WIFI_PASSWORD       = ""

# MQTT broker
MQTT_BROKER         = "broker.hivemq.com"
MQTT_CLIENT_ID      = "esp32_client"
TOPIC_PUB           = "readings"         # temperature published here
TOPIC_SUB           = "mode"             # commands received here

# Thermistor — NTC 10 kΩ, B-coefficient = 3950
THERMISTOR_B        = 3950               # Beta constant of the NTC thermistor
THERMISTOR_T0       = 298.15             # Reference temperature: 25 °C in Kelvin
ADC_MAX             = 4095               # Maximum value for 12-bit ADC

# Thresholds
TEMP_THRESHOLD      = 60.0               # °C — relay turns ON above this in auto mode
ADC_DELTA           = 0.1                # Minimum ADC change to trigger a temperature recalculation
TEMP_PUBLISH_DELTA  = 0.5                # Minimum °C change to trigger an MQTT publish

# Timing
TIMER_PERIOD_MS     = 50                 # Temperature sampling interval (ms)
MAIN_LOOP_DELAY_S   = 0.1               # Pause between main loop iterations (prevents CPU thrashing)
MQTT_RETRY_DELAY_S  = 2                  # Wait between MQTT reconnect attempts (s)
WIFI_POLL_DELAY_S   = 0.5               # Wait between Wi-Fi connection checks (s)

# Pin assignments
PIN_RELAY           = 21
PIN_ADC             = 33

# ──────────────────────── Hardware Init ─────────────────────────
relay = Pin(PIN_RELAY, Pin.OUT)

adc = ADC(Pin(PIN_ADC))
adc.width(ADC.WIDTH_12BIT)

# ──────────────────────── Shared State ──────────────────────────
# NOTE: Variables marked (ISR-shared) are written by the timer ISR
#       and read by the main loop — treat them as volatile.
last_adc            = None   # (ISR-shared) last ADC reading
current_temp        = None   # (ISR-shared) most recent computed temperature (°C)
last_temp_published = None   # last temperature value sent over MQTT
mode                = "manual"  # current control mode: "manual" | "automatic"
client              = None      # active MQTTClient instance

# ──────────────────────── Temperature Sampling (ISR) ────────────
def temp_sample(_):
    """
    Timer ISR — called every TIMER_PERIOD_MS milliseconds.
    Reads the ADC and recomputes temperature only when the ADC
    value has changed by at least ADC_DELTA (avoids unnecessary math).

    Formula: Steinhart–Hart simplified (B-parameter equation)
        T(K) = 1 / ( ln(R/R0) / B + 1/T0 )
    where R/R0 is derived from the voltage-divider ADC reading.
    """
    global last_adc, current_temp

    adc_value = adc.read()

    if (last_adc is None) or (abs(adc_value - last_adc) >= ADC_DELTA):
        last_adc = adc_value
        ratio = ADC_MAX / adc_value - 1   # R/R0 from voltage divider
        current_temp = 1.0 / (math.log(ratio) / THERMISTOR_B + 1.0 / THERMISTOR_T0) - 273.15

timer = Timer(0)
timer.init(period=TIMER_PERIOD_MS, mode=Timer.PERIODIC, callback=temp_sample)

# ──────────────────────── Wi-Fi ─────────────────────────────────
def wifi():
    """
    Connects to Wi-Fi. Blocks until connected.
    Skips reconnection if already connected.
    Prints the assigned IP configuration on success.
    """
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    if wlan.isconnected():
        print("[Wi-Fi] Already connected:", wlan.ifconfig())
        return

    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    print("[Wi-Fi] Connecting...")

    while not wlan.isconnected():
        time.sleep(WIFI_POLL_DELAY_S)

    print("[Wi-Fi] Connected:", wlan.ifconfig())

# ──────────────────────── MQTT Connect ──────────────────────────
def mqtt_connect():
    """
    Creates a new MQTTClient and connects to the broker.
    Retries indefinitely on failure — never raises an exception
    to the caller, ensuring the system keeps attempting to recover.
    """
    global client

    while True:
        try:
            client = MQTTClient(MQTT_CLIENT_ID, MQTT_BROKER)
            client.connect()
            print("[MQTT] Connected to", MQTT_BROKER)
            return
        except Exception as e:
            print(f"[MQTT] Connection failed: {e} — retrying in {MQTT_RETRY_DELAY_S}s")
            time.sleep(MQTT_RETRY_DELAY_S)

# ──────────────────────── MQTT Message Callback ─────────────────
def on_message(topic, msg):
    """
    Called whenever a message arrives on a subscribed topic.
    Both topic and msg are decoded, stripped, and lowercased
    before comparison so user input is fault-tolerant.

    Supported commands on TOPIC_SUB ("mode"):
        "manual"    — switch to manual relay control
        "automatic" — switch to temperature-based automatic control
        "led_on"    — turn relay ON  (only accepted in manual mode)
        "led_off"   — turn relay OFF (only accepted in manual mode)
    """
    global mode

    topic = topic.decode().strip().lower()
    msg   = msg.decode().strip().lower()
    print(f"[MQTT] Received on '{topic}': '{msg}'")

    if msg == "manual":
        mode = "manual"
        print("[Mode] Switched to MANUAL")

    elif msg == "automatic":
        mode = "automatic"
        print("[Mode] Switched to AUTOMATIC")

    elif msg == "led_on":
        if mode == "manual":
            relay.value(1)
            print("[Relay] ON  (manual command)")
        else:
            print("[Relay] Ignored 'led_on' — not in manual mode")

    elif msg == "led_off":
        if mode == "manual":
            relay.value(0)
            print("[Relay] OFF (manual command)")
        else:
            print("[Relay] Ignored 'led_off' — not in manual mode")

    else:
        print(f"[MQTT] Unknown command: '{msg}'")

# ──────────────────────── Main Loop ─────────────────────────────
def pub_and_sub():
    """
    Subscribes to TOPIC_SUB and enters the main control loop:
      1. Publish temperature to TOPIC_PUB when it changes by >= TEMP_PUBLISH_DELTA.
      2. Control the relay automatically when mode == "automatic".
      3. Poll for incoming MQTT messages via check_msg().

    If the MQTT connection drops (OSError), the function reconnects
    automatically and re-subscribes without restarting the device.
    """
    global last_temp_published, client

    counter = 1

    client.set_callback(on_message)
    client.subscribe(TOPIC_SUB)
    print(f"[MQTT] Subscribed to '{TOPIC_SUB}'")

    while True:
        try:
            # ── 1. Publish temperature if changed enough ──────────
            if current_temp is not None:
                if (last_temp_published is None) or (abs(current_temp - last_temp_published) >= TEMP_PUBLISH_DELTA):
                    client.publish(TOPIC_PUB, f"{current_temp:.2f}")
                    print(f"[Publish #{counter}] {current_temp:.2f} °C")
                    last_temp_published = current_temp
                    counter += 1

            # ── 2. Automatic relay control ────────────────────────
            if mode == "automatic" and current_temp is not None:
                relay.value(1 if current_temp > TEMP_THRESHOLD else 0)

            # ── 3. Check for incoming MQTT messages ───────────────
            client.check_msg()

        except OSError as e:
            # Connection dropped — reconnect and re-subscribe
            print(f"[MQTT] Connection lost ({e}), reconnecting...")
            mqtt_connect()
            client.set_callback(on_message)
            client.subscribe(TOPIC_SUB)
            print(f"[MQTT] Re-subscribed to '{TOPIC_SUB}'")

        time.sleep(MAIN_LOOP_DELAY_S)   # prevent CPU thrashing

# ──────────────────────── Entry Point ───────────────────────────
def main():
    wifi()
    mqtt_connect()
    pub_and_sub()

main()
