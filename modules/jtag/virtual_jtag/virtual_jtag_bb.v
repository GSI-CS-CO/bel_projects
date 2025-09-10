
module virtual_jtag (
	tdi,
	tdo,
	ir_in,
	ir_out,
	virtual_state_cdr,
	virtual_state_sdr,
	virtual_state_e1dr,
	virtual_state_pdr,
	virtual_state_e2dr,
	virtual_state_udr,
	virtual_state_cir,
	virtual_state_uir,
	tms,
	jtag_state_tlr,
	jtag_state_rti,
	jtag_state_sdrs,
	jtag_state_cdr,
	jtag_state_sdr,
	jtag_state_e1dr,
	jtag_state_pdr,
	jtag_state_e2dr,
	jtag_state_udr,
	jtag_state_sirs,
	jtag_state_cir,
	jtag_state_sir,
	jtag_state_e1ir,
	jtag_state_pir,
	jtag_state_e2ir,
	jtag_state_uir,
	tck);	

	output		tdi;
	input		tdo;
	output	[0:0]	ir_in;
	input	[0:0]	ir_out;
	output		virtual_state_cdr;
	output		virtual_state_sdr;
	output		virtual_state_e1dr;
	output		virtual_state_pdr;
	output		virtual_state_e2dr;
	output		virtual_state_udr;
	output		virtual_state_cir;
	output		virtual_state_uir;
	output		tms;
	output		jtag_state_tlr;
	output		jtag_state_rti;
	output		jtag_state_sdrs;
	output		jtag_state_cdr;
	output		jtag_state_sdr;
	output		jtag_state_e1dr;
	output		jtag_state_pdr;
	output		jtag_state_e2dr;
	output		jtag_state_udr;
	output		jtag_state_sirs;
	output		jtag_state_cir;
	output		jtag_state_sir;
	output		jtag_state_e1ir;
	output		jtag_state_pir;
	output		jtag_state_e2ir;
	output		jtag_state_uir;
	output		tck;
endmodule
