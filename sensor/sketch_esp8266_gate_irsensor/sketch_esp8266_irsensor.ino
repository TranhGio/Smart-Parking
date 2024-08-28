#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

#define IR1 5
#define IR2 4

const char* ssid = "Tang 2";      //Wifi connect
const char* password = "bin12345";   //Password
const char* mqtt_broker = "192.168.102.189"; // Change it

const int mqtt_port = 1883;
const char *mqtt_ir1_topic = "smartparking/gate/ir_outside";
const char *mqtt_ir2_topic = "smartparking/gate/ir_g";
const char* mqtt_username = "fsb2024"; //User
const char* mqtt_password = "12345678"; //Password

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

bool ir1Data;
bool ir2Data;

void setup() {
    Serial.begin(9600);
    connectToWiFi();
    pinMode(IR1, INPUT);
    pinMode(IR2, INPUT);
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
    bool currentIR1Data = digitalRead(IR1);
    bool currentIR2Data = digitalRead(IR2);

    // Publish whenver value has been changed
    if(currentIR1Data != ir1Data) {
      char message[1];
      sprintf(message, "%d", ir1Data);
      mqtt_client.publish(mqtt_ir1_topic, message);
      ir1Data = currentIR1Data;
    }

    // Publish whenver value has been changed
    if(currentIR2Data != ir2Data) {
      char message[1];
      sprintf(message, "%d", ir2Data);
      mqtt_client.publish(mqtt_ir2_topic, message);
      ir2Data = currentIR2Data;
    }
    delay(500);
}
