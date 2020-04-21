#include <verilated.h>          // Defines common routines
#include "Vlm32_top.h"               // From Verilating "top.v"

#if VM_TRACE
# include <verilated_vcd_c.h>   // Trace file format header
#endif


#include <iostream>


Vlm32_top *top;                      // Instantiation of module

vluint64_t main_time = 0;       // Current simulation time
// This is a 64-bit integer to reduce wrap over issues and
// allow modulus.  You can also use a double, if you wish.

double sc_time_stamp () {       // Called by $time in Verilog
    return main_time;           // converts to double, to match
                                // what SystemC does
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);   // Remember args

    top = new Vlm32_top;             // Create instance
#if VM_TRACE            // If verilator was invoked with --trace
    Verilated::traceEverOn(true);   // Verilator must compute traced signals
    VL_PRINTF("Enabling waves...\n");
    VerilatedVcdC* tfp = new VerilatedVcdC();
    top->trace (tfp, 199);   // Trace 99 levels of hierarchy
    tfp->open ("vlt_dump.vcd"); // Open the dump file
#endif



    top->rst_i = 1;           // Set some inputs
    top->clk_i = 1;
    //while (!Verilated::gotFinish()) {
    while (main_time < 10000) {
        if (main_time > 50) {
            top->rst_i = 0;   // Deassert reset
        }
        top->clk_i = !top->clk_i; // Toggle clock
        top->eval();            // Evaluate model
        //std::cout << top->clk_i << std::endl;       // Read a output
        top->I_ACK_I = top->I_STB_O;
        top->I_DAT_I = top->I_ADR_O;
#if VM_TRACE
    // if ((main_time > 1000 && main_time < 2000)  ||
    //     (main_time > 3000 && main_time < 4000))
    if (tfp) tfp->dump (main_time); // Create waveform trace for this timestamp
#endif
        main_time++;            // Time passes...
    }

#if VM_TRACE
    if (tfp) tfp->close();
#endif

    top->final();               // Done simulating
    //    // (Though this example doesn't get here)
    delete top;
}
