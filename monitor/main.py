#!/usr/bin/python3
from flask import Flask
import flask_restful as fr
import core, mqtt
import threading

app = Flask(__name__)
api = fr.Api(app)

def flask_run():
    app.run(debug=True,
            host='0.0.0.0')

mqtt_thread = threading.Thread(target=mqtt.run, args = ())

if __name__ == '__main__':
    core.api_setup(api)
    # TODO: make sure these threads quit when we do?
    mqtt_thread.start()
    flask_run()
