/*
	Quick input debouncer.
	Output is passed.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "proc_helpers.vh"


module proc_in_debounce #(
		`STD_PROC_PARAMS,
			//Additional parameter(s): configured via json.
		parameter		nr_stages 			= 4
	)(
		`STD_PROC_PORTS	
	);



reg [nr_stages : 0] reg_in;
reg	reg_out;

always @(posedge clock, posedge reset)
	if (reset)
	begin
		reg_in 	<= 0;
		reg_out <= 0;
	end
	else
	begin
		reg_in[nr_stages:1]	<= reg_in[nr_stages-1:0];
		reg_in[0]			<= internal_in;
		
		if (&reg_in)			//all ones
			reg_out			<= 1;
		else if (!(|reg_in))	//all zeros
			reg_out 		<= 0;	
	end

assign internal_out 	= virtual_out;
assign virtual_in 		= reg_out;
assign output_enable	= 1;
assign input_enable		= 1;


endmodule