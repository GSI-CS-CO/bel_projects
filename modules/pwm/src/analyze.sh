#!/bin/bash
source settings.sh

# Run
echo "Removing old files ..."
for item in $TB_NAME *.o *.vcd *.cf *.ghw
do
  echo "Removing $item ..."
  rm $item > /dev/null 2>&1
done

echo "Analysing $VHD_FILES ..."
$GHDL_BIN -a --work=work $GHDL_FLAGS $VHD_FILES


exit 0
