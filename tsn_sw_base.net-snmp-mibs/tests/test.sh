#!/bin/bash

python main.py -t ${target} -r /code/results/snmp-results.xml
chown -R ${USER_ID}:${GROUP_ID} /code/results