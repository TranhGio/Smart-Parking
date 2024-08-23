#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

#define IR 5

const char* ssid = "Tang 2";      //Wifi connect
const char* password = "bin12345";   //Password

const char* mqtt_broker = "192.168.102.189"; // Change it
const int mqtt_port = 1883;
const char *mqtt_topic = "smartparking/infrared_sensor";
const char* mqtt_username = "fsb2024"; //User
const char* mqtt_password = "12345678"; //Password

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

bool irData;

void setup() {
    Serial.begin(9600);
    connectToWiFi();
    pinMode(IR, INPUT);
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    connectToMQTTBroker();
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

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();
    
    // current signal
    bool currentIRData = digitalRead(IR);

    // Publish whenver value has been changed
    if(currentIRData != irData) {
      char message[1];
      sprintf(message, "%d", irData);
      mqtt_client.publish(mqtt_topic, message);
      irData = currentIRData;
    }
    delay(500);
}
