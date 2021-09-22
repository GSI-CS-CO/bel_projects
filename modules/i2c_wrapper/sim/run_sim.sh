#!/bin/bash

# Simulation settings
TB_NAME="i2c_testbench"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="1ms"
VCD_NAME="$TB_NAME.vcd"
VCD_VIEWER="gtkwave"

# Files
I2C_PATH="../../../ip_cores/general-cores/modules/wishbone/wb_i2c_master"
VHD_PACK="../../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd \
../../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd"
VHD_FILES="$VHD_PACK \
../../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
$I2C_PATH/i2c_master_bit_ctrl.vhd \
$I2C_PATH/i2c_master_byte_ctrl.vhd \
$I2C_PATH/i2c_master_top.vhd \
$I2C_PATH/wb_i2c_master.vhd \
$I2C_PATH/xwb_i2c_master.vhd \
i2c_testbench.vhd"

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
