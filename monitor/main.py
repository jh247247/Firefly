#!/usr/bin/python3
from flask import Flask
import flask_restful as fr
import core

app = Flask(__name__)
api = fr.Api(app)

core.api_setup(api)

if __name__ == '__main__':
    app.run(debug=True,
            host='0.0.0.0')
