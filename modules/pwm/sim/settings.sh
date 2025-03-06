# Simulation settings
TB_NAME="pwm_testbench"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="10ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

# Files
PWM_PATH="../src"
VHD_PACK="../../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd"
VHD_FILES="$VHD_PACK \
           ../../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
           $PWM_PATH/pwm_channel_pkg.vhd \
           $PWM_PATH/pwm_channel.vhd \
           $PWM_PATH/pwm_pkg.vhd \
           $PWM_PATH/pwm.vhd \
           pwm_testbench.vhd"