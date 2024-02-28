// gate_deglitcher.v
/*
	A simple synchronous signal deglitcher with programmable number of stages
	Each stage is a D register and they are connected in series.
	The output state changes only if the state of all stages is uniform.
	This means, that the signal must be stable at least for (nr_stages) clock cycles
	to change the output state.
	
Module parameters:
	nr_stages: Number of series flip-flops in the deglitcher.	
*/


`timescale 1 ps / 1 ps
module gate_deglitcher #(
		parameter	nr_stages = 10
	)(
		input  wire       clock,           
		input  wire       reset,           
		input  wire 	  degl_in,
		output wire       degl_out
	);


reg	[nr_stages-1:0] queue;
reg					reg_out;

always @ (posedge reset, posedge clock)
begin
	if (reset)
	begin
		queue 				<= ~0;
		reg_out				<= 0;
	end
	else
	begin
			//Shift queue
		//queue[nr_stages-1:1]<= queue[nr_stages-2:0];
		
		queue[0]			<= degl_in;	
		
			//Switch output
		if (reg_out == 0)		//L->H
		begin
			if (&queue)			//if all ones
				reg_out 		<= 1;
		end
		else					//H->L
		begin
			if (~|queue)		//if all zeros
				reg_out 		<= 0;
		end
	end
end

assign degl_out = reg_out;


endmodule


