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
          ../../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wb_simple_pwm/simple_pwm_wbgen2_pkg.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wb_simple_pwm/simple_pwm_wb.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wb_simple_pwm/wb_simple_pwm.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wb_simple_pwm/xwb_simple_pwm.vhd"
VHD_FILES="$VHD_PACK \
           ../../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
           $PWM_PATH/pwm_pkg.vhd \
           $PWM_PATH/pwm.vhd \
           pwm_testbench.vhd"
