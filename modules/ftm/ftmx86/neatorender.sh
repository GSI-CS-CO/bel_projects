#!/bin/bash
while ./sleep_until_modified.sh $1.dot ; do neato -Grankdir=LR -Goverlap=compress -Gmodel=subset -Gsep=+0.05 -Tsvg $1.dot -o$1.svg; done
