#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>


/*
Includes:
Ultrasonic sensor
3x DHT11 
2x BH1750
TDS sensor
Wi-Fi
HTTPS POST
Oled display


*/

// === Wi-Fi Credentials ===
const char* ssid = "thefrogpit";
const char* password = "";
const char* server = "";

// === OLED Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === DHT Config ===
#define DHTTYPE DHT11
DHT dht1(1, DHTTYPE);  // GPIO1 = Green Tree Frog
DHT dht2(2, DHTTYPE);  // GPIO2 = Plant Tank
DHT dht3(3, DHTTYPE);  // GPIO3 = Living Room

// === Light Sensors (BH1750) ===
BH1750 light1(0x23);  // Green Tree Frog
BH1750 light2(0x5C);  // Plant Tank

// === TDS Sensor (CQRobot) ===
#define TDS_PIN 4  // GPIO4 (ADC1)
float readTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * 3.3 / 4095.0;
  float tds = (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * 0.5;
  return tds;
}

// === Ultrasonic Sensor ===
#define TRIG_PIN 5
#define ECHO_PIN 6
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

// === HTTPS POST Function ===
void postData(String sensor, float temp, float hum, float lux, float tds = -1, float level = -1) {
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

  https.POST(json);
  https.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Wire.begin();
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

  // Send data to server
  postData("Green Tree Frog", gt_temp, gt_hum, gt_lux);
  delay(250);
  postData("Plant Tank", pt_temp, pt_hum, pt_lux);
  delay(250);
  postData("Living Room", lr_temp, lr_hum, -1, tds, water_level);
  delay(250);





 // --- Update Display ---
 tft.fillScreen(ST77XX_BLACK);  // Full black background
  
  
 uint16_t titleColor, tempColor, humColor, luxColor, green;
 
 if (lux > 10) {
   titleColor = ST77XX_WHITE;
   tempColor = ST77XX_BLUE;
   humColor = ST77XX_YELLOW;
   luxColor = 0x7D7C;
   green = ST77XX_GREEN;
 ;
 } else {
   green = 
   titleColor = 0x4208;  
   tempColor = 0x4208;
   humColor = 0x4208;
   luxColor = 0x4208;
 
   }
   
   tft.setTextSize(1);             // SMALL clean text

   
// Title
tft.setCursor(30, 5);
tft.setTextColor(titleColor);
tft.println("Green Frog Tank");

// Frog Tank Data
tft.setCursor(20, 15);
tft.setTextColor(tempColor);
tft.printf("Temp: %.1fF", gt_temp[0]);

tft.setCursor(20, 25);
tft.setTextColor(humColor);
tft.printf("Hum : %.1f%%", gt_hum[0]);

tft.setCursor(20, 35);
tft.setTextColor(luxColor);
tft.printf("Lux  : %.1f lx", gt_lux[0]);

tft.setCursor(30, 45);
tft.setTextColor(titleColor);
tft.println("Aquarium");

tft.setCursor(20, 55);
tft.setTextColor(tempColor);
tft.printf("TDS: %.1fF", tds[1]);

tft.setCursor(30, 45);
tft.setTextColor(titleColor);
tft.println("Living Room");

tft.setCursor(20, 15);
tft.setTextColor(tempColor);
tft.printf("Temp: %.1fF", lr_temp[0]);

tft.setCursor(20, 15);
tft.setTextColor(tempColor);
tft.printf("Hum: %.1fF", lr_hum[0]);

// HTTP POST Confirmation
if (postSuccess) {
  tft.setCursor(90, 60);
  tft.setTextColor(green);
  tft.println("POSTED!");
} else {
      tft.setCursor(90, 65);
  tft.setTextColor(tempColor);
  tft.println("FAILED!");
}






  // // Display TDS + water level on OLED
  // display.clearDisplay();
  // display.setCursor(20, 5);
  // display.setTextSize(1);
  // display.println("TDS Monitor");

  // display.setTextSize(2);
  // display.setCursor(0, 16);
  // display.print(tds, 1);
  // display.println(" ppm");



  // display.setTextSize(2);
  // display.setCursor(0, 16);
  // display.print(gt_hum, 1);
  // display.print(gt_temp, 1);

  // display.display();


  delay(10000);
}
