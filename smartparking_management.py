import random
import time
import mariadb
from string import Template

from paho.mqtt import client as mqtt_client

# connection parameters
conn_params= {
    "user" : "hieunt",
    "password" : "12345678",
    "host" : "localhost",
    "database" : "SmartParking"
}
conn = mariadb.connect(**conn_params)

broker = '192.168.102.189'
port = 1883
username = "fsb2024"
password = "12345678"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

ir1_state = 0 # check true false of infrared sensor 1
ir2_state = 0 # check true false of infrared sensor 2
servo_state = 0 # check true false of servo sensor

state_progress = "NONE"
free_slots = 4

ir1_topic = "smartparking/infrared_sensor1"
ir2_topic = "smartparking/infrared_sensor2"
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
    client.subscribe(ir1_topic)
    client.subscribe(ir2_topic)
    client.on_message = on_message
    
def on_message(client, userdata, msg):
    global ir1_state
    global ir2_state
    global servo_state
    global state_progress
    global free_slots

    print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
    content = msg.payload.decode()

    if msg.topic == ir1_topic:
        ir1_state = content
    elif msg.topic == ir2_topic:
        ir2_state = content

    print(ir1_state)
    print(servo_state)
    print(ir2_state)

    if state_progress == "NONE":
        if str(ir1_state) == '1' and str(servo_state) == '0' and str(ir2_state) == '0':
            print('here')
            state_progress = "IN"
            publishToServo(client, 1)

        if str(ir1_state) == '0' and str(servo_state) == '0' and str(ir2_state) == '1':
            state_progress = "OUT"
            publishToServo(client, 1)

    if state_progress == "IN":
        if str(ir1_state) == '0' and str(servo_state) == '1' and str(ir2_state) == '1':
            publishToServo(client, 0)
        if str(ir1_state) == '0' and str(servo_state) == '0' and str(ir2_state) == '0': # In successfully
            # save to databse
            free_slots -= 1
            insertToDatabase("IN", free_slots)
            state_progress = "NONE"

    if state_progress == "OUT":
        if  str(ir2_state) ==  '0' and str(servo_state) == '1' and str(ir1_state) == '1':
            publishToServo(client, 0)
        if str(ir2_state) == '0' and str(servo_state) == '0' and str(ir1_state) == '0':
            # save to databse
            free_slots += 1
            insertToDatabase("OUT", free_slots)
            state_progress = "NONE"

    print(state_progress)

def subscribeAndPublish(client: mqtt_client):
    subscribe(client) # subscibe

def publishToServo(client: mqtt_client, signal: int):
    global servo_state
    result = client.publish(servo_topic, f"{signal}")
    servo_state = str(signal)
    print(f"result of publish to servo: {result}")

def connectToDatabase():
    print("Connected to Database")
    cur = conn.cursor()
    #retrieving information
    cur.execute("SELECT * FROM ParkingTrack")

    for (ParkingID, ParkingType, Time, FreeSlots) in cur:
        print(f"ParkingID: {ParkingID}, ParkingType: {ParkingType}, Time: {Time}, FreeSlots: {FreeSlots}")

def insertToDatabase(parkingType, freeSlots):
    cur = conn.cursor()
    cur.execute(f"INSERT INTO ParkingTrack (ParkingType, FreeSlots) VALUES (\"{parkingType}\", {freeSlots})")
    conn.commit()

def run():
    client = connect_mqtt()
    connectToDatabase()
    subscribeAndPublish(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
