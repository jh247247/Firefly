#!/usr/bin/python3
from pymongo import MongoClient
from bson import json_util
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result: " + str(rc))
    client.subscribe("node")
    # TODO: subscribe to node channel

def on_message(client, userdata, msg):
    print("MQTT: received message: " + str(msg.topic)+":"+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost",1883,60)

def run():
    client.loop_forever()
