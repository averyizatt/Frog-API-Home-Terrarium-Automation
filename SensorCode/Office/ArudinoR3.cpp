#include <DHT.h>

// --- Sensor Config ---
#define SENSOR_COUNT 3
const uint8_t dhtPins[SENSOR_COUNT] = {12, 4, 14};  // DHT on pins 12, 4, 14

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

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[BOOT] Elegoo Uno R3 Sensor Node Starting...");

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.print("[INIT] Starting DHT sensor '");
    Serial.print(sensorNames[i]);
    Serial.print("' on Pin ");
    Serial.println(dhtPins[i]);
    dhts[i].begin();
  }
}

void loop() {
  Serial.println("\n--- Reading Sensors ---");

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.print("[");
    Serial.print(sensorNames[i]);
    Serial.println("] Reading data...");

    float tempC = dhts[i].readTemperature();
    float humidity = dhts[i].readHumidity();

    if (isnan(tempC) || isnan(humidity)) {
      Serial.print("[");
      Serial.print(sensorNames[i]);
      Serial.println("] Failed to read from DHT sensor!");
      continue;
    }

    float tempF = tempC * 1.8 + 32;

    // Print as a simple line for server to pick up
    Serial.print("sensor:");
    Serial.print(sensorNames[i]);
    Serial.print(",temp:");
    Serial.print(tempF, 1);
    Serial.print(",humidity:");
    Serial.println(humidity, 1);
    
    delay(250); // Small gap between sensors
  }

  Serial.println("--- Loop complete. Waiting 10 seconds ---\n");
  delay(10000); // Main delay
}
