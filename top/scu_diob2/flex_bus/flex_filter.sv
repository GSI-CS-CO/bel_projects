/*
	Address filter for flexible peripherals.
	Pass only cycles with given address range.
	This is for reducing address decoding logic at slaves.
	Allows free defining of:
		- Base address
		- Number of address bits to pass to slaves
		- Address bus width: typ. 16 bits
	
	Base address MUST be aligned at a power of 2. 
	<bits_to_pass> LSBs of address are passed to slaves along with preset base_addr in MSBs. 
	and remaining MSBs are compared against base address to select the device.
	
	Only affected bus signals are routed through this module.
	The logic is purely combinatorial, with no registers. 
	Typically, this block will be paired with flex_hub.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"

module flex_filter #(
		parameter		addr_bus_width 	= `BB_ADDR_BUS_WIDTH,
		parameter		base_addr		= 0,
		parameter		bits_to_pass 	= addr_bus_width
	)(
		input	wire	clock,             							//
		input	wire	reset,             							//   
		
			//Connection to bus master
		input	wire	[addr_bus_width-1 : 0]	addr,				// (Adr_from_SCUB_LA)
		input	wire	addr_strobe,								// (Ext_Adr_Val)

			//Connection to slave(s)
		output	wire	[addr_bus_width-1 : 0]	sec_addr,			// (Adr_from_SCUB_LA)
		output	wire	sec_addr_strobe								// (Ext_Adr_Val)
	);
	
	// Calculate if address matches base_addr
wire addr_match;
generate
	if (bits_to_pass == addr_bus_width)	//Pass everything
		assign addr_match = 1;
	else	
		assign addr_match = (addr[addr_bus_width-1 : bits_to_pass] == base_addr[addr_bus_width-1 : bits_to_pass]) ? 1 : 0;
endgenerate
	
	// Assign outputs
generate
	if (bits_to_pass == addr_bus_width)	//Pass everything
	begin
		assign sec_addr = addr;
		assign sec_addr_strobe = addr_strobe;
	end
	else
	begin
		assign sec_addr = { base_addr[addr_bus_width-1 : bits_to_pass], addr[bits_to_pass-1 : 0] };
		assign sec_addr_strobe = addr_strobe && addr_match;
	end
endgenerate	

endmodule	

	