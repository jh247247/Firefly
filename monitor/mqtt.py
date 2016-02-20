#!/usr/bin/python3
from pymongo import MongoClient
import json
import paho.mqtt.client as mqtt
import time

# TODO: proper logging

DECODE_SCHEME = "utf-8"

# might not be the best to keep these as magic strings, but idk
nodes = MongoClient()['redo']['node']
fireflies = MongoClient()['redo']['firefly']

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result: " + str(rc))
    client.subscribe("node")

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
            data['timestamp'] = int(time.time())
            updateNode(data)



def updateNode(nodeDict):
    current = nodes.find_one({"nodeId" : nodeDict.get("nodeId")},{'_id' : False})

    # does not exist yet...
    if current is None:
        current = {i:nodeDict[i] for i in nodeDict if i!='firefly'}

    # Oh, no fireflies registered? make a blank list.
    if current.get('fireflies') is None:
        current['fireflies'] = []

    if nodeDict.get('firefly') is not None:
        updateFirefly(nodeDict)
        # see if current firefly exists in list
        f = next(((i,v) for i,v in enumerate(current.get('fireflies')) \
                  if v == nodeDict['firefly']['fireflyID']),None)
        if f is None:
            currFire = nodeDict['firefly']['fireflyID']
            current['fireflies'].insert(0,currFire)

    nodes.update_one({'nodeId':current.get('nodeId')},{'$set':current},upsert=True)


def updateFirefly(nodeDict):
    current = fireflies.find_one({"fireflyID" : nodeDict['firefly']['fireflyID']},{'_id' : False})
    if current is None:
        # does not exist yet, create from this
        current = nodeDict['firefly']
    else:
        # we already exist, remove us from previous node
        # fireflies should only be associated with a single node at a time.
        if current['nodeId'] != nodeDict['nodeId']:
            nodes.update({'nodeId' : current['nodeId']},
                         {'$pull': {'fireflies' : nodeDict['firefly']['fireflyID']}})

    # TODO: check power of transmit levels? don't update if updated in last 5 seconds and has lower power level?

    # copy over node we are hearing from
    current['nodeId'] = nodeDict['nodeId']
    current['timestamp'] = nodeDict['timestamp']

    fireflies.update_one({'fireflyId':current.get('fireflyID')},{'$set':current},upsert=True)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

def run():
    client.connect("localhost",1883,60)
    client.loop_forever()
