#!/bin/bash

python main.py -t ${target} -r /code/results/results.xml
chown -R ${USER_ID}:${GROUP_ID} /code/results