// === CONFIG ===
const base = "/frogtank";

const sensors = [
  { id: "whites", label: "Whites Tree Frog Terrarium" },
  { id: "green", label: "Green Tree Frog Terrarium" },
  { id: "office", label: "Office Sensor" },
  { id: "other", label: "Other Sensor" },
  { id: "red knee", label: "Red Knee Tarantula" },
  { id: "avicularia avicularia", label: "Avicularia Avicularia Tarantula" },
  { id: "living room", label: "Living Room" },
  { id: "bedroom", label: "Bedroom" },
  { id: "3d printer", label: "3D Printer" }
];

const comingSoon = [
  { id: "coming-1", label: "Coming Soon 1" },
  { id: "coming-2", label: "Coming Soon 2" },
  { id: "coming-3", label: "Coming Soon 3" }
];

const grid = document.getElementById("sensor-grid");
const miniCharts = {};
let popupSensorId = "";
let popupChart = null;
let currentScale = "hour";
let currentFilter = "all";

// === Build Dashboard Tiles ===
[...sensors, ...comingSoon].forEach(sensor => {
  const tile = document.createElement("div");
  tile.className = "tile";
  tile.setAttribute("data-sensor-id", sensor.id);
  tile.innerHTML = `
    <span id="alert-${sensor.id}" class="alert-icon">•</span>
    <h2>${sensor.label}</h2>
    <div class="reading" id="info-${sensor.id}">Loading...</div>
    <canvas id="chart-${sensor.id}"></canvas>
    <div class="last-update" id="last-${sensor.id}"></div>
  `;
  grid.appendChild(tile);
});

// === Dashboard Functions ===
function formatFullTime(ts) {
  const date = new Date(ts);
  return date.toLocaleString("en-US", {
    month: "short",
    day: "numeric",
    hour: "numeric",
    minute: "numeric",
    hour12: true
  });
}

function updateSensor(sensor) {
  fetch(`${base}/sensor/${sensor.id}`)
    .then(res => res.json())
    .then(data => {
      const info = document.getElementById(`info-${sensor.id}`);
      const alertIcon = document.getElementById(`alert-${sensor.id}`);
      const updated = document.getElementById(`last-${sensor.id}`);

      if (data.error) {
        info.innerHTML = "<span class='offline'>No data available</span>";
        alertIcon.className = "alert-icon bad";
        return;
      }

      const temp = parseFloat(data.temp);
      const humidity = parseFloat(data.humidity);
      const now = Date.now();
      const sensorTime = new Date(data.time).getTime();
      const isOnline = (now - sensorTime) < 10 * 60 * 1000;  // 10 minutes

      let status = "good";
      if (!isOnline) status = "bad";

      alertIcon.className = `alert-icon ${status}`;
      alertIcon.textContent = "●";

      const formatted = formatFullTime(sensorTime);

      info.innerHTML = isOnline
        ? `Temp: ${temp}°F | Humidity: ${humidity}%`
        : `<span class="offline">Offline since ${formatted}</span>`;
      updated.textContent = `Last updated: ${formatted}`;
    });
}

function refreshAll() {
  sensors.forEach(sensor => {
    updateSensor(sensor);
    fetch(`${base}/sensor/${sensor.id}-log`)
      .then(res => res.text())
      .then(csv => {
        const rows = csv.trim().split("\n").slice(-20);
        const labels = [], temp = [], hum = [];

        rows.forEach(r => {
          const [time, , t, h] = r.split(",");
          labels.push(new Date(time).toLocaleTimeString("en-US", {
            hour: 'numeric', minute: 'numeric', hour12: true
          }));
          temp.push(parseFloat(t));
          hum.push(parseFloat(h));
        });

        const ctx = document.getElementById(`chart-${sensor.id}`).getContext('2d');
        if (miniCharts[sensor.id]) miniCharts[sensor.id].destroy();

        miniCharts[sensor.id] = new Chart(ctx, {
          type: "line",
          data: {
            labels: labels,
            datasets: [
              { label: "Temp (°F)", data: temp, borderColor: "orange", fill: false },
              { label: "Humidity (%)", data: hum, borderColor: "lightblue", fill: false }
            ]
          },
          options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: false } },
            scales: { x: { ticks: { color: "#aaa" } }, y: { ticks: { color: "#eee" } } }
          }
        });
      });
  });

  comingSoon.forEach(sensor => {
    const ctx = document.getElementById(`chart-${sensor.id}`).getContext('2d');
    new Chart(ctx, {
      type: "line",
      data: {
        labels: Array(10).fill('T'),
        datasets: [
          { data: Array(10).fill(null), borderColor: "#444", borderDash: [5, 5], fill: false },
          { data: Array(10).fill(null), borderColor: "#555", borderDash: [5, 5], fill: false }
        ]
      },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } } }
    });
  });
}

