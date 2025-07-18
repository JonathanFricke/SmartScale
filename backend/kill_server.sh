#!/bin/bash

# Find the PID of your Flask server using app.py and kill it
pid=$(ps aux | grep '[p]ython.*app.py' | awk '{print $2}')
if [ -n "$pid" ]; then
    echo "Killing Garmin server (PID: $pid)"
    kill $pid
else
    echo "Garmin server is not running."
fi
