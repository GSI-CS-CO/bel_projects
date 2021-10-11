#!/bin/bash
source settings.sh

# Run
echo "Removing old files ..."
for item in $TB_NAME *.o *.vcd *.cf
do
  echo "Removing $item ..."
  rm $item > /dev/null 2>&1
done

echo "Analysing $VHD_FILES ..."
$GHDL_BIN -a --work=work $GHDL_FLAGS $VHD_FILES

echo "Elaborating $TB_NAME ..."
$GHDL_BIN -e --work=work $GHDL_FLAGS $TB_NAME

echo "Starting simulation ..."
$GHDL_BIN -r $TB_NAME --stop-time=$STOP_TIME --vcd=$VCD_NAME

echo "Hint: View simulation -> $VCD_VIEWER $TB_NAME.vcd &"

exit 0
