#!/bin/bash
mkdir -p /data/mosquitto




if [ ! -f /data/mosquitto/mosquitto.conf ]; then
    echo "Copying configuration to /data/mosquitto"
    cp bin/mosquitto.conf /data/mosquitto/mosquitto.conf
fi

if [ ! -f /data/mosquitto/mosquitto_pass ]; then
    touch bin/mosquitto_pass
    bin/mosquitto_passwd -b bin/mosquitto_pass $MOSQUITTO_USERNAME $MOSQUITTO_PASS
    echo "Copying mosquitto_pass to /data/mosquitto"
    cp bin/mosquitto_pass /data/mosquitto/mosquitto_pass
fi


echo "Running mosquitto!"
bin/mosquitto -c /data/mosquitto/mosquitto.conf