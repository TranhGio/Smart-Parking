#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 #include <Servo.h>

const char* ssid = "FSB-201";      //Wifi connect
const char* password = "123@123a";   //Password

const char* mqtt_broker = "192.168.1.69"; // Change it
const int mqtt_port = 1883;
const char *mqtt_topic = "smartparking/servo_sensor";
const char* mqtt_username = "fsb2024"; //User
const char* mqtt_password = "12345678"; //Password

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

char servoData = '0';
int closeStateDegree = 100;
int openStateDegree = 0;
Servo servo;

void connectToWiFi();

void connectToMQTTBroker();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(9600);
    servo.attach(5);
    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
}

bool convertPayloadToBool(byte *payload, unsigned int length) {
    // Ensure that length is 1 to avoid accessing invalid memory
    if (length == 1) {
        // Convert the single character to a boolean
        return (*payload == '1');
    }
    // Handle cases where length is not 1, though it's expected to be 1
    return false;  // Or handle it as appropriate for your logic
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
            // Publish message upon successful connection
            mqtt_client.publish(mqtt_topic, "Connect successfully");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    if(strcmp(topic, mqtt_topic) == 0){
      char result = (char) payload[0];
      if(result != servoData){
          if(result == '1') {
              servo.write(openStateDegree);
          } else {
              servo.write(closeStateDegree);
          }
          servoData = result;
      }
    }
    delay(500);
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();   
}