#!/bin/bash
echo PHASE 1
./dm-sched dev/ttyUSB0 clear
./dm-sched dev/ttyUSB0 add -s test$1_0.dot
cp download.dot before.dot
echo PHASE 2
echo "./dm-sched dev/ttyUSB0 remove -s test$1_1.dot"
./dm-sched dev/ttyUSB0 remove -s test$1_1.dot

