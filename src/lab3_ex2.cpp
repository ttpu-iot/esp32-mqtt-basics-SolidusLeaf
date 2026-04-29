#include "Arduino.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "time.h"

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker settings
const char* mqtt_broker = "mqtt.iotserver.uz";  
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";  
const char* mqtt_password = "mqttpass";  

// global declarations
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12

// topics for subscriber
const char* RED = "ttpu/iot/ramiz/led/red";
const char* GREEN = "ttpu/iot/ramiz/led/green";
const char* BLUE = "ttpu/iot/ramiz/led/blue";
const char* YELLOW = "ttpu/iot/ramiz/led/yellow";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// functions
void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectMQTT();
void connectWiFi();
void checkMQTTconnection();
void check_WiFi_connection();
void subscribeToTopics();

void setup(){
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  
  Serial.begin(115200);
  delay(1000);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  configTime(18000, 0, "pool.ntp.org"); 
  // Connect to WiFi
  connectWiFi();
  
  // Setup MQTT
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);

  // Connect to MQTT broker
  connectMQTT();
}

void loop() {
  check_WiFi_connection();
  checkMQTTconnection();

  // Process incoming MQTT messages
  mqtt_client.loop();

}

// Callback function for received MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // 1. Print the prefix and topic
  Serial.println();
  Serial.print("[MQTT] Received on ");
  Serial.print(topic);
  Serial.print(": ");

  // 2. Print the entire payload without a loop
  Serial.write(payload, length); 
  Serial.println(); // Just to start a new line

  JsonDocument doc;
  deserializeJson(doc, payload, length);
  String state = doc["state"];

  // Convert state to a simple boolean for the switch
  bool power = (state == "ON");

  // 3. Route the topic to the correct Pin
  // strcmp returns 0 if the strings match perfectly
  if (!strcmp(topic, RED)) {
    digitalWrite(RED_LED, power);
    Serial.print("[LED] Red LED ->");
    Serial.println(state);
  } 
  else if (!strcmp(topic, GREEN)) {
    digitalWrite(GREEN_LED, power);
    Serial.print("[LED] Green LED ->");
    Serial.println(state);   
  } 
  else if (!strcmp(topic, BLUE)) {
    digitalWrite(BLUE_LED, power);
    Serial.print("[LED] Blue LED ->");
    Serial.println(state);       
  } 
  else if (!strcmp(topic, YELLOW)) {
    digitalWrite(YELLOW_LED, power);
    Serial.print("[LED] Yellow LED ->");
    Serial.println(state); 
  }
  
}

void check_WiFi_connection(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Reconnecting...");
    connectWiFi();
  }
}

void checkMQTTconnection(){
  if (!mqtt_client.connected()) {
    Serial.println("MQTT disconnected! Reconnecting...");
    connectMQTT();
  }
}

// Function to connect to WiFi
void connectWiFi() {
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect/reconnect to MQTT broker
void connectMQTT() {
  while (!mqtt_client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker!");
      subscribeToTopics();
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.println(mqtt_client.state());
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}


void subscribeToTopics(){
  // Subscribe to topic
  mqtt_client.subscribe(RED);
  Serial.print("Subscribed to topic: ");
  Serial.println(RED);

    // Subscribe to topic
  mqtt_client.subscribe(GREEN);
  Serial.print("Subscribed to topic: ");
  Serial.println(GREEN);

  // Subscribe to topic
  mqtt_client.subscribe(BLUE);
  Serial.print("Subscribed to topic: ");
  Serial.println(BLUE);

  // Subscribe to topic
  mqtt_client.subscribe(YELLOW);
  Serial.print("Subscribed to topic: ");
  Serial.println(YELLOW);
}