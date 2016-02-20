#!/usr/bin/python3
from pymongo import MongoClient
import json
import paho.mqtt.client as mqtt
import time

# TODO: proper logging

DECODE_SCHEME = "utf-8"
TIMESTAMP_DIFF = 5

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
        f = updateFirefly(nodeDict)
        if f['nodeId'] == nodeDict['nodeId'] and f['fireflyID'] not in current['fireflies']:
            current['fireflies'].append(f['fireflyID'])

    current['timestamp'] = nodeDict['timestamp']
    nodes.update_one({'nodeId':current.get('nodeId')},{'$set':current},upsert=True)



def updateFirefly(nodeDict):
    current = fireflies.find_one({"fireflyID" : nodeDict['firefly']['fireflyID']},{'_id' : False})
    update = nodeDict['firefly']


    if current is not None:
        # we already exist, remove us from previous node
        # fireflies should only be associated with a single node at a time.
        # TODO: take into account power levels
        update['nodeId'] = current['nodeId']
        if current['nodeId'] != nodeDict['nodeId']:
            if abs(nodeDict['timestamp']-current['timestamp']) > TIMESTAMP_DIFF:
                nodes.update({'nodeId' : current['nodeId']},
                             {'$pull': {'fireflies' : nodeDict['firefly']['fireflyID']}})
                update['nodeId'] = nodeDict['nodeId']
            else:
                return update
    else:
        # no previous data, set to what we have
        update['nodeId'] = nodeDict['nodeId']

    # update time of being last seen
    update['timestamp'] = nodeDict['timestamp']

    fireflies.update_one({'fireflyId':update.get('fireflyID')},{'$set':update},upsert=True)
    return update # return updated entry


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

def run():
    client.connect("localhost",1883,60)
    client.loop_forever()
