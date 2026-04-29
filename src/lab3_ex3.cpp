#include "Arduino.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "time.h"

//lcd part
#include "Wire.h"
#include "hd44780.h"
#include "hd44780ioClass/hd44780_I2Cexp.h"

// LCD Configuration
hd44780_I2Cexp lcd;  // Auto-detect I2C address
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

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


#define BUTTON 25
const char* topic_button = "ttpu/iot/ramiz/events/button";

// topics for subscriber
const char* RED = "ttpu/iot/ramiz/led/red";
const char* GREEN = "ttpu/iot/ramiz/led/green";
const char* BLUE = "ttpu/iot/ramiz/led/blue";
const char* YELLOW = "ttpu/iot/ramiz/led/yellow";
const char* DISPLAY_TOPIC = "ttpu/iot/ramiz/display"; 

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// functions
void mqttCallback(char* topic, byte* payload, unsigned int length);
String parserForLeds(byte* payload, unsigned int length);
void parseMessageDisplay(byte* payload, unsigned int length);
void connectMQTT();
void connectWiFi();
void checkMQTTconnection();
void check_WiFi_connection();
void subscribeToTopics();
void theButton();

void setup(){
  Serial.begin(9600);
  delay(1000);
   pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  pinMode(BUTTON, INPUT);



  configTime(18000, 0, "pool.ntp.org"); // UTC+5 for Tashkent

  // Initialize LCD
  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status) {
    Serial.println("LCD initialization failed!");
    Serial.print("Status code: ");
    Serial.println(status);
    hd44780::fatalError(status);
  }

  lcd.clear();

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

  //ex1
  theButton();

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

  // 3. Route the topic to the correct Pin
  // strcmp returns 0 if the strings match perfectly
  if (!strcmp(topic, RED)) {
    String state = parserForLeds(payload, length);
    bool power = LOW;
    if (state == "ON") {
      power = HIGH;
    }
    else if (state == "OFF") {
      power = LOW;
    }
    digitalWrite(RED_LED, power);
    Serial.print("[LED] Red LED ->");
    Serial.println(state);
  } 
  else if (!strcmp(topic, GREEN)) {
    String state = parserForLeds(payload, length);
    bool power = LOW;
    if (state == "ON") {
      power = HIGH;
    }
    else if (state == "OFF") {
      power = LOW;
    }
    digitalWrite(GREEN_LED, power);
    Serial.print("[LED] Green LED ->");
    Serial.println(state);   
  } 
  else if (!strcmp(topic, BLUE)) {
    String state = parserForLeds(payload, length);
    bool power = LOW;
    if (state == "ON") {
      power = HIGH;
    }
    else if (state == "OFF") {
      power = LOW;
    }
    digitalWrite(BLUE_LED, power);
    Serial.print("[LED] Blue LED ->");
    Serial.println(state);       
  } 
  else if (!strcmp(topic, YELLOW)) {
    String state = parserForLeds(payload, length);
    bool power = LOW;
    if (state == "ON") {
      power = HIGH;
    }
    else if (state == "OFF") {
      power = LOW;
    }
    digitalWrite(YELLOW_LED, power);
    Serial.print("[LED] Yellow LED ->");
    Serial.println(state); 
  }
  else if (!strcmp(topic, DISPLAY_TOPIC)) {       //displaying massage
    lcd.clear();
    parseMessageDisplay(payload, length);
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%d/%m %H:%M:%S", &timeinfo);
      // Display on LCD Line 2
      lcd.setCursor(0, 1);
      lcd.print(buffer);
    }
  }
  
}

String parserForLeds(byte* payload, unsigned int length){
  JsonDocument doc;
  deserializeJson(doc, payload, length);
  String state = doc["state"];

  return state;
}

void parseMessageDisplay(byte* payload, unsigned int length){
  JsonDocument text;
  deserializeJson(text, payload, length);
  String message = text["text"];

  lcd.setCursor(0, 0);
  lcd.print(message);
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

  // Subscribe to topic
  mqtt_client.subscribe(DISPLAY_TOPIC);
  Serial.print("Subscribed to topic: ");
  Serial.println(DISPLAY_TOPIC);
}

void sentButtonState(const char* State){
    JsonDocument state;
    state["light"] = State;
    state["timestamp"] = time(nullptr);

    char buffer[256];
    serializeJson(state, buffer);

    Serial.print("Publishing message: ");
    Serial.println(buffer);
    
    if (mqtt_client.publish(topic_button
    , buffer)) {
      Serial.println("Message published successfully!");
    } else {
      Serial.println("Failed to publish message!");
    }
    Serial.println("---");
}

void theButton(){
  bool lastState = digitalRead(BUTTON);
  delay(50);
  bool currentState = digitalRead(BUTTON);

  if (lastState == LOW && currentState == HIGH) {
    sentButtonState("PRESSED");
  } 
  else if (lastState == HIGH && currentState == LOW){
    sentButtonState("RELEASED");
  }
}


