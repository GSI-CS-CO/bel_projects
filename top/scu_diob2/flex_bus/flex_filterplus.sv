/*
	A better address filter for flexible peripherals.
	Pass only cycles with given address range.
	This is NOT for reducing address decoding logic at slaves, but for providing safety
	by passing only a range of addersses
	Allows free defining of:
		- Lower and upper address bound
		- Address bus width: typ. 16 bits
	
	Address bounds are not constrained - can be any number.
	
	Only affected bus signals are routed through this module.
	The logic is purely combinatorial, with no registers. 
	Typically, this block will be paired with flex_hub.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"

module flex_filterplus #(
		parameter		addr_bus_width 	= `BB_ADDR_BUS_WIDTH,
		parameter		lower_bound		= 0,
		parameter		higher_bound 	= (1<<addr_bus_width) - 1
	)(
		input	wire	clock,             							//
		input	wire	reset,             							//   
		
			//Connection to bus master
		input	wire	[addr_bus_width-1 : 0]	addr,				// (Adr_from_SCUB_LA)
		input	wire	addr_strobe,								// (Ext_Adr_Val)

			//Connection to slave(s)
		output	wire	sec_addr_strobe								// (Ext_Adr_Val)
	);
	
	// Calculate if address matches base_addr
wire addr_match;

assign addr_match = ((addr >= lower_bound) && (addr <= higher_bound)) ? 1'b1 : 1'b0;
assign sec_addr_strobe = addr_match & addr_strobe;

endmodule	

	