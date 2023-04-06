#include "DHT.h"
#include <ArduinoJson.h>

// DHT
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Flame Sensor
int flameSensor = 4; 

// MQ7 gas sensor
int gasSensorD = 3;
int gasSensorA = 0;

// soil mositure sensor
int soilMoisture = 1;

unsigned long lastMsg = 0;
int value = 0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
dht.begin();

pinMode(flameSensor, INPUT);
pinMode(gasSensorD, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

unsigned long now = millis();

if (now - lastMsg > 500) {
    lastMsg = now;
    ++value;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    int flameDetected = digitalRead(flameSensor);

    int gasDetected = digitalRead(gasSensorD);
    int gasValue = analogRead(gasSensorA);

    int soilMoistureValue = analogRead(analogRead);

    DynamicJsonDocument doc(1024);

    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["time"] = now;
    doc["device_id"] = "device_01";
    doc["flame_detected"] = flameDetected;
    doc["gas_detected"] = gasDetected;
    doc["gas_value"] = gasValue;
    doc["soil_moisture_value"] = soilMoistureValue;

    

    char data[256];
    serializeJson(doc, data);
    Serial.println(data);
    

  }
}
