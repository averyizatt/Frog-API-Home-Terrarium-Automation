#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Wi-Fi Credentials ---
const char* ssid = "thefrogpit";
const char* password = "";
const char* server = "https://averyizatt.com/frogtank/api/sensor";

// --- Sensor Pins ---
#define DHT_PIN_PLANT 4      // DHT11 - Plant Tank
#define DHT_PIN_FROG 19      // DHT11 - Green Tree Frog
#define TDS_PIN 34           // Analog - TDS Sensor
#define SDA_PIN 21           // OLED I2C SDA
#define SCL_PIN 22           // OLED I2C SCL

// --- Display ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- DHT Setup ---
#define DHTTYPE DHT11
DHT dhtPlant(DHT_PIN_PLANT, DHTTYPE);
DHT dhtFrog(DHT_PIN_FROG, DHTTYPE);

// --- Wi-Fi Recovery Timers ---
const unsigned long WIFI_TIMEOUT_MS = 10000;
const unsigned long WIFI_RECOVER_MS = 30000;
unsigned long wifiDisconnectedSince = 0;

bool postSuccess = false;

void postSensor(String name, float temp, float hum, float tds = -1);



void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[BOOT] ESP32 Terrarium Monitor");

  Wire.begin(SDA_PIN, SCL_PIN);
  dhtPlant.begin();
  dhtFrog.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  connectWiFi();
}

void loop() {
  autoReconnectWiFi();

  float tempPlant = dhtPlant.readTemperature(true);
  float humPlant = dhtPlant.readHumidity();
  float tempFrog = dhtFrog.readTemperature(true);
  float humFrog = dhtFrog.readHumidity();
  float tds = readTDS();

  Serial.printf("[READ] Plant Tank: %.1fF %.1f%%\n", tempPlant, humPlant);
  Serial.printf("[READ] Green Frog: %.1fF %.1f%%\n", tempFrog, humFrog);
  Serial.printf("[READ] TDS: %.1f ppm\n", tds);

  postSensor("Plant Tank", tempPlant, humPlant);
  delay(250);
  postSensor("Green Tree Frog", tempFrog, humFrog);
  delay(250);
  postSensor("Aquarium", -1, -1, tds);
  delay(250);

  updateDisplay(tempPlant, humPlant, tempFrog, humFrog, tds);
  delay(10000);
}

void postSensor(String name, float temp, float hum, float tds) {
  String payload = "{\"sensor\":\"" + name + "\"";
  if (temp >= 0) payload += ",\"temp\":" + String(temp, 1);
  if (hum >= 0) payload += ",\"humidity\":" + String(hum, 1);
  if (tds >= 0) payload += ",\"tds\":" + String(tds, 1);
  payload += "}";

  Serial.printf("[POST] %s\n", payload.c_str());

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, server);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);
    Serial.printf("[HTTP] Code: %d\n", code);
    postSuccess = (code == 200);
    http.end();
  } else {
    Serial.println("[WARN] Wi-Fi not connected.");
  }
}

float readTDS() {
  int raw = analogRead(TDS_PIN);
  float voltage = raw * 3.3 / 4095.0;
  float tds = (133.42 * voltage * voltage * voltage
              - 255.86 * voltage * voltage
              + 857.39 * voltage) * 0.5;
  return tds;
}

void updateDisplay(float tp, float hp, float tf, float hf, float tds) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Plant Tank:");
  display.printf("T: %.1fF H: %.1f%%\n", tp, hp);
  display.println("Green Frog:");
  display.printf("T: %.1fF H: %.1f%%\n", tf, hf);
  display.printf("TDS: %.1f ppm\n", tds);
  display.println(postSuccess ? "POSTED" : "FAILED");
  display.display();
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
    if (wifiDisconnectedSince == 0) wifiDisconnectedSince = millis();
    if (millis() - wifiDisconnectedSince > WIFI_RECOVER_MS) ESP.restart();
    connectWiFi();
  } else {
    wifiDisconnectedSince = 0;
  }
}
