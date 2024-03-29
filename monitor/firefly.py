#!/usr/bin/python3
from pymongo import MongoClient
import json
from bson import json_util
from flask import request
import flask_restful as fr
import time

fireflies = MongoClient()['redo']['firefly']

# TODO: add list api for fireflies.

class firefly_res(fr.Resource):
    def get(self, firefly_id):
        # grab the data from the database, send it back out
        # maybe authenticate? idk
        lookup = fireflies.find_one({"fireflyId" : firefly_id}, {'_id' : False})
        if lookup is not None:
            return lookup
        return 204


    def put(self, firefly_id):
        dat = {"usrData" : request.form}
        dat["fireflyId"] = firefly_id
        dat["timestamp"] = int(time.time())
        
        fireflies.update_one({"fireflyId":firefly_id}, {'$set': dict(dat)}, upsert=True)
        
        return dat

class fireflylist_res(fr.Resource):
    def get(self):
        return {'ids':fireflies.distinct('fireflyId')}


def setup(api):
    api.add_resource(fireflylist_res,'/firefly')
    api.add_resource(firefly_res,'/firefly/<string:firefly_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
