#include "DHT.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>

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


// buzzer
int buzzer = 4;


// rain sensor
int rainD = 26;

Adafruit_BMP085 bmp;

// wind speed
volatile byte revolutions = 0;

float rpmilli = 0;
float speed = 0;
unsigned long timeold=0;

TaskHandle_t Task1;


const char* ssid = "SLT_FIBRE";
// const char* ssid = "Sahan prenando";
const char* password = "Sheeba@#1212";
// const char* password = "sahan123";
const char* mqtt_server = "13.50.105.25";


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (512)
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



//Task1code: blinks an LED every 1000 ms
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    int flameDetected = digitalRead(flameSensor);

    if (flameDetected == 0) {
      Serial.println("----------------------- Flame detected -------------------");
      publishData(1);
    }

    delay(100);
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
  pinMode(rainD, INPUT);
  pinMode(buzzer, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(27), rpm_fun, RISING);




  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
}

void loop() {


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // delay(3000);

  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    publishData(0);
  }
}


void publishData(int fire) {

  calWind();

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
  int heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  int gasDetected = digitalRead(gasSensorD);
  float gasValue = analogRead(gasSensorA);

  int flameDetected = digitalRead(flameSensor);

  int rainDetected = digitalRead(rainD);

  float soilMoistureValue = analogRead(soilMoistureD);
  soilMoistureValue = map(soilMoistureValue, 550, 0, 0, 100);


  float bmpTemperature = bmp.readTemperature();
  float bmpPressure = bmp.readPressure();
  float bmpSealevelPressure = bmp.readSealevelPressure();
  float bmpAltitude = bmp.readAltitude();
  float bmpAltitudeWithSealevelPressure = bmp.readAltitude(seaLevelPressure_hPa * 100);


  if (gasDetected == 1) {
    gasDetected = 0;
  } else {
    gasDetected = 1;
  }

  if (flameDetected == 1) {
    flameDetected = 0;
  } else {
    flameDetected = 1;
  }


  if (rainDetected == 1) {
    rainDetected = 0;
  } else {
    rainDetected = 1;
  }


  DynamicJsonDocument doc(2054);

  doc["temp"] = roundf(temperature);
  doc["humi"] = humidity;
  doc["hi"] = heatIndex;
  doc["gas_det"] = gasDetected;
  doc["gas"] = gasValue;

  doc["sm"] = soilMoistureValue;
  doc["bp_temp"] = bmpTemperature;
  doc["bp_pressure"] = bmpPressure;
  doc["bp_slp"] = bmpSealevelPressure;
  doc["bp_alti"] = bmpAltitude;
  doc["bp_alti_slp"] = bmpAltitudeWithSealevelPressure;
  doc["rain"] = rainDetected;
  doc["di"] = "D001";
  doc["rpm"] = rpmilli * 60000;
  doc["sp"] = speed;

  if (fire == 1) {
    doc["flame"] = 1;
  } else {
    doc["flame"] = flameDetected;
  }


  char data[256];
  serializeJson(doc, data);
  Serial.println(data);

  // snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  // Serial.print("Publish message: ");
  // Serial.println(data);

  client.publish("sensor-data", data);

  if(fire == 1){
    for(int x = 0; x < 10; x++){
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(100);
    }
  }else{
    digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  }
}



void rpm_fun()
{
  revolutions++;
}



void calWind(){
  // if (revolutions >= 2) { 
    //Update RPM every 20 counts, increase this for better RPM resolution,
    //decrease for faster update

    // calculate the revolutions per milli(second)
    rpmilli = ((float)revolutions)/(millis()-timeold);

    timeold = millis();
    revolutions = 0;

    // WHEELCIRC = 2 * PI * radius (in meters)
    // speed = rpmilli * WHEELCIRC * "milliseconds per hour" / "meters per kilometer"

    // simplify the equation to reduce the number of floating point operations
    // speed = rpmilli * WHEELCIRC * 3600000 / 1000
    // speed = rpmilli * WHEELCIRC * 3600

    speed = rpmilli * 0.6597 * 3600;

    Serial.print("RPM:");
    Serial.print(rpmilli * 60000);
    Serial.print(" Speed:");
    Serial.print(speed);
    Serial.println(" kph");
  // }
}



// float roundNumbers(float var)
// {
//     // 37.66666 * 100 =3766.66
//     // 3766.66 + .5 =3767.16    for rounding off value
//     // then type cast to int so value is 3767
//     // then divided by 100 so the value converted into 37.67
//     float value = (int)(var * 100 + .5);
//     return (float)value / 100;
// }
