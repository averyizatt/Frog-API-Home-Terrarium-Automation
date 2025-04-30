#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// --- Wi-Fi Setup ---
const char* ssid = "thefrogpit";
const char* password = "";

// --- API Endpoint ---
const char* server = "https://averyizatt.com/frogtank/api/sensor";

// --- Sensor Setup ---
#define SENSOR_COUNT 3
const uint8_t dhtPins[SENSOR_COUNT] = {12, 4, 14};  // Pinktoe, Red Knee, Office
const char* sensorNames[SENSOR_COUNT] = {
  "Avicularia Avicularia",  // Pinktoe
  "Red Knee",               // Red Knee
  "Office Sensor"           // Office Temp
};

DHT dhts[SENSOR_COUNT] = {
  DHT(dhtPins[0], DHT11),
  DHT(dhtPins[1], DHT11),
  DHT(dhtPins[2], DHT11)
};

// --- Wi-Fi Auto Recovery ---
const unsigned long WIFI_TIMEOUT_MS = 10000;
const unsigned long WIFI_RECOVER_MS = 30000;
unsigned long wifiDisconnectedSince = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[BOOT] ESP32 Office Node Starting...");

  connectWiFi();

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.printf("[INIT] Starting DHT sensor '%s' on GPIO%d\n", sensorNames[i], dhtPins[i]);
    dhts[i].begin();
  }
}

void loop() {
  autoReconnectWiFi();

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

    Serial.printf("[%s] Sending payload: %s\n", sensorNames[i], payload.c_str());

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();  // disable SSL cert check

      HTTPClient http;
      http.begin(client, server);
      http.addHeader("Content-Type", "application/json");

      int code = http.POST(payload);
      String response = http.getString();
      Serial.printf("[%s] HTTP %d - %s\n", sensorNames[i], code, response.c_str());
      http.end();
    } else {
      Serial.println("[WARN] Wi-Fi not connected, skipping POST.");
    }

    delay(250);
  }

  Serial.println("--- Loop complete. Waiting 10 seconds ---\n");
  delay(10000);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WIFI] Connected: %s\n", WiFi.localIP().toString().c_str());
    wifiDisconnectedSince = 0;
  } else {
    Serial.println("\n[WIFI] Failed.");
    wifiDisconnectedSince = millis();
  }
}

void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiDisconnectedSince == 0) {
      wifiDisconnectedSince = millis();
    }
    if (millis() - wifiDisconnectedSince > WIFI_RECOVER_MS) {
      Serial.println("[WIFI] Rebooting due to lost connection...");
      ESP.restart();
    }
    connectWiFi();
  } else {
    wifiDisconnectedSince = 0;
  }
}
