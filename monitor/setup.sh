#!/bin/bash

echo "Installing prerequisites..."
sudo apt-get update
sudo apt-get -y install python3 python3-pip python3-flask mosquitto mongodb

# I think you can only install 1 thing at a time using pip...
sudo pip3 install paho-mqtt
sudo pip3 install PyMongo
sudo pip3 install flask-restful