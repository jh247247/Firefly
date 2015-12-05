#!/bin/bash

# MQTT broker
if pgrep mosquitto > /dev/null 2>&1; then
    echo "MQTT broker already running!"
else
    echo "Starting MQTT broker!"
    mosquitto -d &
fi

# Check if database is running
if /etc/init.d/mongodb status | grep running > /dev/null; then
    echo "Database server already running!"
else
    echo "Starting database server..."
    /etc/init.d/mongodb start > /dev/null
fi

# Check if our glue code is running, a bit hard to do since we can't
# guarantee that the script name will be the same. Oh well.
if pgrep monitor.py > /dev/null 2>&1; then
    echo "Monitor is already running!"
else
    echo "Starting monitor..."
    ./main.py &
fi
