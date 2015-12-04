#!/bin/bash

echo "Installing prerequisites..."
sudo apt-get update
sudo apt-get -y install python3 python3-pip mosquitto mongodb

# I think you can only install 1 thing at a time using pip...
sudo pip install paho-mqtt
sudo pip install Flask
sudo pip install PyMongo

