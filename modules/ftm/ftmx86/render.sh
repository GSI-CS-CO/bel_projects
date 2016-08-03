#!/bin/bash
while ./sleep_until_modified.sh download.dot ; do dot -Tsvg download.dot -odownload.svg; done
