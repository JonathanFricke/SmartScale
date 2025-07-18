#!/bin/bash

source ./.venv/bin/activate

set -a  # automatically export all variables
source ./.env
set +a  # stop automatically exporting variables

nohup .venv/bin/python ./app.py &

tail -f nohup.out

exit 0
