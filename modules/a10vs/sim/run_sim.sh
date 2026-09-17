#!/bin/bash
source settings.sh

# Run
echo "Removing old files ..."
remove_old_files

echo "Analysing $VHD_FILES ..."
$GHDL_BIN -a --work=work $GHDL_FLAGS $VHD_FILES

echo "Elaborating $TB_NAME ..."
$GHDL_BIN -e --work=work $GHDL_FLAGS $TB_NAME

echo "Starting simulation ..."
$GHDL_BIN -r $TB_NAME --stop-time=$STOP_TIME --wave=$GHW_NAME

exit 0
