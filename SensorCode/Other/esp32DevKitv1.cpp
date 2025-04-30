#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Wi-Fi Credentials
const char* ssid = "thefrogpit";
const char* password = "";
const char* server = "https://averyizatt.com/frogtank/api/sensor";  // Add your server URL

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT11 Config
#define DHTTYPE DHT11
DHT dht1(14, DHTTYPE);  // Frog Tank
DHT dht2(27, DHTTYPE);  // Plant Tank
DHT dht3(26, DHTTYPE);  // Living Room

// Light Sensors
BH1750 light1(0x23);  // Frog Tank
BH1750 light2(0x5C);  // Plant Tank

// TDS Sensor
#define TDS_PIN 34
float readTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * 3.3 / 4095.0;
  float tds = (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * 0.5;
  return tds;
}

// Ultrasonic Sensor
#define TRIG_PIN 25
#define ECHO_PIN 33
float tank_full_cm = 15.0;
float getWaterLevelPercent() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = duration * 0.0343 / 2.0;
  float percent = max(0.0, min(100.0, 100.0 - ((distance / tank_full_cm) * 100.0)));
  return percent;
}

// HTTPS POST
bool postData(String sensor, float temp, float hum, float lux, float tds = -1, float level = -1) {
  std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  https.begin(*client, server);
  https.addHeader("Content-Type", "application/json");

  String json = "{\"sensor\":\"" + sensor + "\",\"temp\":" + temp + ",\"humidity\":" + hum;
  if (lux >= 0) json += ",\"lux\":" + String(lux);
  if (tds >= 0) json += ",\"tds\":" + String(tds);
  if (level >= 0) json += ",\"water_level\":" + String(level);
  json += "}";

  int code = https.POST(json);
  https.end();
  return (code == 200);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Wire.begin(21, 22);
  dht1.begin(); dht2.begin(); dht3.begin();
  light1.begin(); light2.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void loop() {
  float tds = readTDS();
  float water_level = getWaterLevelPercent();

  float gt_temp = dht1.readTemperature(true);
  float gt_hum = dht1.readHumidity();
  float gt_lux = light1.readLightLevel();

  float pt_temp = dht2.readTemperature(true);
  float pt_hum = dht2.readHumidity();
  float pt_lux = light2.readLightLevel();

  float lr_temp = dht3.readTemperature(true);
  float lr_hum = dht3.readHumidity();

  bool postSuccess = true;
  postSuccess &= postData("Green Tree Frog", gt_temp, gt_hum, gt_lux);
  delay(250);
  postSuccess &= postData("Plant Tank", pt_temp, pt_hum, pt_lux);
  delay(250);
  postSuccess &= postData("Living Room", lr_temp, lr_hum, -1, tds, water_level);
  delay(250);

  // OLED Output
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  display.println("Green Tree Frog:");
  display.printf("T: %.1fF H: %.1f%%\n", gt_temp, gt_hum);
  display.printf("Lux: %.1f\n\n", gt_lux);

  display.println("Plant Tank:");
  display.printf("T: %.1fF H: %.1f%%\n", pt_temp, pt_hum);
  display.printf("Lux: %.1f\n\n", pt_lux);

  display.println("Living Room:");
  display.printf("T: %.1fF H: %.1f%%\n", lr_temp, lr_hum);
  display.printf("TDS: %.1f ppm\n", tds);
  display.printf("Water: %.1f%%\n", water_level);

  display.setCursor(90, 56);
  display.println(postSuccess ? "OK" : "FAIL");
  display.display();

  delay(10000);
}
/*

Function	        GPIO Pin	            Notes
DHT - Frog Tank	    GPIO 14	                 Safe input pin
DHT - Plant Tank	GPIO 27	
DHT - Living Room	GPIO 26	
TDS Sensor	        GPIO 34	ADC1,           input only
BH1750 SDA	        GPIO 21	                Standard I2C SDA
BH1750 SCL	        GPIO 22	                Standard I2C SCL
OLED Display I2C	Shared with BH1750	       SDA=21, SCL=22
Ultrasonic TRIG    	GPIO 25	                  Output
Ultrasonic ECHO	    GPIO 33             	Input



*/