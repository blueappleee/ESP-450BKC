#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// credentials stored in config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
WiFiClient esp_wifi;

const char* mqtt_server = MQTT_SERVER;
PubSubClient client(esp_wifi);

const int button_pin =5; //3v, g, D1
int buttonValue = 0;

const int servo_pin = 4; //vu, g, D2
Servo servo;

int turnOn = 0;
int primed = 0;
char temp[1];

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (topic=="basement/coffeeMachine") {
    if (messageTemp == "status") {
      itoa(primed, temp, 10);
      client.publish("basement/coffeeStatus", temp);
      
    } else if (messageTemp == "on") {
      turnOn = 1;
    }  
  } 
}

void connect_MQTT() {
  digitalWrite(LED_BUILTIN, LOW);
  while (!client.connected()) {
    Serial.println("MQTT Connecting");

    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT Connected");  
      // Subscribe or resubscribe to a topic
      client.subscribe("basement/coffeeMachine");
    } else {
      Serial.println("Failed. Retrying in 2 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
  delay(500);
  digitalWrite(LED_BUILTIN,HIGH);
}

void connect_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("WIFI Connecting");
  }
  
  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // setup Serial
  Serial.begin(9600);
  Serial.setTimeout(1);
  
  //Setup pins
  pinMode(button_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  servo.attach(servo_pin);
  servo.write(0);
  
 // Connect Wifi and MQTT. Onboard LED ON while connecting
 digitalWrite(LED_BUILTIN, LOW);
 
 connect_wifi();
 digitalWrite(LED_BUILTIN,HIGH);
 delay(500);

  Serial.println("Setting up MQTT");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("MQTT Started");
  connect_MQTT();

  delay(500);
}

void loop() {
  buttonValue = digitalRead(button_pin);
  
  if (buttonValue == HIGH) {
    Serial.println("HIGH");
    primed = 1;
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN,HIGH);
  }
  
  if (!client.connected()) {
    connect_MQTT();
  }

  if(!client.loop()) {
    client.connect("ESP8266Client");
  }


  if (turnOn == 1 && primed == 1) {
    Serial.println("Turning on");
    
    servo.write(120);
    delay(200);
    servo.write(0);
    
    turnOn = 0;
    primed = 0;
  } else if (turnOn == 1 && primed == 0) {
    turnOn = 0;
    
  }
  Serial.print("Primed:");
  Serial.println(primed);
  Serial.print("Servo value:");
  Serial.println(servo.read());
  Serial.print("TurnOn:");
  Serial.print(turnOn);
  Serial.println(" ");
  //delay();
  
}
