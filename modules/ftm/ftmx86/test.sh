#!/bin/bash
echo PHASE 1
./dm-sched dev/ttyUSB0 clear
./dm-sched dev/ttyUSB0 add -s test$1_0.dot
cp download.dot before.dot
echo PHASE 2
./dm-sched dev/ttyUSB0 add -s test$1_1.dot

