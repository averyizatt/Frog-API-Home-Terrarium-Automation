# 🐸 Frog API - Terrarium Environmental Monitoring System

The Frog API is a Flask-based backend designed for real-time monitoring of terrarium environments.  
It collects, stores, and serves real-time environmental data from terrarium setups, including live dashboards and graph displays.  
The Frog API is lightweight, modular, and built for reliability on Ubuntu Server, designed to scale with additional sensors and tanks.

---

## 📍 Live Dashboard

**Main Monitoring Page:**  
https://averyizatt.com/frogtank

**Graph View:**  
https://averyizatt.com/terrarium

---

## 🌟 Key Features

- Collects sensor data via secure JSON POST requests
- Supports Temperature (°C), Humidity (%), Light Intensity (lux), Total Dissolved Solids (TDS ppm), and Water Distance (cm)
- Auto-logs each reading into timestamped per-sensor CSV files
- Live dashboard served via Nginx reverse proxy under `/frogtank`
- Historical graph pages for every sensor
- Sends real-time Ntfy push alerts for critical sensor thresholds
- Lightweight Flask API mounted under `/frogtank` via DispatcherMiddleware
- Integrates seamlessly with Homer dashboard tiles
- Easily expandable for additional sensors and automation

---

## 📁 Project Structure Overview

```
frog-api/
├── app.py        # Core Flask application
├── logs/         # CSV sensor logs (auto-created)
└── static/       # HTML pages served externally
```

---

## 🛠️ Supported Sensor Types

- **Temperature** — °C readings from DHT11 sensors
- **Humidity** — % relative humidity from DHT11 sensors
- **Light Level** — Lux measurements from BH1750 sensors
- **Water Quality** — TDS (ppm) readings from analog TDS sensors
- **Water Level** — Distance (cm) from HC-SR04 ultrasonic sensors

---

## 📡 API Endpoints

### Send Sensor Data (POST)
`POST /frogtank/api/sensor`

Body Example (JSON):
```
{
  "sensor": "Green Tree Frog Terrarium",
  "temp": 24.5,
  "humidity": 68.0,
  "lux": 400,
  "tds": 300,
  "distance": 12.3
}
```

### Get Latest Sensor Reading
`GET /frogtank/sensor/{sensor_name}`

### Download Full Sensor Log
`GET /frogtank/sensor/{sensor_name}-log`

### View Graph of Sensor Data
`GET /frogtank/graph/{sensor_name}`

### Access Dashboard Homepage
`GET /frogtank/`

---

## 🔔 Real-Time Notifications

The Frog API uses [Ntfy](https://ntfy.sh/) to send real-time push notifications when sensors detect:

- Temperature too high or low
- Humidity out of safe range
- TDS levels unsafe for aquatic systems
- Abnormal water distance indicating low water levels

**Notification Topic:** `thefrogpit`

Example Alert:
> "⚠️ ALERT: Green Tree Frog Terrarium temperature dropped to 16°C!"

---

## 🌐 Nginx Reverse Proxy Example

Add the following block to forward HTTPS `/frogtank` requests:

```
location /frogtank {
    proxy_pass http://127.0.0.1:5020/frogtank;
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
}
```

---

## 🧠 Sensor Name Mappings

| Sensor Description              | API Name  |
|----------------------------------|-----------|
| Whites Tree Frog Terrarium       | whites    |
| Green Tree Frog Terrarium        | green     |
| Office Sensor                    | office    |
| Other Sensor                     | other     |
| Living Room Sensor               | living    |
| Bedroom Sensor                   | bedroom   |
| 3D Printer Sensor                | printer   |

---

## 📊 Data Logging and Retention

- Every sensor reading is timestamped and recorded into a CSV log.
- Each sensor has its own CSV file.
- Future enhancements will include automatic pruning of logs older than 7 days.

---

## ✅ Deployment Instructions

1. Clone the repository:
```
git clone https://github.com/yourname/frog-api.git
```

2. Install Python dependencies:
```
pip install flask flask-cors werkzeug
```

3. Launch the application manually:
```
python3 app.py
```

4. Set up a systemd service (`frog-api.service`) for automatic boot startup.

---

## 📈 Planned Improvements

- WebSocket support for instant live updates
- Admin dashboard for safe range management
- OAuth2-protected routes for sensor and threshold settings
- Multiple tank profile management
- Enhanced device health checks and downtime alerts

---

## 🐸 Maintainer Information

**Project Lead:** Avery Izatt  
**Website:** [averyizatt.com](https://averyizatt.com)  
**Primary Server:** thefrogpit  
**Location:** Colorado

---

# End of Document
