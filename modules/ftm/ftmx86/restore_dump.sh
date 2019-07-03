#!/bin/bash
dev=$1
for i in `seq 0 3`;
do
	let j="2 * ($i + 1)"
	eb-put "$dev" 0x41"$j"0000 ./memdump_$i.bin
done
