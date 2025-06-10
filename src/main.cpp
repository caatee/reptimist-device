#include <Arduino.h>
#include <DHT.h>
#include <config.h>
#include <pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

// Global declarations
DHT dht(DHTPIN, DHTTYPE);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long previousMillis = 0;

void feedStatus() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(true); // read in Fahrenheit
  ds18b20.requestTemperatures();
  float tempF = ds18b20.getTempFByIndex(0);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  Serial.print("Published status successfully: ");
  Serial.print("Relative Humidity: ");
  Serial.print(h);
  Serial.print("%, Temperature: ");
  Serial.print(t);
  Serial.print("°F, Probe Temperature: ");
  Serial.print(tempF);
  Serial.println("°F");

  mqttClient.publish("reptimist/status/humidity", String(h).c_str());
  mqttClient.publish("reptimist/status/temperature", String(t).c_str());
  mqttClient.publish("reptimist/status/probe_temperature", String(tempF).c_str());
}

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
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }
}

void commandHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  // parse as json
  Serial.print("Payload length: ");
  Serial.println(length);
  if (length == 0) {
    Serial.println("No payload received.");
    return;
  }

  String truePayload = "";
  for (unsigned int i = 0; i < length; i++) { // ??? what the fuck
    truePayload += (char)payload[i]; 
  }
  truePayload.trim();
  Serial.print("Payload: ");
  Serial.println(truePayload);

  // parse as JSON object
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, truePayload);
  if (error) {
    Serial.print("bad payload: ");
    Serial.println(error.c_str());
    return;
  }

  if (!doc["command"].isNull()) {
    String command = doc["command"].as<String>();
    Serial.print("Command: ");
    Serial.println(command);

    if (command == "start_misting") {
      
    if (!doc["duration"].isNull()) {
        int duration = doc["duration"].as<int>();
        Serial.print("Misting for ");
        Serial.print(duration);
        Serial.println(" seconds.");
        // TODO: Make mist function for this to work
      } else {
        Serial.println("No duration specified, misting indefinitely.");
        // TODO: Make mist function for this to work
      }
    } else if (command == "set_humidity") {
      if (!doc["humidity"].isNull()) {
        int humidity = doc["humidity"].as<int>();
        Serial.print("Setting humidity to ");
        Serial.print(humidity);
        Serial.println(" %.");
        // TODO: Add code to set the humidity level which needs the misting function but im still waiting on the transistors to come in the mail 
      } else {
        Serial.println("No humidity value specified.");
      }

    } else if (command == "get_status") {
      Serial.println("Getting status...");

      feedStatus();
    } else if (command == "stop_misting") {
      Serial.println("Stopping misting.");

    } else {
      Serial.println("Unknown command.");
    }
  } else {
    Serial.println("No command found in payload.");
  }
}

void connectToMQTT() {
  Serial.println("Connecting to MQTT broker...");

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(commandHandler);
  
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected!");
      // sub to cmds
      mqttClient.subscribe("reptimist/commands");
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
  ds18b20.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= LOOP_INTERVAL) {
    previousMillis = currentMillis;
    feedStatus();
  }

  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
}
