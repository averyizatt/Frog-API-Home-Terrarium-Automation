#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <CQRobotTDS.h>

// --- Wi-Fi Credentials ---
const char* ssid = "thefrogpit";
const char* password = "";
const char* server = "https://averyizatt.com/frogtank/api/sensor";

// --- Sensor Pins ---
#define DHT_PIN_LIVING_ROOM 4
#define DHT_PIN_GREEN_FROG 19
#define TDS_PIN 35
#define SDA_PIN 21
#define SCL_PIN 22

// --- Display ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- DHT Setup ---
#define DHTTYPE DHT11
DHT dhtLivingRoom(DHT_PIN_LIVING_ROOM, DHTTYPE);
DHT dhtGreenFrog(DHT_PIN_GREEN_FROG, DHTTYPE);

// --- TDS Sensor Setup ---
CQRobotTDS tdsSensor(TDS_PIN, 3.3);  // initialized here

// --- Wi-Fi Recovery Timers ---
const unsigned long WIFI_TIMEOUT_MS = 10000;
const unsigned long WIFI_RECOVER_MS = 30000;
unsigned long wifiDisconnectedSince = 0;

bool postSuccess = false;

// Forward declare postSensor before loop()
void postSensor(String name, float temp, float hum, float tds = -1);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[BOOT] ESP32 Terrarium Monitor");

  Wire.begin(SDA_PIN, SCL_PIN);
  dhtLivingRoom.begin();
  dhtGreenFrog.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // TDS config (no .begin())
  tdsSensor.setAdcRange(4096);
  tdsSensor.setTemperature(26.7); // 80°F in Celsius

  connectWiFi();
}

void loop() {
  autoReconnectWiFi();

  float tempLivingRoom = dhtLivingRoom.readTemperature(true);
  float humLivingRoom = dhtLivingRoom.readHumidity();
  float tempGreenFrog = dhtGreenFrog.readTemperature(true);
  float humGreenFrog = dhtGreenFrog.readHumidity();

  tdsSensor.update();
  float tdsValue = tdsSensor.getTdsValue();

  Serial.printf("[READ] Living Room: %.1f°F %.1f%%\n", tempLivingRoom, humLivingRoom);
  Serial.printf("[READ] Green Tree Frog Terrarium: %.1f°F %.1f%%\n", tempGreenFrog, humGreenFrog);
  Serial.printf("[READ] TDS: %.1f ppm\n", tdsValue);

  postSensor("Living Room", tempLivingRoom, humLivingRoom);
  delay(250);
  postSensor("Green Tree Frog Terrarium", tempGreenFrog, humGreenFrog);
  delay(250);
  postSensor("Aquarium", -1, -1, tdsValue);
  delay(250);

  updateDisplay(tempLivingRoom, humLivingRoom, tempGreenFrog, humGreenFrog, tdsValue);
  delay(10000);
}

// --- Sensor Post ---
void postSensor(String name, float temp, float hum, float tds) {
  String payload = "{\"sensor\":\"" + name + "\"";
  if (temp >= 0) payload += ",\"temp\":" + String(temp, 1);
  if (hum >= 0)  payload += ",\"humidity\":" + String(hum, 1);
  if (tds >= 0)  payload += ",\"tds\":" + String(tds, 1);
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

// --- Display ---
void updateDisplay(float tp, float hp, float tf, float hf, float tds) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Living Room:");
  display.printf("T: %.1f°F H: %.1f%%\n", tp, hp);
  display.println("Green Frog:");
  display.printf("T: %.1f°F H: %.1f%%\n", tf, hf);
  display.printf("TDS: %.1f ppm\n", tds);
  display.println(postSuccess ? "POSTED" : "FAILED");
  display.display();
}

// --- Wi-Fi ---
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
