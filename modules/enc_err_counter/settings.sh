# Simulation settings
TB_NAME="enc_err_counter_tb"
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
		  enc_err_counter_pkg.vhd"
VHD_FILES="$VHD_PACK \
           ../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
           ../../ip_cores/general-cores/modules/genrams/generic/generic_async_fifo.vhd \
           ../../ip_cores/general-cores/modules/genrams/common/inferred_async_fifo.vhd \
           ../../ip_cores/general-cores/modules/common/gc_sync_register.vhd \
           ../../ip_cores/general-cores/modules/common/gc_sync_ffs.vhd \
           enc_err_counter_tb.vhd \
           enc_err_counter.vhd"
           
		   #ip_cores/general-cores/modules/genrams/generic/generic_async_fifo.vhd \
           #ip_cores/general-cores/modules/genrams/common/inferred_async_fifo.vhd \
