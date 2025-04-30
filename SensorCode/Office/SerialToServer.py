import serial
import requests
import time

# === CONFIG ===
serial_port = "/dev/ttyACM0"
baud_rate = 115200
server_url = "http://localhost:5020/frogtank/api/sensor"

# === SETUP SERIAL ===
ser = serial.Serial(serial_port, baud_rate, timeout=1)
time.sleep(2)  # Give time for Arduino reset

print("Listening on", serial_port)

while True:
    try:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print("Received:", line)

            # Only process lines that start with "sensor:"
            if not line.startswith("sensor:"):
                continue

            # Parse sensor line
            parts = line.split(",")
            sensor_name = None
            temp = None
            humidity = None

            for part in parts:
                if part.startswith("sensor:"):
                    sensor_name = part.split("sensor:")[1]
                elif part.startswith("temp:"):
                    temp = float(part.split("temp:")[1])
                elif part.startswith("humidity:"):
                    humidity = float(part.split("humidity:")[1])

            if sensor_name and temp is not None and humidity is not None:
                payload = {
                    "sensor": sensor_name,
                    "temp": temp,
                    "humidity": humidity
                }

                print("Posting payload:", payload)

                response = requests.post(server_url, json=payload)

                if response.status_code == 200:
                    print("✅ Data sent successfully.")
                else:
                    print("❌ Failed to send:", response.status_code, response.text)
            else:
                print("❗ Incomplete data:", line)

        time.sleep(0.5)

    except Exception as e:
        print("Error:", e)
        time.sleep(2)
