#include <verilated.h>          // Defines common routines
#include "Vsockit_owm.h"          // From Verilating "lm32_top.v"

#if VM_TRACE
# include <verilated_vcd_c.h>   // Trace file format header
#endif

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstdint>
#include <climits>
#include <cstdio>

#define VARNAME (work__file_access__my_var)
extern int VARNAME;

// Container for all sockit_owm instances that will ever be instantiated
// Users will work with an index into this container.
std::vector<Vsockit_owm*> top_instances;
std::vector<VerilatedVcdC*> tfp_instances;

vluint64_t main_time = 0;       // Current simulation time
// This is a 64-bit integer to reduce wrap over issues and
// allow modulus.  You can also use a double, if you wish.
double sc_time_stamp () {       // Called by $time in Verilog
    return main_time;           // converts to double, to match
                                // what SystemC does
}

// GHDL interface
extern "C" {
	int interface_sockit_owm_init(int *pts) {
		int idx = top_instances.size();
		top_instances.push_back(new Vsockit_owm);
	    Verilated::traceEverOn(true);   // Verilator must compute traced signals
	    tfp_instances.push_back(new VerilatedVcdC);
	    top_instances[idx]->trace(tfp_instances[idx], 99);   // Trace 99 levels of hierarchy
	    std::ostringstream filename;
	    filename << "vlt_dump_sockit_owm_" << std::setw(2) << std::setfill('0') << std::dec << idx << ".vcd";
    	tfp_instances[idx]->open(filename.str().c_str()); // Open the dump file
		//std::cout << "interface_sockit_owm_init in C++ called. returing index " << idx << std::endl;
		return idx;
	}

	// set clk and call eval, i.e. this has to be the last of all calls in a clock cycle
    // VL_IN8(clk,0,0);
	void interface_sockit_owm_clk(int idx, int clk) {
		top_instances[idx]->eval();
		// std::cerr << "clk[" << idx << "] = " << clk << std::endl;
		top_instances[idx]->clk = clk;
	    if (tfp_instances[idx]) tfp_instances[idx]->dump(main_time); // Create waveform trace for this timestamp
	    main_time += 8;
	}

    // VL_IN8(rst,0,0);
	void interface_sockit_owm_rst(int idx, int rst) {
		top_instances[idx]->rst = rst;
	}

    // VL_IN8(bus_ren,0,0);
	void interface_sockit_owm_bus_ren(int idx, int bus_ren) {
		top_instances[idx]->bus_ren = bus_ren;
	}
    // VL_IN8(bus_wen,0,0);
	void interface_sockit_owm_bus_wen(int idx, int bus_wen) {
		// if (bus_wen != 0) {
		// 	printf("C++: interface_sockit_owm_bus_wen(%d,%d)\n", idx, bus_wen);
		// }
		top_instances[idx]->bus_wen = bus_wen;
	}
    // VL_IN8(bus_adr,0,0);
	void interface_sockit_owm_bus_adr(int idx, int bus_adr) {
		top_instances[idx]->bus_adr = bus_adr;
	}


    // VL_OUT8(bus_irq,0,0);
	int interface_sockit_owm_bus_irq(int idx) {
		return top_instances[idx]->bus_irq;
	}
    // VL_OUT8(owr_p,0,0);
	int interface_sockit_owm_owr_p(int idx) {
		return top_instances[idx]->owr_p;
	}
    // VL_OUT8(owr_e,0,0);
	int interface_sockit_owm_owr_e(int idx) {
		return top_instances[idx]->owr_e;
	}
    // VL_IN8(owr_i,0,0);
	void interface_sockit_owm_owr_i(int idx, int owr_i) {
		top_instances[idx]->owr_i = owr_i;
	}
    // VL_IN(bus_wdt,31,0);
	void interface_sockit_owm_bus_wdt(int idx, int bus_wdt) {
		top_instances[idx]->bus_wdt = bus_wdt-INT_MIN;
	}
    // VL_OUT(bus_rdt,31,0);
	int interface_sockit_owm_bus_rdt(int idx) {
		return top_instances[idx]->bus_rdt+INT_MIN;
	}

}
