#!/usr/bin/python3
from pymongo import MongoClient
import json
from bson import json_util
from flask import request
import flask_restful as fr
import time

nodes = MongoClient()['redo']['node']

"""
This class is the api call for HOSTNAME/node/NODE_ID.
get: Returns the data known for the given node id, 404 if not known.
put: Puts the data into the database, appends the node_id. Returns the
     full JSON inserted into the database.
"""
class node_res(fr.Resource):
    def get(self, node_id):
        # grab the data from the database, send it back out
        # maybe authenticate? idk
        lookup = nodes.find_one({"nodeId" : node_id},{'_id' : False})
        if lookup is not None:
            return lookup
        return 204

    def put(self, node_id):
        # insert node data into database. easy peasy.
        # This overwrites any data that was previously there, so may
        # need some protection?

        # NOTE: will we ever actually put in a new node using the app?
        dat = {"usrData" : request.form}
        dat["nodeId"] = node_id
        dat["timestamp"] = int(time.time())

        nodes.update_one({"nodeId":node_id}, {'$set': dict(dat)}, upsert=True)

        return dat

"""
This class is the api call for HOSTNAME/node.
Calling this returns a list of all known node IDs, formatted as basic
JSON.
Note that as of yet there is no verification of the format, since this
is straight from the database we can assume that the data will be
relatively clean.
"""
class nodelist_res(fr.Resource):
    def get(self):
        return {'ids':nodes.distinct('nodeId')}


"""
This function sets up the API for Flask.
"""
def setup(api):
    api.add_resource(nodelist_res,'/node')
    api.add_resource(node_res,'/node/<string:node_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
