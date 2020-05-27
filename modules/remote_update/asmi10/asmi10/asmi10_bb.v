
module asmi10 (
	addr,
	busy,
	clkin,
	data_valid,
	datain,
	dataout,
	en4b_addr,
	fast_read,
	illegal_erase,
	illegal_write,
	rden,
	rdid_out,
	read_address,
	read_rdid,
	read_status,
	reset,
	sce,
	sector_erase,
	shift_bytes,
	status_out,
	wren,
	write,
	read_dummyclk);	

	input	[31:0]	addr;
	output		busy;
	input		clkin;
	output		data_valid;
	input	[7:0]	datain;
	output	[7:0]	dataout;
	input		en4b_addr;
	input		fast_read;
	output		illegal_erase;
	output		illegal_write;
	input		rden;
	output	[7:0]	rdid_out;
	output	[31:0]	read_address;
	input		read_rdid;
	input		read_status;
	input		reset;
	input	[2:0]	sce;
	input		sector_erase;
	input		shift_bytes;
	output	[7:0]	status_out;
	input		wren;
	input		write;
	input		read_dummyclk;
endmodule
