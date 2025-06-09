#include <Arduino.h>
#include <DHT.h>
#include <config.h>
#include <pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Global declarations
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void initializeWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("got IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }
}

void connectToMQTT() {
  Serial.println("Connecting to MQTT broker...");

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  delay(1000);
}

void setup() {
  Serial.begin(9600);

  Serial.println("\n\n");
  Serial.println("             ____");
  Serial.println("            / . .\\");
  Serial.println("            \\  ---<");
  Serial.println("             \\  /");
  Serial.println("   __________/ /");
  Serial.println("-=:___________/\n");

  Serial.println("ReptiMist v1.0 initializing...");

  initializeWiFi();

  connectToMQTT();

  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(true); // read in Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" °F");
  }

  delay(2000);
}
