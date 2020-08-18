#include <verilated.h>          // Defines common routines
#include "Vlm32_top.h"          // From Verilating "lm32_top.v"

#if VM_TRACE
# include <verilated_vcd_c.h>   // Trace file format header
#endif

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstdint>
#include <climits>

#define VARNAME (work__file_access__my_var)
extern int VARNAME;

// Container for all lm32 instances that will ever be instantiated
// Users will work with an index into this container.
std::vector<Vlm32_top*> top_instances;
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
	int interface_lm32_init(int *pts) {
		int idx = top_instances.size();
		top_instances.push_back(new Vlm32_top);
	    Verilated::traceEverOn(true);   // Verilator must compute traced signals
	    tfp_instances.push_back(new VerilatedVcdC);
	    top_instances[idx]->trace(tfp_instances[idx], 99);   // Trace 99 levels of hierarchy
	    std::ostringstream filename;
	    filename << "vlt_dump_" << std::setw(2) << std::setfill('0') << std::dec << idx << ".vcd";
    	tfp_instances[idx]->open(filename.str().c_str()); // Open the dump file
		//std::cout << "interface_lm32_init in C++ called. returing index " << idx << std::endl;
		return idx;
	}

	// set clk and call eval, i.e. this has to be the last of all calls in a clock cycle
	void interface_lm32_clk(int idx, int clk) {
		top_instances[idx]->eval();
		//std::cerr << "clk[" << idx << "] = " << clk << std::endl;
		top_instances[idx]->clk_i = clk;
	    if (tfp_instances[idx]) tfp_instances[idx]->dump (main_time); // Create waveform trace for this timestamp
	    main_time += 4;
	}
	void interface_lm32_rst(int idx, int rst) {
		top_instances[idx]->rst_i = rst;
	}

	void interface_lm32_sym_interrupt(int idx, int interrupt) {
		top_instances[idx]->__SYM__interrupt = interrupt;
	}

	// instruction wb interface output
	int interface_lm32_i_adr_o(int idx) {
		return top_instances[idx]->I_ADR_O;
	}
	int interface_lm32_i_cyc_o(int idx) {
		return top_instances[idx]->I_CYC_O;
	}
	int interface_lm32_i_stb_o(int idx) {
		return top_instances[idx]->I_STB_O;
	}
	int interface_lm32_i_dat_o(int idx) {
		return top_instances[idx]->I_DAT_O;
	}
	int interface_lm32_i_sel_o(int idx) {
		return top_instances[idx]->I_SEL_O;
	}
	int interface_lm32_i_we_o(int idx) {
		return top_instances[idx]->I_WE_O;
	}
	int interface_lm32_i_cti_o(int idx) {
		return top_instances[idx]->I_CTI_O;
	}
	int interface_lm32_i_lock_o(int idx) {
		return top_instances[idx]->I_LOCK_O;
	}
	int interface_lm32_i_bte_o(int idx) {
		return top_instances[idx]->I_BTE_O;
	}
	// instruction wb interface input
	void interface_lm32_i_ack_i(int idx, int ack) {
		top_instances[idx]->I_ACK_I = ack;
	}
	void interface_lm32_i_dat_i(int idx, int dat) {
		top_instances[idx]->I_DAT_I = dat;
	}
	void interface_lm32_i_err_i(int idx, int err) {
		top_instances[idx]->I_ERR_I = err;
	}
	void interface_lm32_i_rty_i(int idx, int rty) {
		top_instances[idx]->I_RTY_I = rty;
	}


	// data wb interface output
	int interface_lm32_d_adr_o(int idx) {
		return (int)(top_instances[idx]->D_ADR_O);
	}
	int interface_lm32_d_cyc_o(int idx) {
		return top_instances[idx]->D_CYC_O;
	}
	int interface_lm32_d_stb_o(int idx) {
		return top_instances[idx]->D_STB_O;
	}
	int interface_lm32_d_dat_o(int idx) {
		return top_instances[idx]->D_DAT_O;
	}
	int interface_lm32_d_sel_o(int idx) {
		return top_instances[idx]->D_SEL_O;
	}
	int interface_lm32_d_we_o(int idx) {
		return top_instances[idx]->D_WE_O;
	}
	int interface_lm32_d_cti_o(int idx) {
		return top_instances[idx]->D_CTI_O;
	}
	int interface_lm32_d_lock_o(int idx) {
		return top_instances[idx]->D_LOCK_O;
	}
	int interface_lm32_d_bte_o(int idx) {
		return top_instances[idx]->D_BTE_O;
	}
	// data wb interface input
	void interface_lm32_d_ack_i(int idx, int ack) {
		top_instances[idx]->D_ACK_I = ack;
	}
	void interface_lm32_d_dat_i(int idx, int dat) {
		top_instances[idx]->D_DAT_I = dat;
	}
	void interface_lm32_d_err_i(int idx, int err) {
		top_instances[idx]->D_ERR_I = err;
	}
	void interface_lm32_d_rty_i(int idx, int rty) {
		top_instances[idx]->D_RTY_I = rty;
	}
}
