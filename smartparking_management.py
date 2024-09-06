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

broker = '192.168.1.69' # change later
port = 1883
username = "fsb2024"
password = "12345678"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

gate_ir1_state = 0 # check true false of gate infrared sensor 1
gate_ir2_state = 0 # check true false of gate infrared sensor 2
servo_state = 0 # check true false of servo sensor

slot_ir1_state = 0
slot_ir2_state = 0
slot_ir3_state = 0
slot_ir4_state = 0

state_progress = "NONE"
free_slots = 4

gate_ir1_topic = "smartparking/gate/infrared_sensor1"
gate_ir2_topic = "smartparking/gate/infrared_sensor2"
servo_topic = "smartparking/servo_sensor"
lcd_topic = "smartparking/lcd_parking"
slot_ir1_topic = "smartparking/slot/infrared_sensor1"
slot_ir2_topic = "smartparking/slot/infrared_sensor2"
slot_ir3_topic = "smartparking/slot/infrared_sensor3"
slot_ir4_topic = "smartparking/slot/infrared_sensor4"

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
    # subscribe gate ir sensor
    client.subscribe(gate_ir1_topic)
    client.subscribe(gate_ir2_topic)

    # subscribe slot ir sensor
    client.subscribe(slot_ir1_topic)
    client.subscribe(slot_ir2_topic)
    client.subscribe(slot_ir3_topic)
    client.subscribe(slot_ir4_topic)

    client.on_message = on_message
    
def on_message(client, userdata, msg):
    global gate_ir1_state
    global gate_ir2_state
    global servo_state
    global slot_ir1_state
    global slot_ir2_state
    global slot_ir3_state
    global slot_ir4_state
    global state_progress
    global free_slots

    print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
    content = msg.payload.decode()

    if msg.topic == gate_ir1_topic:
        gate_ir1_state = content
    elif msg.topic == gate_ir2_topic:
        gate_ir2_state = content

    print(gate_ir1_state)
    print(gate_ir2_state)
    print(servo_state)
    print(slot_ir1_state)
    print(slot_ir2_state)
    print(slot_ir3_state)
    print(slot_ir4_state)

    # handle the gate state
    if state_progress == "NONE":
        if str(gate_ir1_state) == '1' and str(servo_state) == '0' and str(gate_ir2_state) == '0':
            if free_slots == 0:
                publishToLCD(client, "Sorry, we're run out of slots!")
            else:
                state_progress = "IN"
                publishToServo(client, 1)

        if str(gate_ir1_state) == '0' and str(servo_state) == '0' and str(gate_ir2_state) == '1':
            state_progress = "OUT"
            publishToServo(client, 1)

    if state_progress == "IN":
        if str(gate_ir1_state) == '0' and str(servo_state) == '1' and str(gate_ir2_state) == '1':
            publishToServo(client, 0)
        if str(gate_ir1_state) == '0' and str(servo_state) == '0' and str(gate_ir2_state) == '0':
            state_progress = "NONE"

    if state_progress == "OUT":
        if  str(gate_ir2_state) ==  '0' and str(servo_state) == '1' and str(gate_ir1_state) == '1':
            publishToServo(client, 0)
        if str(gate_ir2_state) == '0' and str(servo_state) == '0' and str(gate_ir1_state) == '0':
            state_progress = "NONE"

    print(state_progress)

    # handle slot occupied
    if msg.topic == slot_ir1_topic and content != slot_ir1_state:
        slot_ir1_state = content
        changeSlotState(1, slot_ir1_state)
        updateFreeSlot()
        publishToLCD(client, f"Welcome! Slot left: {free_slots}")
    elif msg.topic == slot_ir2_topic and content != slot_ir2_state:
        slot_ir2_state = content
        changeSlotState(2, slot_ir2_state)
        updateFreeSlot()
        publishToLCD(client, f"Welcome! Slot left: {free_slots}")
    elif msg.topic == slot_ir3_topic and content != slot_ir3_state:
        slot_ir3_state = content
        changeSlotState(3, slot_ir3_state)
        updateFreeSlot()
        publishToLCD(client, f"Welcome! Slot left: {free_slots}")
    elif msg.topic == slot_ir4_topic and content != slot_ir4_state:
        slot_ir4_state = content
        changeSlotState(4, slot_ir4_state)  
        updateFreeSlot()
        publishToLCD(client, f"Welcome! Slot left: {free_slots}")

def subscribeAndPublish(client: mqtt_client):
    subscribe(client) # subscibe

def publishToServo(client: mqtt_client, signal: int):
    global servo_state
    result = client.publish(servo_topic, f"{signal}")
    servo_state = str(signal)
    print(f"result of publish to servo: {result}")

def updateFreeSlot():
    global free_slots
    freeSlotsTempt = 0

    if(slot_ir1_state == 1):
        freeSlotsTempt = freeSlotsTempt + 1

    if(slot_ir2_state == 1):
        freeSlotsTempt = freeSlotsTempt + 1

    if(slot_ir3_state == 1):
        freeSlotsTempt = freeSlotsTempt + 1

    if(slot_ir4_state == 1):
        freeSlotsTempt = freeSlotsTempt + 1

    free_slots = freeSlotsTempt

def publishToLCD(client: mqtt_client, signal: str):
    result = client.publish(lcd_topic, signal)

def connectToDatabase():
    print("Connected to Database")
    cur = conn.cursor()
    # retrieving information
    cur.execute("SELECT * FROM ParkingTrack")
    for (ParkingID, SlotId, EntryTime, ExitTime) in cur:
        print(f"ParkingID: {ParkingID}, SlotId: {SlotId}, EntryTime: {EntryTime}, ExitTime: {ExitTime}")

def changeSlotState(slotId: int, state: str):
    entry = True
    if state == '0':
        entry = False
    else:
        entry = True
    cur = conn.cursor()
    cur.execute(f"SELECT * FROM `ParkingTrack` WHERE SlotID = {slotId} ORDER BY EntryTime DESC LIMIT 1;")
    conn.commit()

    rowcount = cur.rowcount

    if rowcount == 0 and entry == True:
        # insert a new record of slot 1
        cur.execute(f"INSERT INTO `ParkingTrack`(`SlotID`) VALUES ({slotId})")
        cur.execute(f"UPDATE `ParkingSlot` SET `Status`= 1 where `SlotID` = {slotId}")
    elif rowcount == 1 and entry == False:
        # edit the that latest record checkout time
        result_set = cur.fetchall()
        for row in result_set:
            cur.execute(f"UPDATE ParkingTrack SET ExitTime=now() WHERE ParkingID = {row[0]}")
            cur.execute(f"UPDATE `ParkingSlot` SET `Status`= 0 where `SlotID` = {row[1]}")
    conn.commit()

def run():
    client = connect_mqtt()
    connectToDatabase()
    subscribeAndPublish(client)
    client.loop_forever()

if __name__ == '__main__':
    run()
