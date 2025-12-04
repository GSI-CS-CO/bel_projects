/*
	Hub for flexible peripherals.
	Integrates upstream signals(data_r, data_r_act and dtack) from multiple devices
	Allows free defining of:
		- Number of slaves
		- Data bus (register) width: typ. 16 bits
	
	Only affected bus signals are routed through this module.
	The logic is purely combinatorial, with no registers. Data routing depends on sec_data_r_act signals.
	Flattened sec_data_r bus is assigned so that LSBs belong to data_r_act[0] MSBs belong to data_r_act[nr_slaves-1].
	Use `slice macro (see blackbox_defines.vh) to help getting the right bits.
	
	There is no error checking. If multiple devices assert data_r_act, the LSB-closest one will be routed.
	sec_data_r_act and sec_dtack are just group-ored respectively to data_r_act and dtack and independently of each other.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"

module flex_hub #(
		parameter		data_bus_width 	= `BB_DATA_BUS_WIDTH,
		parameter		nr_slaves 		= 2
	)(
		input	wire	clock,             									//
		input	wire	reset,             									//   

			//Master connection
		output	wire	[data_bus_width-1 : 0]	data_r,						// (Data_to_SCUB)
		output 	wire	dtack,												// (Dtack_to_SCUB)
		output 	wire	data_r_act,											// (Reg_rd_active)

			//Slave connections
		input		wire	[nr_slaves*data_bus_width-1 : 0]	sec_data_r,		// (Data_to_SCUB)
		input 	wire	[nr_slaves-1 : 0]					sec_dtack,		// (Dtack_to_SCUB)
		input 	wire	[nr_slaves-1 : 0]					sec_data_r_act	// (Reg_rd_active)
	);
	
	// Number of bits used for slave selection
localparam	sec_sel_bits = $clog2(nr_slaves);

	//Selector
reg [sec_sel_bits-1 : 0]	selector;

integer i;
always @(*) begin
	selector = 0;
	for (i = 0; i < nr_slaves; i = i + 1)
		if (sec_data_r_act[i])
			selector = i;
end

assign data_r = sec_data_r[(selector+1)*data_bus_width - 1 -: data_bus_width];
assign data_r_act = |sec_data_r_act;
assign dtack = |sec_dtack;
	
endmodule
	