#!/bin/bash

for i in "make A" "make B" "add nuzky" "add vrtacka" "start Nora nuzky" "start Vojta vrtacka"; do
    echo $i
    sleep 0.1
done | ./factory
