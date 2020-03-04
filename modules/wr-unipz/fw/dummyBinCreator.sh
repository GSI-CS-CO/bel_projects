#!/bin/bash
# write data to file

FILENAME=dummy.bin
FILESIZE=98304

NWORDS=$((FILESIZE / 4))

rm $FILENAME

counter=1
while [ $counter -le $NWORDS ]
do
    echo -n -e '\xde\xad\xbe\xef' >> $FILENAME
    ((counter++))
done

echo All done
