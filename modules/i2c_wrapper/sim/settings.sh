# Simulation settings
TB_NAME="i2c_testbench"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="5ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

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
           ../src/hdl/wb_i2c_wrapper_pkg.vhd \
           i2c_testbench.vhd"
