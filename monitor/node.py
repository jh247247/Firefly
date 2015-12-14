#!/usr/bin/python3
from pymongo import MongoClient
import json
from bson import json_util
from flask import request
import flask_restful as fr

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
        lookup = nodes.find_one({"nodeId" : str(node_id)})
        if lookup is not None:
            return json_util.dumps(lookup)
        return 204

    def put(self, node_id):
        # insert node data into database. easy peasy.
        # This overwrites any data that was previously there, so may
        # need some protection?

        # NOTE: will we ever actually put in a new node using the app?
        dat = {"data" : request.form}
        dat["nodeId"] = node_id

        nodes.insert_one(dat)

        # TODO: we may need to clean out the mongodb _id field from this...
        return json.dumps(dat,default=json_util.default)

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
        return nodes.distinct('nodeId')


"""
This function sets up the API for Flask.
"""
def setup(api):
    api.add_resource(nodelist_res,'/node')
    api.add_resource(node_res,'/node/<int:node_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
