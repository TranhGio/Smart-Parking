# python 3.11

import random
import time

from paho.mqtt import client as mqtt_client

broker = '192.168.102.189'
port = 1883
username = "fsb2024"
password = "12345678"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

ir_state = 1 # check true false of infrared sensor
servo_state = 1 # check true false of servo sensor

ir_topic = "smartparking/infrared_sensor"
servo_topic = "smartparking/servo_sensor"

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION1, client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        content = msg.payload.decode()
    
        publishToServo(client, content)

    client.subscribe(ir_topic)
    client.on_message = on_message


def subscribeAndPublish(client: mqtt_client):
    subscribe(client) # subscibe 
    
    
def publishToServo(client: mqtt_client, signal: int):
    result = client.publish(servo_topic, f"{signal}")
    print(f"result of publish to servo: {result}")
def run():
    client = connect_mqtt()
    subscribeAndPublish(client)
    client.loop_forever()


if __name__ == '__main__':
    run()