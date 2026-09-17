# Simulation settings
TB_NAME="psram_testbench"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="5ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

# Files
VHD_PACK="../../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd \
          ../../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd"
VHD_FILES="$VHD_PACK \
          ../psram_pkg.vhd \
          ../psram.vhd \
           psram_testbench.vhd"
