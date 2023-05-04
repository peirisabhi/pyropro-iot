#include "DHT.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <PubSubClient.h>

// define the sea level pressure
#define seaLevelPressure_hPa 1013.25

// DHT11 Sensor
#define DHTPIN 33     
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);


// MQ7 gas sensor
int gasSensorD = 32;
int gasSensorA = 35;


// Flame Sensor
int flameSensor = 34; 


// Soil moisture sensor
int soilMoistureD = 25;



Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  if (!bmp.begin()) {
    Serial.println("BMP180 Not Found. CHECK CIRCUIT!");
    while (1) {}
  }

  pinMode(gasSensorD, INPUT);
  pinMode(flameSensor, INPUT);
}

void loop() {
  
  delay(1000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  int gasDetected = digitalRead(gasSensorD);
    int gasValue = analogRead(gasSensorA);

    int flameDetected = digitalRead(flameSensor);

  int soilMoistureValue= analogRead(soilMoistureD);
  soilMoistureValue = map(soilMoistureValue,550,0,0,100);


  int bmpTemperature = bmp.readTemperature();
  int bmpPressure = bmp.readPressure();
  int bmpSealevelPressure = bmp.readSealevelPressure();
  int bmpAltitude = bmp.readAltitude();
  int bmpAltitudeWithSealevelPressure = bmp.readAltitude(seaLevelPressure_hPa * 100);


  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.print(F("°F"));

  Serial.print(F(" gasDetected:"));
  Serial.print(gasDetected);

  Serial.print(F(" gasValue:"));
  Serial.print(gasValue);

  Serial.print(F(" flameDetected:"));
  Serial.print(flameDetected);

  Serial.print(F(" soilMoistureValue:"));
  Serial.print(soilMoistureValue);

  Serial.print(F(" soilMoistureValue:"));
  Serial.print(soilMoistureValue);

  Serial.print(F(" bmpTemperature:"));
  Serial.print(bmpTemperature);

  Serial.print(F(" bmpPressure:"));
  Serial.print(bmpPressure);

  Serial.print(F(" bmpSealevelPressure:"));
  Serial.print(bmpSealevelPressure);

  Serial.print(F(" bmpAltitudeWithSealevelPressure:"));
  Serial.print(bmpAltitudeWithSealevelPressure);

  Serial.print(F(" bmpAltitude:"));
  Serial.println(bmpAltitude);
}