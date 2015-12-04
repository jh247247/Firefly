#!/usr/bin/python3
from pymongo import MongoClient
from bson import json_util
import flask_restful as fr


node_coll = MongoClient()['redo']['node']

node_coll.insert_one({'hello' : 'world', 'this is' : 'a test!'})
node_coll.insert_one({'hello' : 'earthlings', 'this is' : 'an alien'})
node_coll.insert_one({'hello' : 'pardners', 'this is' : 'a cowboy'})

class node_res(fr.Resource):
    def get(self):
        return json_util.dumps(node_coll.find_one({'hello' : 'world'}))

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
