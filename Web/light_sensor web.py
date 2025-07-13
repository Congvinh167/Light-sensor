from flask import Flask, render_template, jsonify
import threading
import paho.mqtt.client as mqtt

app = Flask(__name__)

# Shared variable to hold latest light intensity
current_light = 0.0

# === MQTT Setup ===
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    client.subscribe("sensor/light")

def on_message(client, userdata, msg):
    global current_light
    try:
        current_light = float(msg.payload.decode())
    except:
        print("Invalid payload:", msg.payload)

def mqtt_thread():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_forever()

# === Flask Web Routes ===
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/light')
def get_light():
    return jsonify({'lux': current_light})

if __name__ == '__main__':
    # Start MQTT in background
    threading.Thread(target=mqtt_thread, daemon=True).start()
    app.run(debug=True)
