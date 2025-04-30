import time
from pathlib import Path
from random import uniform

logdir = Path("/home/thefrogpit/frog-api/logs")
logdir.mkdir(parents=True, exist_ok=True)

sensors = {
    "green": "Green Tree Frog Terrarium",
    "office": "Office Sensor",
    "other": "Other Sensor"
}

now = int(time.time())

for key in sensors:
    lines = []
    for i in range(100):
        ts = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(now - (100 - i) * 60))
        temp = round(uniform(40.0, 50.0), 1)     # Low fake Fahrenheit
        humidity = round(uniform(10.0, 25.0), 1) # Low fake Humidity
        lines.append(f"{ts},{key},{temp},{humidity}")
    
    with open(logdir / f"{key}.csv", "w") as f:
        f.write("\n".join(lines))
