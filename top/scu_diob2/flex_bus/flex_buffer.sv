/*
	Synchronous buffer for flexible peripheral bus.
	To be used if excessive amount of combinatorial logic causes timing errors.
	Allows free defining of:
		- Buffering direction(s)
		- Address bus width: typ. 16 bits
		- Data bus width: typ. 16 bits
		
	Tis unit doesn't perform any logic operations on signals. 
	All down- and/or upstream signals get delayed by one clock cycle.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"

module flex_buffer #(
		parameter		addr_bus_width 	= `BB_ADDR_BUS_WIDTH,
		parameter		data_bus_width 	= `BB_DATA_BUS_WIDTH,
		parameter		buf_downstream	= 1,					//Buffer master-to-slave signals (0/1)
		parameter		buf_upstream	= 1						//Buffer slave-to-master signals (0/1)
	)(
		input	wire	clock,             						//
		input	wire	reset,             						//   
		
			//Connection to bus master
		input	wire	[addr_bus_width-1 : 0]	addr,			// (Adr_from_SCUB_LA)
		input	wire	[data_bus_width-1 : 0]	data_w,			// (Data_from_SCUB_LA)
		output	wire	[data_bus_width-1 : 0]	data_r,			// (Data_to_SCUB)

		input	wire	addr_strobe,							// (Ext_Adr_Val)
		input	wire	read_trg,								// (Ext_Rd_active)
		input	wire	write_trg,								// (Ext_Wr_active)
		input	wire	read_fin,								// (Ext_Rd_fin)
		input	wire	write_fin,								// (Ext_Wr_fin)
		input	wire	event_trg,								//

		output 	wire	dtack,									// (Dtack_to_SCUB)
		output 	wire	data_r_act,								// (Reg_rd_active)

			//Connection to slave(s)
		output	wire	[addr_bus_width-1 : 0]	sec_addr,		// (Adr_from_SCUB_LA)
		output	wire	[data_bus_width-1 : 0]	sec_data_w,		// (Data_from_SCUB_LA)
		input	wire	[data_bus_width-1 : 0]	sec_data_r,		// (Data_to_SCUB)

		output	wire	sec_addr_strobe,						// (Ext_Adr_Val)
		output	wire	sec_read_trg,							// (Ext_Rd_active)
		output	wire	sec_write_trg,							// (Ext_Wr_active)
		output	wire	sec_read_fin,							// (Ext_Rd_fin)
		output	wire	sec_write_fin,							// (Ext_Wr_fin)
		output	wire	sec_event_trg,							//

		input 	wire	sec_dtack,								// (Dtack_to_SCUB)
		input 	wire	sec_data_r_act							// (Reg_rd_active)
	);
	
		

generate

if (buf_downstream)
begin
	reg	[addr_bus_width-1 : 0]	reg_addr;
	reg	[data_bus_width-1 : 0]	reg_data_w;
	reg reg_addr_strobe;
	reg	reg_read_trg;
	reg	reg_write_trg;
	reg	reg_read_fin;
	reg	reg_write_fin;
	reg reg_event_trg;

	always @(posedge clock)
	begin
		reg_addr		<= addr;
		reg_data_w		<= data_w;
		reg_addr_strobe	<= addr_strobe;
		reg_read_trg	<= read_trg;
		reg_write_trg	<= write_trg;
		reg_read_fin	<= read_fin;
		reg_write_fin	<= write_fin;
		reg_event_trg	<= event_trg;
	end

	assign sec_addr 		= reg_addr;
	assign sec_data_w 		= reg_data_w;
	assign sec_addr_strobe	= reg_addr_strobe;
	assign sec_read_trg		= reg_read_trg;
	assign sec_write_trg	= reg_write_trg;
	assign sec_read_fin		= reg_read_fin;
	assign sec_write_fin	= reg_write_fin;
	assign sec_event_trg	= reg_event_trg;
end
else
begin
	assign sec_addr 		= addr;
	assign sec_data_w 		= data_w;
	assign sec_addr_strobe	= addr_strobe;
	assign sec_read_trg		= read_trg;
	assign sec_write_trg	= write_trg;
	assign sec_read_fin		= read_fin;
	assign sec_write_fin	= write_fin;
	assign sec_event_trg	= event_trg;
end

if (buf_upstream)
begin
	reg	[data_bus_width-1 : 0]	reg_data_r;
	reg	reg_dtack;
	reg	reg_data_r_act;

	always @(posedge clock)
	begin
		reg_data_r		<= sec_data_r;
		reg_dtack		<= sec_dtack;
		reg_data_r_act	<= sec_data_r_act;
	end

	assign data_r			= reg_data_r;
	assign dtack			= reg_dtack;
	assign data_r_act		= reg_data_r_act;
end
else
begin
	assign data_r			= sec_data_r;
	assign dtack			= sec_dtack;
	assign data_r_act		= sec_data_r_act;
end

endgenerate

endmodule	

	