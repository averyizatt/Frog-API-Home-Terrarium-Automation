#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Define the data pin
#define DHTPIN 4       // Using GPIO 4 on your ESP32-C3
#define DHTTYPE DHT11  // Change this if using a DHT22 or DHT21

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("DHT11 reading on ESP32-C3 (Temp in Fahrenheit)");
}

void loop() {
  delay(2000); // Wait between readings

  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();     // Read in Celsius
  float tempF = dht.readTemperature(true); // Read in Fahrenheit

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");

  Serial.print("Temperature: ");
  Serial.print(tempF);
  Serial.println(" °F");
}
