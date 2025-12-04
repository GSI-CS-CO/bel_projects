/*
	A typical 5in-1out cardlet
*/

`timescale 1 ps / 1 ps
`include "../../blackbox_defines.vh"
`include "../../blackbox_config.vh"
`include "interbackplane_helpers.vh"


module ibpl_5in1out #(
		`STD_CARDLET_PARAMS
	)(
		`STD_CARDLET_PORTS
	);
	
assign diob_dir		= 6'h20;
assign diob_out		= {internal_out[5], 5'b0};
assign internal_in	= {3'b0, diob_in[4:0]};
assign diob_led1	= {2'b0, output_enable[5], input_enable[4:0]};
assign diob_led2	= {2'b0, output_act[5], input_act[4:0]};

	// Generate module error if ay of signals 4-0 has output enabled but input disabled, ...
	// or channel 5 has input enabled but output disabled
assign plugin_error = 	( |((output_enable & ~input_enable) & 'h1F) ) | 
						( |((input_enable & ~output_enable) & 'h20) );

endmodule