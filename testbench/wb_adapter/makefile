VENDOR_LIBS=/home/michael/build/ghdl_vendor_scripts
GHDL_ALTERA=$(VENDOR_LIBS)/altera

GHDLFLAGS = --ieee=synopsys --std=08 \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v93       \
	-P$(GHDL_ALTERA)/altera_lnsim/v93 \
	-P$(GHDL_ALTERA)/altera_mf/v93    \
	-P$(GHDL_ALTERA)/arriav/v93       

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

run:
	ghdl -r testbench --stop-time=10000ns --wave=simulation.ghw --ieee-asserts=disable #--read-wave-opt=simulation_wave.opt
	gsettings set com.geda.gtkwave reload 0

notrace:
	ghdl -r testbench --ieee-asserts=disable

# start simulation (which regenerates wave file)
simulation.ghw: testbench run

# 			genram_pkg.vhd \

testbench: 	\
			wishbone_pkg.vhd \
			wb_slave_adapter.vhd   \
			testbench.vhd
	ghdl -a $(GHDLFLAGS) $?
	ghdl -m $(GHDLFLAGS) testbench 

proof: 
	sby --yosys "yosys -m ghdl" -f wb_adapter_bmc.sby


clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw 
