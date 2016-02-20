#!/usr/bin/python3
from pymongo import MongoClient
import json
from bson import json_util
from flask import request
import flask_restful as fr

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
        dat = {"data" : request.form}
        dat["fireflyId"] = node_id

        fireflies.insert_one(dict(dat))

        return dat

class fireflylist_res(fr.Resource):
    def get(self):
        return {'fireflyIds':fireflies.distinct('fireflyId')}


def setup(api):
    api.add_resource(fireflylist_res,'/firefly')
    api.add_resource(firefly_res,'/firefly/<string:firefly_id>')

if __name__ == '__main__':
    print('WRITE TESTS FOR MEH')
