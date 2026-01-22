/*
	Modulated output, demodulated input - as described in DIOB description.
	1 is transmitted as 3.90625 square wave (125MHz/32, T=256ns), 50% duty cycle
	0 is transmitted as continuous 0.
	1 is received for a square wave with Tlow=96-160ns, Thigh=96-160ns (12-20 125MHz clock cycles)
	0 is received otherwise
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "proc_helpers.vh"


module proc_modem #(
		`STD_PROC_PARAMS
	)(
		`STD_PROC_PORTS
	);


//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                     MODULATOR                       *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

reg  [4:0]	tx_counter;
reg			reg_out;

always @(posedge reset, posedge clock)
begin
	if (reset)
	begin
		tx_counter	<= 0;	
		reg_out		<= 0;
	end
	else
	begin
		tx_counter 	<= tx_counter + 1;
		if (virtual_out)
			reg_out	<= tx_counter[4];
		else
			reg_out	<= 0;	
	end
end

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                    DEMODULATOR                      *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

reg  [3:0]	rx_counter;
reg  [2:0]	rx_state;
reg  [1:0]	reg_in_pass;


localparam	RX_STATE_0_EARLY		= 0;	// Checking for minimum 0 period
localparam	RX_STATE_0_FREE			= 1;	// Free to change to 1
localparam	RX_STATE_0_LATE			= 2;	// Period of 0 too long

localparam	RX_STATE_1_EARLY		= 3;	// Checking for minimum 1 period
localparam	RX_STATE_1_FREE			= 4;	// Free to change to 0
localparam	RX_STATE_1_LATE			= 5;	// Period of 1 too long

always @(posedge reset, posedge clock)
begin
	if (reset)
	begin
		rx_counter		<= 0;
		rx_state		<= RX_STATE_0_LATE;
		reg_in_pass		<= 2'b00;
	end
	else
	begin
		case (rx_state)

		RX_STATE_0_EARLY:
		begin
			rx_counter			<= rx_counter - 1;
			if (internal_in)			// Level change too early
			begin
				reg_in_pass[0]	<= 0;
				rx_counter		<= 11;
				rx_state		<= RX_STATE_1_EARLY;
			end
			else if (rx_counter == 0)	// Min. time constraint passed
			begin
				rx_counter		<= 8;
				rx_state		<= RX_STATE_0_FREE;
			end
		end
		
		RX_STATE_0_FREE:
		begin
			rx_counter			<= rx_counter - 1;
			if (rx_counter == 0)	// Max. time constraint violated
			begin
				reg_in_pass[0]	<= 0;
				rx_state		<= RX_STATE_0_LATE;
			end
			else if (internal_in)	// Level change correct
			begin
				reg_in_pass[0]	<= 1;
				rx_counter		<= 11;
				rx_state		<= RX_STATE_1_EARLY;
			end
		end
		
		RX_STATE_0_LATE:
		begin
			if (internal_in)
			begin
				rx_counter	<= 11;
				rx_state	<= RX_STATE_1_EARLY;
			end		
		end
		
		RX_STATE_1_EARLY:
		begin
			rx_counter			<= rx_counter - 1;
			if (!internal_in)			// Level change too early
			begin
				reg_in_pass[1]	<= 0;
				rx_counter		<= 11;
				rx_state		<= RX_STATE_0_EARLY;
			end
			else if (rx_counter == 0)	// Min. time constraint passed
			begin
				rx_counter		<= 8;
				rx_state		<= RX_STATE_1_FREE;
			end
		end
		
		RX_STATE_1_FREE:
		begin
			rx_counter			<= rx_counter - 1;
			if (rx_counter == 0)	// Max. time constraint violated
			begin
				reg_in_pass[1]	<= 0;
				rx_state		<= RX_STATE_1_LATE;
			end
			else if (!internal_in)	// Level change correct
			begin
				reg_in_pass[1]	<= 1;
				rx_counter		<= 11;
				rx_state		<= RX_STATE_0_EARLY;
			end
		end

		RX_STATE_1_LATE:
		begin
			if (!internal_in)
			begin
				rx_counter	<= 12;
				rx_state	<= RX_STATE_0_EARLY;
			end		
		end

		endcase
	end
end


//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                  I/O ASSIGNMENTS                    *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 


assign internal_out 	= reg_out;
assign virtual_in 		= &reg_in_pass;
assign output_enable	= 1;
assign input_enable		= 1;


endmodule