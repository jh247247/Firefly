#!/usr/bin/python3
from pymongo import MongoClient
from bson import json_util
from flask import request
import flask_restful as fr

nodes = MongoClient()['redo']['node']

class node_res(fr.Resource):
    def get(self, node_id):
        # grab the data from the database, send it back out
        # maybe authenticate? idk
        return json_util.dumps(
            nodes.find_one({str(node_id) : {'$exists' : True}}))

    def put(self, node_id):
        # insert node data into database. easy peasy.
        # This overwrites any data that was previously there, so may
        # need some protection?
        nodes.insert_one({str(node_id) : request.form})
        return {node_id : request.form}

def setup(api):
    api.add_resource(node_res,'/<int:node_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
