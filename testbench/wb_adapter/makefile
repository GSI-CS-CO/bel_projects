GHDL_ALTERA=/home/michael/build/ghdl_vendor_scripts/altera
GHDL_XILINX=/home/michael/build/ghdl_vendor_scripts/xilinx-ise

GHDLFLAGS = --ieee=synopsys --std=93c \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v93       \
	-P$(GHDL_ALTERA)/altera_lnsim/v93 \
	-P$(GHDL_ALTERA)/altera_mf/v93    \
	-P$(GHDL_ALTERA)/arriav/v93       \
	-P$(GHDL_XILINX)/secureip/v93     \
	-P$(GHDL_XILINX)/simprim/v93      \
	-P$(GHDL_XILINX)/unimacro/v93     \
	-P$(GHDL_XILINX)/unisim/v93       \
	-P$(GHDL_XILINX)/xilinxcorelib/v93

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

run:
	ghdl -r testbench --stop-time=100ns --wave=simulation.ghw --ieee-asserts=disable #--read-wave-opt=simulation_wave.opt
	gsettings set com.geda.gtkwave reload 0

notrace:
	ghdl -r testbench --ieee-asserts=disable

# start simulation (which regenerates wave file), then update viewer
simulation.ghw: testbench run

testbench: 	\
			genram_pkg.vhd \
			wishbone_pkg.vhd \
			wb_slave_adapter.vhd   \
			testbench.vhd
	ghdl -a $(GHDLFLAGS) $?
	ghdl -m $(GHDLFLAGS) testbench 

clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw 
