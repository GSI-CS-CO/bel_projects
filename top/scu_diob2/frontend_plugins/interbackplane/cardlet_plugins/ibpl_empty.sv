/*
	Default for empty/unknown cardlet
*/

`timescale 1 ps / 1 ps
`include "../../../blackbox_defines.vh"
`include "../../../blackbox_config.vh"
`include "../interbackplane_helpers.vh"


module ibpl_empty #(
		`STD_CARDLET_PARAMS
	)(
		`STD_CARDLET_PORTS
	);
	
assign diob_dir		= 6'h00;
assign diob_out		= 6'h00;
assign internal_in	= 8'h00;
assign diob_led2 	= 8'h00;
assign diob_led1 	= 8'h00;

	// Generate module error if ANY of the signals has output enabled or input enabled
assign plugin_error = |((output_enable | input_enable) & 'h3F);

endmodule