setInterval(refreshAll, 15000);
refreshAll();

// === Popup Graph Functions ===
document.querySelectorAll(".tile").forEach(tile => {
  tile.addEventListener("click", () => {
    popupSensorId = tile.getAttribute("data-sensor-id");
    openPopup(popupSensorId);
  });
});

function openPopup(sensorId) {
  document.getElementById("popup").style.display = "flex";
  document.getElementById("popup-title").innerText = sensors.find(s => s.id === sensorId)?.label || "Sensor Graph";
  currentScale = "hour"; // reset
  currentFilter = "all"; // reset
  document.getElementById("filter-select").value = "all";
  loadPopupGraph(sensorId);
}

function closePopup() {
  document.getElementById("popup").style.display = "none";
  if (popupChart) popupChart.destroy();
}

function setTimescale(scale) {
  currentScale = scale;
  loadPopupGraph(popupSensorId);
}

function setFilter() {
  currentFilter = document.getElementById("filter-select").value;
  loadPopupGraph(popupSensorId);
}

function loadPopupGraph(sensorId) {
  fetch(`${base}/sensor/${sensorId}-log`)
    .then(res => res.text())
    .then(csv => {
      const rows = csv.trim().split("\n");
      const labels = [], temp = [], hum = [], lux = [];

      const now = Date.now();
      let cutoff = now;
      if (currentScale === "hour") cutoff -= 1 * 60 * 60 * 1000;
      else if (currentScale === "day") cutoff -= 24 * 60 * 60 * 1000;
      else if (currentScale === "week") cutoff -= 7 * 24 * 60 * 60 * 1000;
      else if (currentScale === "month") cutoff -= 30 * 24 * 60 * 60 * 1000;

      let lastIncludedTime = 0;
      const spacing = currentScale === "hour" ? 2 * 60 * 1000 : 
                      currentScale === "day" ? 15 * 60 * 1000 : 
                      currentScale === "week" ? 2 * 60 * 60 * 1000 :
                      24 * 60 * 60 * 1000;

      rows.forEach(r => {
        const parts = r.split(",");
        const time = parts[0];
        const ts = new Date(time).getTime();
        const hours = new Date(ts).getHours();

        if (ts >= cutoff) {
          let passFilter = true;
          if (currentFilter === "day" && (hours < 7 || hours >= 19)) passFilter = false;
          if (currentFilter === "night" && (hours >= 7 && hours < 19)) passFilter = false;

          if (passFilter && (ts - lastIncludedTime >= spacing)) {
            labels.push(new Date(time).toLocaleString("en-US", {
              month: "short",
              day: "numeric",
              hour: "numeric",
              minute: "numeric",
              hour12: true
            }));
            temp.push(parseFloat(parts[2]));
            hum.push(parseFloat(parts[3]));
            if (parts.length > 4) lux.push(parseFloat(parts[4]));
            lastIncludedTime = ts;
          }
        }
      });

      const ctx = document.getElementById("popup-chart").getContext('2d');
      if (popupChart) popupChart.destroy();

      popupChart = new Chart(ctx, {
        type: "line",
        data: {
          labels: labels,
          datasets: [
            { label: "Temp (°F)", data: temp, borderColor: "orange", fill: false },
            { label: "Humidity (%)", data: hum, borderColor: "lightblue", fill: false },
            ...(lux.length ? [{ label: "Lux (lx)", data: lux, borderColor: "yellow", fill: false }] : [])
          ]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          plugins: { legend: { labels: { color: "#eee" } } },
          scales: { x: { ticks: { color: "#aaa" } }, y: { ticks: { color: "#eee" } } }
        }
      });
    });
}

