#!/bin/bash

for i in "make A" "make B" "add nuzky" "add vrtacka" "start Nora nuzky" "start Vojta vrtacka" "remove vrtacka"; do
    echo $i
    if [ "$i" == "start Vojta vrtacka" ]; then
        sleep 1
    fi
done | ./factory
