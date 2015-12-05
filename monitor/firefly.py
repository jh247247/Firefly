#!/usr/bin/python3
from pymongo import MongoClient
from bson import json_util
from flask import request
import flask_restful as fr

fireflies = MongoClient()['redo']['firefly']

class firefly_res(fr.Resource):
    def get(self, firefly_id):
        # grab the data from the database, send it back out
        # maybe authenticate? idk
        lookup = fireflies.find_one({str(firefly_id) : {'$exists' : True}})
        if lookup is not None:
            return json_util.dumps(lookup)
        return 404


    def put(self, firefly_id):
        # insert node data into database. easy peasy.
        # This overwrites any data that was previously there, so may
        # need some protection?

        # NOTE: will we ever actually put in a new node using the app?
        fireflies.insert_one({str(firefly_id) : request.form})
        return {firefly_id : request.form}

def setup(api):
    api.add_resource(firefly_res,'/firefly/<int:firefly_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
