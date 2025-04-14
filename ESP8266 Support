#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// --- Wi-Fi Setup ---
const char* ssid = "t";
const char* password = "";

// --- API Endpoint ---
const char* server = "";

// --- Sensor Config ---
#define SENSOR_COUNT 3
const uint8_t dhtPins[SENSOR_COUNT] = {12, 4, 14};  // GPIO12, GPIO4, GPIO14

const char* sensorNames[SENSOR_COUNT] = {
  "Avicularia Avicularia",  // pinktoe
  "Red Knee",               // red_knee
  "Office Sensor"           // office_temp
};

DHT dhts[SENSOR_COUNT] = {
  DHT(dhtPins[0], DHT11),
  DHT(dhtPins[1], DHT11),
  DHT(dhtPins[2], DHT11)
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[BOOT] Starting up...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
  Serial.print("[WIFI] IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize DHT sensors
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

    Serial.printf("[%s] Temp: %.1f°F, Humidity: %.1f%%\n", sensorNames[i], tempF, humidity);

    String payload = "{\"sensor\":\"" + String(sensorNames[i]) +
                     "\",\"temp\":" + String(tempF, 1) +
                     ",\"humidity\":" + String(humidity, 1) + "}";

    Serial.printf("[%s] Sending payload: %s\n", sensorNames[i], payload.c_str());

    // Secure HTTPS connection per sensor
    std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
    client->setInsecure();  // disable SSL cert check for dev/testing

    HTTPClient http;
    http.begin(*client, server);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);
    String response = http.getString();

    Serial.printf("[%s] HTTP %d - %s\n", sensorNames[i], httpCode, response.c_str());

    http.end();
    delay(250);  // short pause between sensors
  }

  Serial.println("--- Loop complete. Waiting 10 seconds ---\n");
  delay(10000);  // main loop delay
}
