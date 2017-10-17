#!/bin/bash
while ./sleep_until_modified.sh $1.dot ; do dot -Tsvg $1.dot -o$1.svg; done
