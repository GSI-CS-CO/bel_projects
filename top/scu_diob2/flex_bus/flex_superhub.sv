/*
	'Superhub' for flexible peripherals.
	It integrates a filter, a buffer and a hub in a single component.
	
	Note that buffering is switched off by default (buf_downstream == 0, buf_upstream == 0).
	Also, by default there is no filtering (bits_to_pass == addr_bus_width).
	Consequently, there's also no hub (nr_slaves == 1).
	So, user must properly configure the parameters to do something more than just straight pipelining.
	
	For more detail, see descriptions of flex_filter, flex_buffer & flex_hub.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"

module flex_superhub #(
			//General parameters
		parameter		addr_bus_width 	= `BB_ADDR_BUS_WIDTH,
		parameter		data_bus_width 	= `BB_DATA_BUS_WIDTH,
			//Filter parameters
		parameter		base_addr		= 0,
		parameter		bits_to_pass 	= addr_bus_width,	//Set to lower value for address filtering
			//Buffer parameters
		parameter		buf_downstream	= 0,				//Buffer master-to-slave signals (0/1)
		parameter		buf_upstream	= 0,				//Buffer slave-to-master signals (0/1)
			//Hub parameters
		parameter		nr_slaves 		= 1					//Set number of slaves
	)(
		input	wire	clock,             							//
		input	wire	reset,             							//   
		
			//Connection to bus master
		input	wire	[addr_bus_width-1 : 0]	addr,				// (Adr_from_SCUB_LA)
		input	wire	[data_bus_width-1 : 0]	data_w,				// (Data_from_SCUB_LA)
		output	wire	[data_bus_width-1 : 0]	data_r,				// (Data_to_SCUB)

		input	wire	addr_strobe,								// (Ext_Adr_Val)
		input	wire	read_trg,										// (Ext_Rd_active)
		input	wire	write_trg,										// (Ext_Wr_active)
		input	wire	read_fin,									// (Ext_Rd_fin)
		input	wire	write_fin,									// (Ext_Wr_fin)
		input	wire	event_trg,								//


		output 	wire	dtack,										// (Dtack_to_SCUB)
		output 	wire	data_r_act,									// (Reg_rd_active)

			//Connection to slave(s)
		output	wire	[addr_bus_width-1 : 0]	sec_addr,			// (Adr_from_SCUB_LA)
		output	wire	[data_bus_width-1 : 0]	sec_data_w,			// (Data_from_SCUB_LA)
		input	wire	[nr_slaves*data_bus_width-1 : 0] sec_data_r,// (Data_to_SCUB)

		output	wire	sec_addr_strobe,							// (Ext_Adr_Val)
		output	wire	sec_read_trg,									// (Ext_Rd_active)
		output	wire	sec_write_trg,									// (Ext_Wr_active)
		output	wire	sec_read_fin,								// (Ext_Rd_fin)
		output	wire	sec_write_fin,								// (Ext_Wr_fin)
		output	wire	sec_event_trg,								//

		input 	wire	[nr_slaves-1 : 0]		sec_dtack,			// (Dtack_to_SCUB)
		input 	wire	[nr_slaves-1 : 0]		sec_data_r_act		// (Reg_rd_active)
	);
	
	//Intermediate signals
wire [15:0]		int_addr;
wire			int_addr_strobe;
wire [15:0]		int_data_r;
wire			int_dtack;
wire			int_data_r_act;

/*
	Connection diagram
	
	           /--> u_flex_filter <-int->               <----------------------\ 
	MASTER <--*                           u_flex_buffer                         *---> SLAVE(s)
	           \------------------------>               <-int-> u_flex_hub1 <--/ 
*/

flex_filter #(
	.base_addr		(base_addr),     
	.bits_to_pass	(bits_to_pass) ,         
	.addr_bus_width	(addr_bus_width)	
	
) u_flex_filter (
	.clock			(clock),
	.reset			(reset),
	
	.addr			(addr),
	.addr_strobe	(addr_strobe),
	
	.sec_addr		(int_addr),
	.sec_addr_strobe(int_addr_strobe)
);


flex_buffer #(
	.buf_downstream	(buf_downstream),
	.buf_upstream	(buf_upstream),
	.addr_bus_width	(addr_bus_width),
	.data_bus_width	(data_bus_width)	
) u_flex_buffer (
	.clock			(clock),
	.reset			(reset),
	
	.addr			(int_addr),
	.data_w			(data_w),
	.data_r			(data_r),
	.addr_strobe	(int_addr_strobe),
	.read_trg		(read_trg),
	.write_trg		(write_trg),
	.read_fin		(read_fin),
	.write_fin		(write_fin),
	.event_trg		(event_trg),
	.dtack			(dtack),
	.data_r_act		(data_r_act),

	.sec_addr		(sec_addr),
	.sec_data_w		(sec_data_w),
	.sec_data_r		(int_data_r),
	.sec_addr_strobe(sec_addr_strobe),
	.sec_read_trg	(sec_read_trg),
	.sec_write_trg	(sec_write_trg),
	.sec_read_fin	(sec_read_fin),
	.sec_write_fin	(sec_write_fin),
	.sec_event_trg	(sec_event_trg),
	.sec_dtack		(int_dtack),
	.sec_data_r_act	(int_data_r_act)
);

flex_hub #(
	.nr_slaves		(nr_slaves),
	.data_bus_width	(data_bus_width)
) u_flex_hub (
	.clock			(clock),
	.reset			(reset),
	
	.data_r			(int_data_r),
	.dtack			(int_dtack),
	.data_r_act		(int_data_r_act),
	
	.sec_data_r		(sec_data_r),
	.sec_dtack		(sec_dtack),
	.sec_data_r_act	(sec_data_r_act)
);


endmodule	

	