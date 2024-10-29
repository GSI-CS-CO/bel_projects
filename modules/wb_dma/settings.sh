# Simulation settings
TB_NAME="wb_dma_tb"
GHDL_BIN="ghdl"
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic"
STOP_TIME="1ms"
VCD_NAME="$TB_NAME.vcd"
GHW_NAME="$TB_NAME.ghw"
VCD_VIEWER="gtkwave"
GTKW_NAME="$TB_NAME.gtkw"

# Files
VHD_PACK="../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd \
		  ../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd \
		  ../../ip_cores/general-cores/modules/common/gencores_pkg.vhd \
		  wb_dma_pkg.vhd"
VHD_FILES="$VHD_PACK \
           ../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
           generic_sync_fifo_m.vhd \
           inferred_sync_fifo_m.vhd \
           generic_dpram_m.vhd \
           simple_dpram.vhd \
           wb_dma_ch_rf.vhd \
           wb_dma_engine.vhd \
           wb_dma.vhd \
           wb_dma_tb.vhd"
