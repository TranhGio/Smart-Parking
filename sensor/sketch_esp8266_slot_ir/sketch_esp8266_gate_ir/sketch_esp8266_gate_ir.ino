#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

#define SLOT_1 5
#define SLOT_2 4
// #define SLOT_3 0
// #define SLOT_4 2

const char* ssid = "Tang 2";      //Wifi connect
const char* password = "bin12345";   //Password
const char* mqtt_broker = "192.168.102.189"; // Change it

const int mqtt_port = 1883;
const char *mqtt_slot_ir1_topic = "smartparking/slot/ir1";
const char *mqtt_slot_ir2_topic = "smartparking/slot/ir2";
// const char *mqtt_slot_ir3_topic = "smartparking/slot/ir3";
// const char *mqtt_slot_ir4_topic = "smartparking/slot/ir4";
const char* mqtt_username = "fsb2024"; //User
const char* mqtt_password = "12345678"; //Password

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

bool ir1Data;
bool ir2Data;
// bool ir3Data;
// bool ir4Data;

void setup() {
    Serial.begin(9600);
    connectToWiFi();
    pinMode(SLOT_1, INPUT);
    pinMode(SLOT_2, INPUT);
    // pinMode(SLOT_3, INPUT);
    // pinMode(SLOT_4, INPUT);
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
    bool currentIR1Data = digitalRead(SLOT_1);
    bool currentIR2Data = digitalRead(SLOT_2);
    Serial.print(currentIR1Data);
    Serial.print(currentIR2Data);
    // bool currentIR3Data = digitalRead(SLOT_3);
    // bool currentIR4Data = digitalRead(SLOT_4);

    // Publish whenver value has been changed
    if(currentIR1Data != ir1Data) {
      char message[1];
      sprintf(message, "%d", currentIR1Data);
      mqtt_client.publish(mqtt_slot_ir1_topic, message);
      ir1Data = currentIR1Data;
    }

    if(currentIR2Data != ir2Data) {
      char message[1];
      sprintf(message, "%d", currentIR2Data);
      mqtt_client.publish(mqtt_slot_ir2_topic, message);
      ir2Data = currentIR2Data;
    }

    // if(currentIR3Data != ir3Data) {
    //   char message[1];
    //   sprintf(message, "%d", ir3Data);
    //   mqtt_client.publish(mqtt_slot_ir3_topic, message);
    //   ir3Data = currentIR3Data;
    // }

    // if(currentIR4Data != ir4Data) {
    //   char message[1];
    //   sprintf(message, "%d", ir4Data);
    //   mqtt_client.publish(mqtt_slot_ir4_topic, message);
    //   ir4Data = currentIR4Data;
    // }
    delay(500);
}
