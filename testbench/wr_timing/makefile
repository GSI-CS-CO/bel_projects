GHDL_ALTERA=/home/michael/build/ghdl_vendor_scripts/altera

GHDLFLAGS = --ieee=synopsys --std=93c \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v93      \
	-P$(GHDL_ALTERA)/altera_lnsim/v93\
	-P$(GHDL_ALTERA)/altera_mf/v93   \
	-P$(GHDL_ALTERA)/arriav/v93      \
	-P$(GHDL_ALTERA)/arriaii/v93     \
	-P$(GHDL_ALTERA)/lpm/v93         \
	-P$(GHDL_ALTERA)/sgate/v93       

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

# start simulation (which regenerates wave file), then update viewer
simulation.ghw: compile_testbench 
	ghdl -r testbench --stop-time=10us --wave=simulation.ghw --ieee-asserts=disable
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
	gsettings set com.geda.gtkwave reload 0

compile_testbench: testbench

testbench: 	\
			../common/wr_timing.vhd        \
			testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $?
	ghdl -m $(GHDLFLAGS) testbench 


clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw
