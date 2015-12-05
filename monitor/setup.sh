#!/bin/bash

echo "Installing prerequisites..."
sudo apt-get update
sudo apt-get -y install python3 python3-pip python3-flask mosquitto \
     mongodb avahi-daemon avahi-discover libnss-mdns

# I think you can only install 1 thing at a time using pip...
sudo pip3 install paho-mqtt
sudo pip3 install PyMongo
sudo pip3 install flask-restful

# change permission of avahi folder so we can actually write to it
sudo chmod 755 -R /etc/avahi/services/
sudo cp mosquitto.service > /etc/avahi/services/mosquitto.service
sudo cp *.service > /etc/avahi/services/*
sudo /etc/init.d/avahi-daemon restart
