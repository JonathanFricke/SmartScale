#!/bin/bash

# Stop the server
./kill_garmin_server.sh

# Optional: short delay to ensure the process has been fully stopped
sleep 1

# Start the server again
./start_garmin_server.sh
