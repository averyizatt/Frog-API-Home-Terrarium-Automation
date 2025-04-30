from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
from werkzeug.middleware.dispatcher import DispatcherMiddleware
from werkzeug.serving import run_simple
import json, time, requests
from pathlib import Path

# === Core Flask App ===
app = Flask(__name__)
CORS(app)

# === Config ===
logdir = Path("/home/thefrogpit/frog-api/logs")
logdir.mkdir(parents=True, exist_ok=True)

sensor_name_map = {
    "Whites Tree Frog Terrarium": "whites",
    "Green Tree Frog Terrarium": "green",
    "Office Sensor": "office",
    "Other Sensor": "other",
    "Red Knee": "red knee",
    "Avicularia Avicularia": "avicularia avicularia",
    "Living Room": "living room",
    "Bedroom": "bedroom",
    "3D Printer": "3d printer"
}
sensor_labels = {v: k for k, v in sensor_name_map.items()}

thresholds = {
    "whites": {"temp": (70, 85), "humidity": (50, 80)},
    "green": {"temp": (72, 85), "humidity": (50, 80)},
    "red knee": {"temp": (75, 80), "humidity": (60, 70)},
    "avicularia avicularia": {"temp": (75, 85), "humidity": (70, 80)},
    "office": {"temp": (0, 100), "humidity": (0, 100)},
    "other": {"temp": (0, 100), "humidity": (0, 100)},
    "living room": {"temp": (60, 85), "humidity": (20, 60)},
    "bedroom": {"temp": (60, 85), "humidity": (20, 60)},
    "3d printer": {"temp": (0, 100), "humidity": (0, 100)}
}

# === Routes ===

@app.route("/sensor/<sensor_name>")
def latest(sensor_name):
    logfile = logdir / f"{sensor_name}.csv"
    try:
        with open(logfile) as f:
            last = f.readlines()[-1].strip().split(",")
            return jsonify({
                "time": last[0],
                "sensor": sensor_labels.get(sensor_name, sensor_name),
                "temp": last[2],
                "humidity": last[3],
                "lux": last[4] if len(last) > 4 else None,
                "tds": last[5] if len(last) > 5 else None
            })
    except Exception as e:
        return jsonify({"error": "no data", "detail": str(e)}), 404

@app.route("/sensor/<sensor_name>-log")
def sensor_log(sensor_name):
    logfile = logdir / f"{sensor_name}.csv"
    if not logfile.exists():
        return "Log file not found", 404
    return send_file(logfile, mimetype="text/plain")

@app.route("/graph/<sensor_name>")
def graph_page(sensor_name):
    return send_file("/var/www/homer/assets/graph.html", mimetype="text/html")

@app.route("/", methods=["GET"])
def dashboard_home():
    return send_file("/var/www/dashboard/index.html")

@app.route("/api/sensor", methods=["POST"])
def log_data():
    data = request.json
    full_name = data.get("sensor", "unknown")
    sensor = sensor_name_map.get(full_name, full_name.lower())
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    logfile = logdir / f"{sensor}.csv"

    # Extract all readings
    temp = data.get("temp", "")
    humidity = data.get("humidity", "")
    lux = data.get("lux", "")
    tds = data.get("tds", "")

    with open(logfile, "a") as f:
        f.write(f"{ts},{sensor},{temp},{humidity},{lux},{tds}\n")

    # Optional: alert logic for temp/humidity
    try:
        temp_val = float(temp)
        humidity_val = float(humidity)
        limits = thresholds.get(sensor, thresholds["other"])

        if not (limits["temp"][0] <= temp_val <= limits["temp"][1]) or not (limits["humidity"][0] <= humidity_val <= limits["humidity"][1]):
            alert_msg = f"{sensor_labels.get(sensor, sensor)} out of range!\nTemp: {temp_val}°F | Humidity: {humidity_val}%"
            requests.post(
                "https://ntfy.sh/thefrogpit",
                data=alert_msg.encode("utf-8"),
                headers={
                    "Title": "🐸 FrogTank Alert",
                    "Priority": "5",
                    "Tags": "frog,warning,temp"
                }
            )
    except Exception as e:
        print(f"[ntfy Error] {e}")

    return jsonify({"status": "ok"}), 200

# === Mount app under /frogtank ===
application = DispatcherMiddleware(Flask("dummy"), {
    "/frogtank": app
})

if __name__ == "__main__":
    run_simple("0.0.0.0", 5020, application)
