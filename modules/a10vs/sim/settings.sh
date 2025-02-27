# Simulation settings
TB_NAME="a10vs_tb"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic -g"
STOP_TIME="1ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

# Files
GENCORE_PATH="../../../ip_cores/general-cores"
A10VS_PATH="../src/hdl"

VHD_PACK="$GENCORE_PATH/modules/genrams/genram_pkg.vhd \
          $GENCORE_PATH/modules/wishbone/wishbone_pkg.vhd \
          $GENCORE_PATH/modules/common/gencores_pkg.vhd"

SIM_FILES="avalon_vs.vhd \
            avalon_vs_pkg.vhd"

VHD_FILES="$VHD_PACK \
           $SIM_FILES \
           $A10VS_PATH/a10vs_pkg.vhd \
           $A10VS_PATH/a10vs.vhd \
           $A10VS_PATH/a10vs_tb.vhd"

# Functions
remove_old_files() {
  for item in $TB_NAME *.o *.vcd *.cf *.ghw
  do
    echo "Removing $item ..."
    rm $item > /dev/null 2>&1
  done
}