
module temp_sens (
	clk,
	tsdcalo,
	tsdcaldone,
	ce,
	clr);	

	input		clk;
	output	[7:0]	tsdcalo;
	output		tsdcaldone;
	input		ce;
	input		clr;
endmodule
