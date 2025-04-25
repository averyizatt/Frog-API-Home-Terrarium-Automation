#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// --- Wi-Fi Setup ---
const char* ssid = "t";
const char* password = "";

// --- API Endpoint ---
const char* server = "";

// --- Sensor Config ---
#define SENSOR_COUNT 2
const uint8_t dhtPins[SENSOR_COUNT] = {4, 5};  // GPIO4 = White Tree Frog, GPIO5 = Bedroom

const char* sensorNames[SENSOR_COUNT] = {
  "White Tree Frog Terrarium",
  "Bedroom"
};

DHT dhts[SENSOR_COUNT] = {
  DHT(dhtPins[0], DHT11),
  DHT(dhtPins[1], DHT11)
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[BOOT] ESP32-C3 Nano Starting...");

  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
  Serial.print("[WIFI] IP address: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.printf("[INIT] Starting DHT sensor '%s' on GPIO%d...\n", sensorNames[i], dhtPins[i]);
    dhts[i].begin();
  }
}

void loop() {
  Serial.println("\n--- Reading Sensors ---");

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.printf("[%s] Reading data...\n", sensorNames[i]);

    float tempC = dhts[i].readTemperature();
    float humidity = dhts[i].readHumidity();

    if (isnan(tempC) || isnan(humidity)) {
      Serial.printf("[%s] Failed to read from DHT sensor!\n", sensorNames[i]);
      continue;
    }

    float tempF = tempC * 1.8 + 32;

    String payload = "{\"sensor\":\"" + String(sensorNames[i]) +
                     "\",\"temp\":" + String(tempF, 1) +
                     ",\"humidity\":" + String(humidity, 1) + "}";

    Serial.printf("[%s] Temp: %.1f°F, Humidity: %.1f%%\n", sensorNames[i], tempF, humidity);
    Serial.printf("[%s] Sending payload: %s\n", sensorNames[i], payload.c_str());

    std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
    client->setInsecure();  // skip SSL cert validation

    HTTPClient http;
    http.begin(*client, server);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);
    Serial.printf("[%s] HTTP %d\n", sensorNames[i], httpCode);

    http.end();
    delay(250);  // short pause between sensors
  }

  Serial.println("--- Loop complete. Waiting 10 seconds ---\n");
  delay(10000);
}
