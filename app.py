from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
from werkzeug.middleware.dispatcher import DispatcherMiddleware
from werkzeug.serving import run_simple
import json, time
from pathlib import Path

# === Flask App Setup ===
app = Flask(__name__)
CORS(app)  # Enable cross-origin requests

# === Config ===
logdir = Path("/home/thefrogpit/frog-api/logs")
logdir.mkdir(parents=True, exist_ok=True)

# Maps long ESP names to short file-friendly names
sensor_name_map = {
    "Whites Tree Frog Terrarium": "whites",
    "Green Tree Frog Terrarium": "green",
    "Office Sensor": "office",
    "Other Sensor": "other"
}
sensor_labels = {v: k for k, v in sensor_name_map.items()}

# === Routes ===

@app.route("/api/sensor", methods=["POST"])
def log_data():
    data = request.json
    full_name = data.get("sensor", "unknown")
    sensor = sensor_name_map.get(full_name, full_name.lower())
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    logfile = logdir / f"{sensor}.csv"

    with open(logfile, "a") as f:
        f.write(f"{ts},{sensor},{data['temp']},{data['humidity']}\n")

    return jsonify({"status": "ok"}), 200

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
                "humidity": last[3]
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

# === Mount app under /frogtank ===
application = DispatcherMiddleware(Flask("dummy"), {
    "/frogtank": app
})

# === Run via Werkzeug (instead of app.run) ===
if __name__ == "__main__":
    run_simple("0.0.0.0", 5010, application)
