#include <Arduino.h>


/***************************
https://www.bydeezoom.com
****************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define DHTPIN 15 // pin for Dustation Devkit v1.3 กำหนดพินไหนก็ได้แต่บอร์ดนี้ตั้งไว้ให้ 15

float t;
float h;

#include "PMS.h"
PMS pms(Serial2); // Serial2 is GPIO17(TX) GPIO16(RX)
PMS::DATA data;
int pm1;
int pm25;
int pm10;

// #define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
// #define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Replace the next variables with your SSID/Password combination
const char *ssid = "baraoom";
const char *password = "12345678";

// Add your MQTT Broker IP address, example:
const char *mqtt_server = "183.90.170.182"; // for EDG
// const char *mqtt_server = "iot.nipa.cloud";// for PE
const char *mqttPassword = "";
// const char *mqttUser = "K2zKWoBP42onaVGDCElL"; // access token EDGE AirQ_1
// const char *mqttUser = "ZPP6cFxZsV380G6Q2jlr"; // access token EDGE AirQ_2
// const char *mqttUser = "9C34caIGO6kmTVlQgMO1"; // access token EDGE AirQ_3
// const char *mqttUser = "009VfpQbSahyGMem6ui3"; // access token EDGE AirQ_4
 const char *mqttUser = "kU7T4HaCJhJIr3n5CvmJ"; // access token EDGE AirQ_5

const char *topic = "v1/devices/me/telemetry";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

// LED Pin
const int ledPin = 4; // LED on board

void am2302()
{

  // read temperature and humidity
  t = dht.readTemperature();
  h = dht.readHumidity();
  if (isnan(h) || isnan(t))
  {
    Serial.println("AM2302 Failed");
  }
  Serial.print("Temp : ");
  Serial.println(t);
  Serial.print("Humid: ");
  Serial.println(h);
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    // WiFi.reconnect();
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      Serial.println("connected");
      // Subscribe
      // client.subscribe("esp32/output");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      WiFi.reconnect();
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial2.begin(9600);
  Serial.begin(9600);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {

      Serial.println("connected");
    }
    else
    {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  pinMode(ledPin, OUTPUT);
  char json[200];

}

void loop()
{
  // delay(5000);

  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

  if (pms.read(data))
  {
    pm1 = data.PM_AE_UG_1_0;
    pm25 = data.PM_AE_UG_2_5;
    pm10 = data.PM_AE_UG_10_0;

    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_10_0);

    Serial.println();

    am2302();  
    long now = millis();
  if (now - lastMsg > 1000) // set up time time interval
  {
    lastMsg = now;

    char json[200];

    sprintf(json, "{\"PM25\":\"%d\", \"PM10\":\"%d\", \"PM1\":\"%d\",\"temperature\":\"%.2f\",\"humidity\":\"%.2f\"}",
            pm25, pm10, pm1, t, h); // 4 for 4   2 for all
    client.publish(topic, json);
    Serial.print(json);
    Serial.println("sentdata");
  }
  }

}
