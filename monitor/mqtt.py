#!/usr/bin/python3
from pymongo import MongoClient
import json
import paho.mqtt.client as mqtt

DECODE_SCHEME = "utf-8"

# might not be the best to keep these as magic strings, but idk
nodes = MongoClient()['redo']['node']
fireflies = MongoClient()['redo']['firefly']

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result: " + str(rc))
    client.subscribe("node")
    # TODO: subscribe to node channel

def on_message(client, userdata, msg):
    print("MQTT: received message: " +
          str(msg.topic)+":"+str(msg.payload, DECODE_SCHEME))
    data = None
    try:
        data = json.loads(str(msg.payload, DECODE_SCHEME))
    except ValueError as e:
        print("MQTT: json decode error: " + str(e))

    # woo! no errors, put into database
    if data is not None:
        # might be a good idea for some authentication here.
        print("MQTT inserting decoded json into database")
        nodes.update_one({'nodeId':data.get('nodeId')},{'$setOnInsert':data},True)




client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost",1883,60)

def run():
    client.loop_forever()
