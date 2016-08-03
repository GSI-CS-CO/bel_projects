#!/bin/bash

PNGNAME=${1%.dot}.png

head -n 4 $1 > tmp.dot
sort <(sed 's/^< //' <(tail -n +2 <(diff <(tail -n +4 $1) <(tail -n 1 $1)))) >> tmp.dot
tail -n 1 $1 >> tmp.dot
dot -Tpng tmp.dot -o$PNGNAME
