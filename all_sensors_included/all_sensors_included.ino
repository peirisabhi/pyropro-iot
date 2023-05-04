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



// const char* ssid = "SLT_FIBRE";
const char* ssid = "Sahan prenando";
// const char* password = "Sheeba@#1212";
const char* password = "sahan123";
const char* mqtt_server = "13.50.105.25";


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(256)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    // digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    // digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(9600);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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
  

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // delay(3000);

   unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  int gasDetected = digitalRead(gasSensorD);
  float gasValue = analogRead(gasSensorA);

  int flameDetected = digitalRead(flameSensor);

  float soilMoistureValue= analogRead(soilMoistureD);
  soilMoistureValue = map(soilMoistureValue,550,0,0,100);


  float bmpTemperature = bmp.readTemperature();
  float bmpPressure = bmp.readPressure();
  float bmpSealevelPressure = bmp.readSealevelPressure();
  float bmpAltitude = bmp.readAltitude();
  float bmpAltitudeWithSealevelPressure = bmp.readAltitude(seaLevelPressure_hPa * 100);


  // Serial.print(F("Humidity: "));
  // Serial.print(h);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(t);
  // Serial.print(F("°C "));
  // Serial.print(f);
  // Serial.print(F("°F  Heat index: "));
  // Serial.print(hic);
  // Serial.print(F("°C "));
  // Serial.print(hif);
  // Serial.print(F("°F"));

  // Serial.print(F(" gasDetected:"));
  // Serial.print(gasDetected);

  // Serial.print(F(" gasValue:"));
  // Serial.print(gasValue);

  // Serial.print(F(" flameDetected:"));
  // Serial.print(flameDetected);

  // Serial.print(F(" soilMoistureValue:"));
  // Serial.print(soilMoistureValue);

  // Serial.print(F(" soilMoistureValue:"));
  // Serial.print(soilMoistureValue);

  // Serial.print(F(" bmpTemperature:"));
  // Serial.print(bmpTemperature);

  // Serial.print(F(" bmpPressure:"));
  // Serial.print(bmpPressure);

  // Serial.print(F(" bmpSealevelPressure:"));
  // Serial.print(bmpSealevelPressure);

  // Serial.print(F(" bmpAltitudeWithSealevelPressure:"));
  // Serial.print(bmpAltitudeWithSealevelPressure);

  // Serial.print(F(" bmpAltitude:"));
  // Serial.println(bmpAltitude);



  DynamicJsonDocument doc(1024);

    doc["temp"] = temperature;
    doc["humi"] = humidity;
    doc["hi"] = heatIndex;
    doc["gas_det"] = gasDetected;
    doc["gas"] = gasValue;
    doc["flame"] = flameDetected;
    doc["sm"] = soilMoistureValue;
    doc["bmp_temp"] = bmpTemperature;
    doc["bmp_pressure"] = bmpPressure;
    doc["bmp_slp"] = bmpSealevelPressure;
    doc["bmp_alti"] = bmpAltitude;
    doc["bmp_alti_slp"] = bmpAltitudeWithSealevelPressure;
    doc["time"] = now;


    char data[256];
    serializeJson(doc, data);
    Serial.print(data);

    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(data);
    client.publish("sensor-data", data);

  }
}