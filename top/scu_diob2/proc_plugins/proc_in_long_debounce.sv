/*
	Adjustable-length input debouncer.
	Output is passed directly.
	In contrast to "quick" debounce, a state machine is used here to guarantee minimum constant level duration.
	Logarithmic complexity vs debounce time allows for longer periods.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "proc_helpers.vh"


module proc_in_long_debounce #(
		`STD_PROC_PARAMS,
			//Additional parameter(s): configured via json.
		parameter		count_ticks			= 125000	//Number of clock ticks for the input signal to be stable
	)(
		input	wire	clock,             	//   
		input	wire	reset,             	//   

		input	wire	plugin_enable,		// Information if the plugin is in use at the time  

		output	wire	internal_out,		// Connection to frontend plugin part  
		input	wire	internal_in,		// Connection to frontend plugin part  
		
		input	wire	virtual_out,		// Connection to user  
		output	wire	virtual_in,			// Connection to user  

		output	wire	output_enable,    	// Information that channel may be used as output  
		output	wire	input_enable    	// Information that channel may be used as input  	
	);

localparam counter_bits = $clog2(count_ticks);

reg [counter_bits-1 : 0]	counter;
reg [1:0]					state;
reg	reg_out;

localparam STATE_STABLE_0	= 0;
localparam STATE_STABLE_1	= 1;
localparam STATE_CHECK_0	= 2;
localparam STATE_CHECK_1	= 3;

/*
	STATE DIAGRAM

	STABLE_0         STABLE_1
	  ^  ^             ^  ^
	  |  |             |  |
	  |  \-----\ /-----/  |
	  |         X         |
	  |  \-----/ \-----\  |
	  |  |             |  |
	  |  v             v  | 
	 CHECK_0          CHECK_1
*/

always @ (posedge reset, posedge clock)
begin
	if (reset)
	begin
		state	<= STATE_STABLE_0;
		reg_out	<= 0;
	end
	else
	begin
		case (state)
		
			STATE_STABLE_0:
			begin
				if (internal_in)
				begin
					counter <= count_ticks;
					state	<= STATE_CHECK_1;				
				end		
			end
			
			STATE_STABLE_1:
			begin
				if (!internal_in)
				begin
					counter <= count_ticks;
					state	<= STATE_CHECK_0;				
				end		
			end		

			STATE_CHECK_0:
			begin
				counter		<= counter - 1;
				if (internal_in)
				begin
					state	<= STATE_STABLE_1;				
				end		
				else if (counter == 0)
				begin
					reg_out	<= 0;
					state	<= STATE_STABLE_0;
				end
			end
		
			STATE_CHECK_1:
			begin
				counter		<= counter - 1;
				if (!internal_in)
				begin
					state	<= STATE_STABLE_0;				
				end		
				else if (counter == 0)
				begin
					reg_out	<= 1;
					state	<= STATE_STABLE_1;
				end
			end
	
		endcase
	end



end

assign internal_out 	= virtual_out;
assign virtual_in 		= reg_out;
assign output_enable	= 1;
assign input_enable		= 1;


endmodule