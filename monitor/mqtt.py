#!/usr/bin/python3
from pymongo import MongoClient
import json
import paho.mqtt.client as mqtt

# TODO: proper logging

DECODE_SCHEME = "utf-8"

# might not be the best to keep these as magic strings, but idk
nodes = MongoClient()['redo']['node']
fireflies = MongoClient()['redo']['firefly']

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result: " + str(rc))
    client.subscribe("node")
    client.subscribe("firefly")

def on_message(client, userdata, msg):
    print("MQTT: received message: " +
          str(msg.topic)+": "+str(msg.payload, DECODE_SCHEME))
    data = None
    try:
        data = json.loads(str(msg.payload, DECODE_SCHEME))
    except ValueError as e:
        print("MQTT: json decode error: " + str(e))

    # Put into db depending on topic
    # TODO: Add authentication before db insertian
    if str(msg.topic) == "node":
        if data is not None and data.get('nodeId') is not None:
            print("MQTT: inserting decoded json into node database: " + str(data))
            nodes.update_one({'nodeId':data.get('nodeId')},{'$set':data},upsert=True)

    elif str(msg.topic) == "firefly":
        if data is not None and data.get('fireflyId') is not None:
            print("MQTT: inserting decoded json into firefly database: " + str(data))
            fireflies.update_one({'fireflyId':data.get('fireflyId')},{'$set':data},upsert=True)



client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost",1883,60)

def run():
    client.loop_forever()
