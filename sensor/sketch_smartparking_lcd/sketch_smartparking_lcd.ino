#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "FSB-201";      //Wifi connect
const char* password = "123@123a";   //Password

const char* mqtt_broker = "192.168.1.69"; // Change it
const int mqtt_port = 1883;
const char *mqtt_topic = "smartparking/lcd_parking";
const char* mqtt_username = "fsb2024"; //User
const char* mqtt_password = "12345678"; //Password

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(9600);
    
    lcd.init();           // Initialize the LCD
    lcd.backlight();      // Turn on the backlight
    lcd.setCursor(0, 0);  // Set cursor to the first row, first column
    lcd.print("Hello World");

    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
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
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Received message: ");

    String message;
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        message += (char)payload[i];
    }
    Serial.println();  

    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);

    
    lcd.clear();        
    lcd.setCursor(0, 0); 
    

    if (message.length() > 16) {
        lcd.print(message.substring(0, 16));  
        lcd.setCursor(0, 1);                
        lcd.print(message.substring(16));   
    } else {
        lcd.print(message); 
    }
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();   
}
