# Simulation settings
TB_NAME="cellular_ram_testbench"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="10ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

# Files
VHD_PACK="../../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd"
VHD_FILES="$VHD_PACK \
          ../src/hdl/cellular_ram_pkg.vhd \
          ../src/hdl/cellular_ram.vhd \
          cellular_ram_testbench.vhd"
