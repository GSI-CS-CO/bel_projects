/*
	A blackbox prototype
*/

// ATTENTION: *.v.template file is the one to edit. *.v is automatically generated!

`timescale 1 ps / 1 ps
`include "blackbox_defines.vh"
`include "blackbox_config.vh"
`include "flex_bus/flex_helpers.vh"

module io_blackbox #(
		parameter		nr_diob_ios 			= `BB_NR_DIOB_IOS,
		parameter		nr_virt_ios 			= `BB_NR_VIRT_IOS,
		parameter		nr_backplane_ios 		= `BB_NR_BACKPLANE_IOS,
		parameter		nr_irq_lines			= `BB_NR_IRQ_LINES,
		parameter		max_frontend_plugins 	= `BB_MAX_FRONTEND_PLUGINS,
		parameter		max_proc_plugins 		= `BB_MAX_PROC_PLUGINS,
		parameter		max_user_plugins 		= `BB_MAX_USER_PLUGINS,
		parameter		frontend_status_bits	= `BB_FRONTEND_STATUS_BITS,
		parameter		frontend_sel_bits 		= $clog2(max_frontend_plugins),
		parameter		proc_sel_bits 			= $clog2(max_proc_plugins),
		parameter		user_sel_bits 			= $clog2(max_user_plugins),
		
		parameter		addr_bus_width 			= `BB_ADDR_BUS_WIDTH,
		parameter		data_bus_width			= `BB_DATA_BUS_WIDTH
	)(
			// Common
		input	wire	clock,             								//   
		input	wire	reset,             								//   
			// Frontend
		inout	wire	[nr_diob_ios-1:0]		diob_io,   				// Connection to DIOB I/O
		input	wire	[frontend_sel_bits-1:0]	frontend_plugin_select,	// I/O plugin selection
		inout	wire	[nr_backplane_ios-1:0]	backplane_io,			// Backplane input/output fed (almost) directly	to user plugin
			// SCU-bus
		`MAKE_PRI_BUS(addr_bus_width, data_bus_width),
			// Interrupts
		output	wire	[nr_irq_lines-1:0]		irq,	
			// Debugging purposes
		output	wire	[15:0]					test_out
	);

genvar i;

// ********** Define actual number of plugins **********
	
// *** BEGIN generated code ***
localparam nr_frontend_plugins 		= 4;
localparam nr_proc_plugins 			= 5;
localparam nr_user_plugins 			= 1;
// *** END generated code ***

localparam int_frontend_sel_bits 	= $clog2(nr_frontend_plugins);
localparam int_proc_sel_bits 		= $clog2(nr_proc_plugins);
localparam int_user_sel_bits 		= $clog2(nr_proc_plugins);

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                    SIGNAL PATH                      *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

// ********* Datapath, status & selector wires *********

	// DIOB signals
wire [nr_diob_ios-1:0]	diob_out_array[nr_frontend_plugins-1:0];
wire [nr_diob_ios-1:0]	diob_dir_array[nr_frontend_plugins-1:0];
wire [nr_diob_ios-1:0]	diob_in;
reg  [nr_diob_ios-1:0]	diob_in_buf1;
reg  [nr_diob_ios-1:0]	diob_in_buf2;
wire [nr_diob_ios-1:0]	diob_out;
wire [nr_diob_ios-1:0]	diob_dir;

	// Frontend plugin status
wire [frontend_status_bits-1:0]	frontend_status_array[nr_frontend_plugins-1:0];
wire [nr_frontend_plugins-1:0]	frontend_error_array;
wire [frontend_status_bits-1:0]	frontend_status;
wire							frontend_error;		

	// Internal signals
wire [nr_virt_ios-1:0] internal_in_array[nr_frontend_plugins-1:0];
wire [nr_virt_ios-1:0] internal_out_array[nr_proc_plugins-1:0];
wire [nr_virt_ios-1:0] internal_oe_array[nr_proc_plugins-1:0];
wire [nr_virt_ios-1:0] internal_ie_array[nr_proc_plugins-1:0];
wire [nr_virt_ios-1:0] internal_in;
wire [nr_virt_ios-1:0] internal_out;
wire [nr_virt_ios-1:0] internal_oe;
wire [nr_virt_ios-1:0] internal_ie;

	// Virtual signals
wire [nr_virt_ios-1:0] virtual_in_array[nr_proc_plugins-1:0];	
wire [nr_virt_ios-1:0] virtual_out_array[nr_user_plugins-1:0];	
wire [nr_virt_ios-1:0] virtual_out;   								
wire [nr_virt_ios-1:0] virtual_in;  		

	// Backplane signals
wire [nr_backplane_ios-1:0] backplane_out_array[nr_user_plugins-1:0];	
wire [nr_backplane_ios-1:0] backplane_dir_array[nr_user_plugins-1:0];	
wire [nr_backplane_ios-1:0] backplane_out;   								
wire [nr_backplane_ios-1:0] backplane_dir;   								
wire [nr_backplane_ios-1:0] backplane_in;  	

	// Interrupt signals
wire [nr_irq_lines-1:0]		irq_array[nr_user_plugins-1:0];

	//Plugin selectors
wire [nr_virt_ios*proc_sel_bits-1:0]	proc_plugin_select_array[nr_user_plugins-1:0];				
wire [nr_virt_ios*proc_sel_bits-1:0]	proc_plugin_select;					// Processing plugin selection - flattened clog2(nr_proc_plugins) * nr_ios bits
wire [user_sel_bits-1:0]				user_plugin_select;							// User plugin selection 
reg [int_frontend_sel_bits-1: 0] 		int_frontend_plugin_select;
reg [int_proc_sel_bits-1: 0] 			int_proc_plugin_select[nr_virt_ios-1:0];
reg [int_user_sel_bits-1: 0] 			int_user_plugin_select;
reg										frontend_plugin_default_selected;
reg	[nr_virt_ios-1:0]					proc_plugin_default_selected;
reg										user_plugin_default_selected;

// -----------------------------------------------------------------------------

// ********** Remap (external) plugin select signals to internal (sqeezed) selects which (probably) need less bits ********** 

	// Frontend plugins
always @(*)
begin
	case (frontend_plugin_select)
		// *** BEGIN generated code ***
			255:		// default
		begin	
			int_frontend_plugin_select <= 0;	// unknown
			frontend_plugin_default_selected <= 0;
		end	
			4,		// ocio1
			14:		// ocio2
		begin	
			int_frontend_plugin_select <= 1;	// ocio
			frontend_plugin_default_selected <= 0;
		end	
			3:		// ocin1
		begin	
			int_frontend_plugin_select <= 2;	// ocin
			frontend_plugin_default_selected <= 0;
		end	
			19:		// inlb12s1
		begin	
			int_frontend_plugin_select <= 3;	// interbackplane
			frontend_plugin_default_selected <= 0;
		end	
		default: 
		begin
			int_frontend_plugin_select <= 0;	// default: unknown
			frontend_plugin_default_selected <= 1;
		end	
		// *** END generated code ***
	endcase
end

	// Proc plugins	
generate
	for (i = 0; i < nr_virt_ios; i = i + 1) begin : gen_proc_select	
		wire [proc_sel_bits-1 : 0]	selector;
		assign selector = proc_plugin_select[(i+1)*proc_sel_bits - 1 -: proc_sel_bits];
		always @(*)
		begin
			case (selector)
				// *** BEGIN generated code ***
				0:	
				begin
					int_proc_plugin_select[i] <= 0;	// disable
					proc_plugin_default_selected[i] <= 0;
				end	
				1:	
				begin
					int_proc_plugin_select[i] <= 1;	// pass
					proc_plugin_default_selected[i] <= 0;
				end	
				2:	
				begin
					int_proc_plugin_select[i] <= 2;	// in_debounce2
					proc_plugin_default_selected[i] <= 0;
				end	
				3:	
				begin
					int_proc_plugin_select[i] <= 3;	// in_debounce4
					proc_plugin_default_selected[i] <= 0;
				end	
				4:	
				begin
					int_proc_plugin_select[i] <= 4;	// in_debounce125
					proc_plugin_default_selected[i] <= 0;
				end	
				default: 
				begin
					int_proc_plugin_select[i] <= 0;	// default: disable	
					proc_plugin_default_selected[i] <= 1;
				end	
				// *** END generated code ***
			endcase
		end
	end
endgenerate

	// User plugins
always @(*)
begin
	case (user_plugin_select)
		// *** BEGIN generated code ***
		0: 
		begin	
			int_user_plugin_select <= 0;	// gpio
			user_plugin_default_selected <= 0;
		end	
		default: 
		begin
			int_user_plugin_select <= 0;	// default: gpio
			user_plugin_default_selected <= 1;
		end	
		// *** END generated code ***
	endcase
end

// -----------------------------------------------------------------------------
	
// ********** DIOB signals - demultiplexing & bi-dir driver control & synchronization********** 
		
assign diob_out = diob_out_array[int_frontend_plugin_select];
assign diob_dir = diob_dir_array[int_frontend_plugin_select];
assign diob_in = diob_io;

generate
	for (i = 0; i < nr_diob_ios; i = i + 1) begin : gen_diob	
		assign diob_io[i] = diob_dir[i] ? diob_out[i] : 'bz;
	end
endgenerate

always @ (posedge clock)
begin
	diob_in_buf1 <= diob_in;
	diob_in_buf2 <= diob_in_buf1;
end

// ********** Frontend status signals - demultiplexing ********** 

assign frontend_status = frontend_status_array[int_frontend_plugin_select];
assign frontend_error = frontend_error_array[int_frontend_plugin_select];

// ********** Internal signals - demultiplexing ********** 

assign internal_in 	= internal_in_array[int_frontend_plugin_select];

generate
	for (i = 0; i < nr_virt_ios; i = i + 1) begin : gen_internal	
		assign internal_out[i]	= internal_out_array[int_proc_plugin_select[i]][i];
		assign internal_oe[i]	= internal_oe_array[int_proc_plugin_select[i]][i];
		assign internal_ie[i] 	= internal_ie_array[int_proc_plugin_select[i]][i];
	end
endgenerate

// ********** Virtual signals & proc plugin selection - demultiplexing ********** 

assign virtual_out = virtual_out_array[int_user_plugin_select];
assign proc_plugin_select = proc_plugin_select_array[int_user_plugin_select];

generate
	for (i = 0; i < nr_virt_ios; i = i + 1) begin : gen_virtual	
		assign virtual_in[i] 	= virtual_in_array[int_proc_plugin_select[i]][i];
	end
endgenerate

// ********** Backplane signals - demultiplexing & bi-dir driver control & synchronization********** 
		
assign backplane_out = backplane_out_array[int_user_plugin_select];
assign backplane_dir = backplane_dir_array[int_user_plugin_select];
assign backplane_in = backplane_io;

generate
	for (i = 0; i < nr_backplane_ios; i = i + 1) begin : gen_backplane
		assign backplane_io[i] = backplane_dir[i] ? backplane_out[i] : 'bz;
	end
endgenerate

// ********** Interrupt signals - demultiplexing ********** 

assign irq = irq_array[int_user_plugin_select];

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                      SCU BUS                        *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

localparam nr_internal_bus_slaves 	= 3;
localparam nr_all_bus_slaves 		= nr_internal_bus_slaves + nr_user_plugins;		// Number of devices on secondary bus

wire								INTaddr_strobe;	
wire								USRaddr_strobe;	

`MAKE_SEC_BUS(SEC, nr_all_bus_slaves, addr_bus_width, data_bus_width)

// ********** Address filter + main hub ********** 

flex_filterplus #(
	.addr_bus_width	(addr_bus_width),
	.lower_bound (`BB_BASE_blackbox)
) flex_filterplus_main (
	.clock			(clock),
	.reset			(reset),
	
	.addr			(addr),
	.addr_strobe	(addr_strobe),
	.sec_addr_strobe(INTaddr_strobe)
);

flex_superhub #(
	.base_addr		(0),     
	.bits_to_pass	(addr_bus_width),	//pass everything          
	.buf_downstream	(1),
	.buf_upstream	(1),
	.nr_slaves		(nr_all_bus_slaves),
	.addr_bus_width	(addr_bus_width),
	.data_bus_width	(data_bus_width)
) flex_superhub_main (
	.clock			(clock),
	.reset			(reset),
	
	.addr			(addr),
	.data_w			(data_w),
	.data_r			(data_r),
	.addr_strobe	(INTaddr_strobe),
	.read_trg		(read_trg),
	.write_trg		(write_trg),
	.read_fin		(read_fin),
	.write_fin		(write_fin),
	.event_trg		(event_trg),
	.dtack			(dtack),
	.data_r_act		(data_r_act),

	.sec_addr		(SECaddr),
	.sec_data_w		(SECdata_w),
	.sec_data_r		(SECdata_r),
	.sec_addr_strobe(SECaddr_strobe),
	.sec_read_trg	(SECread_trg),
	.sec_write_trg	(SECwrite_trg),
	.sec_read_fin	(SECread_fin),
	.sec_write_fin	(SECwrite_fin),
	.sec_event_trg	(SECevent_trg),
	.sec_dtack		(SECdtack),
	.sec_data_r_act	(SECdata_r_act)
);

// ********** Address filter for user plugins ********** 

flex_filterplus #(
	.addr_bus_width	(addr_bus_width),
	.lower_bound (`BB_BASE_user_area)
) flex_filterplus_user (
	.clock			(clock),
	.reset			(reset),
	
	.addr			(addr),
	.addr_strobe	(SECaddr_strobe),
	.sec_addr_strobe(USRaddr_strobe)
);

// ********** Info block ********** 
// +00: Version: 8 MSBs are major version, 8 LSBs are minor version
// +01: Blackbox configuration number
// +02:	12'b0, user_plugin_default_selected, |proc_plugin_default_selected, frontend_plugin_default_selected,  frontend_error
// +03: Frontend plugin selection

wire [11:0] bb_version_major	= `BB_VERSION_MAJOR;
wire [3:0] bb_version_minor		= `BB_VERSION_MINOR;
wire [15:0] bb_config			= 0;
wire [15:0] fps16 				= frontend_plugin_select;

wire [63 : 0] status_info;
assign status_info = {	fps16,																					// Frontend plugin selection
						12'b0, user_plugin_default_selected, |proc_plugin_default_selected, frontend_plugin_default_selected,  frontend_error,  // status (12e+4b)
						bb_config,																				// Config (16b)
						bb_version_major, bb_version_minor};													// Blackbox versions

`FLEX_IN(0, 
		flex_in_info, 
		SEC, 
		`BB_BASE_info, 
		`ceildiv(64, data_bus_width), 
		addr_bus_width, 
		data_bus_width, 
		status_info)

// ********** Frontend status **********

`FLEX_IN(1, 
		flex_in_frontend_status, 
		SEC, 
		`BB_BASE_frontend_status, 
		`ceildiv(frontend_status_bits, data_bus_width), 
		addr_bus_width, 
		data_bus_width, 
		frontend_status)

// ********** User plugin selection **********

wire [data_bus_width-1 : 0] ups16;
assign user_plugin_select = ups16;

`FLEX_OUT(2, 
		flex_out_user_sel, 
		SEC, 
		`BB_BASE_user_sel, 
		1, 
		addr_bus_width, 
		data_bus_width, 
		ups16)

// ********** Frontend plugin status **********

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                      PLUGINS                        *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

// ********** Frontend plugins ********** 
// *** BEGIN generated code ***
frontend_unknown #(
	.nr_diob_ios	(nr_diob_ios),
	.nr_virt_ios	(nr_virt_ios),
	.nr_status_bits	(frontend_status_bits)
) frontend_inst_unknown (
	.clock       	(clock),
	.reset       	(reset),
	.plugin_enable	(int_frontend_plugin_select == 0 ? 1'b1 : 1'b0),
	.plugin_error	(frontend_error_array[0]),
	.plugin_status	(frontend_status_array[0]),
	.diob_in     	(diob_in_buf2),
	.diob_out    	(diob_out_array[0]),
	.diob_dir    	(diob_dir_array[0]),
	.internal_out	(internal_out),
	.internal_in 	(internal_in_array[0]),
	.output_enable 	(internal_oe),
	.input_enable  	(internal_ie),
	.output_act    	(internal_oe & virtual_out),
	.input_act    	(internal_ie & virtual_in)
);
	
frontend_ocio #(
	.nr_diob_ios	(nr_diob_ios),
	.nr_virt_ios	(nr_virt_ios),
	.nr_status_bits	(frontend_status_bits)
) frontend_inst_ocio (
	.clock       	(clock),
	.reset       	(reset),
	.plugin_enable	(int_frontend_plugin_select == 1 ? 1'b1 : 1'b0),
	.plugin_error	(frontend_error_array[1]),
	.plugin_status	(frontend_status_array[1]),
	.diob_in     	(diob_in_buf2),
	.diob_out    	(diob_out_array[1]),
	.diob_dir    	(diob_dir_array[1]),
	.internal_out	(internal_out),
	.internal_in 	(internal_in_array[1]),
	.output_enable 	(internal_oe),
	.input_enable  	(internal_ie),
	.output_act    	(internal_oe & virtual_out),
	.input_act    	(internal_ie & virtual_in)
);
	
frontend_ocin #(
	.nr_diob_ios	(nr_diob_ios),
	.nr_virt_ios	(nr_virt_ios),
	.nr_status_bits	(frontend_status_bits)
) frontend_inst_ocin (
	.clock       	(clock),
	.reset       	(reset),
	.plugin_enable	(int_frontend_plugin_select == 2 ? 1'b1 : 1'b0),
	.plugin_error	(frontend_error_array[2]),
	.plugin_status	(frontend_status_array[2]),
	.diob_in     	(diob_in_buf2),
	.diob_out    	(diob_out_array[2]),
	.diob_dir    	(diob_dir_array[2]),
	.internal_out	(internal_out),
	.internal_in 	(internal_in_array[2]),
	.output_enable 	(internal_oe),
	.input_enable  	(internal_ie),
	.output_act    	(internal_oe & virtual_out),
	.input_act    	(internal_ie & virtual_in)
);
	
frontend_interbackplane #(
	.nr_diob_ios	(nr_diob_ios),
	.nr_virt_ios	(nr_virt_ios),
	.nr_status_bits	(frontend_status_bits)
) frontend_inst_interbackplane (
	.clock       	(clock),
	.reset       	(reset),
	.plugin_enable	(int_frontend_plugin_select == 3 ? 1'b1 : 1'b0),
	.plugin_error	(frontend_error_array[3]),
	.plugin_status	(frontend_status_array[3]),
	.diob_in     	(diob_in_buf2),
	.diob_out    	(diob_out_array[3]),
	.diob_dir    	(diob_dir_array[3]),
	.internal_out	(internal_out),
	.internal_in 	(internal_in_array[3]),
	.output_enable 	(internal_oe),
	.input_enable  	(internal_ie),
	.output_act    	(internal_oe & virtual_out),
	.input_act    	(internal_ie & virtual_in)
);
	
// *** END generated code ***	

// ********** Proc plugins ********** 
generate
	for (i = 0; i < nr_virt_ios; i = i + 1) begin : gen_proc
		// *** BEGIN generated code ***
		proc_disable proc_inst_disable (
			.clock       	(clock),
			.reset       	(reset),
			.plugin_enable	(int_proc_plugin_select[i] == 0 ? 1'b1 : 1'b0),
			.internal_out	(internal_out_array[0][i]),
			.internal_in	(internal_in[i]),
			.virtual_out	(virtual_out[i]),
			.virtual_in		(virtual_in_array[0][i]),
			.output_enable	(internal_oe_array[0][i]),
			.input_enable	(internal_ie_array[0][i])
		);			

		proc_pass proc_inst_pass (
			.clock       	(clock),
			.reset       	(reset),
			.plugin_enable	(int_proc_plugin_select[i] == 1 ? 1'b1 : 1'b0),
			.internal_out	(internal_out_array[1][i]),
			.internal_in	(internal_in[i]),
			.virtual_out	(virtual_out[i]),
			.virtual_in		(virtual_in_array[1][i]),
			.output_enable	(internal_oe_array[1][i]),
			.input_enable	(internal_ie_array[1][i])
		);			

		proc_in_debounce #(
			.nr_stages	(2)
		) proc_inst_in_debounce2 (
			.clock       	(clock),
			.reset       	(reset),
			.plugin_enable	(int_proc_plugin_select[i] == 2 ? 1'b1 : 1'b0),
			.internal_out	(internal_out_array[2][i]),
			.internal_in	(internal_in[i]),
			.virtual_out	(virtual_out[i]),
			.virtual_in		(virtual_in_array[2][i]),
			.output_enable	(internal_oe_array[2][i]),
			.input_enable	(internal_ie_array[2][i])
		);			

		proc_in_debounce #(
			.nr_stages	(4)
		) proc_inst_in_debounce4 (
			.clock       	(clock),
			.reset       	(reset),
			.plugin_enable	(int_proc_plugin_select[i] == 3 ? 1'b1 : 1'b0),
			.internal_out	(internal_out_array[3][i]),
			.internal_in	(internal_in[i]),
			.virtual_out	(virtual_out[i]),
			.virtual_in		(virtual_in_array[3][i]),
			.output_enable	(internal_oe_array[3][i]),
			.input_enable	(internal_ie_array[3][i])
		);			

		proc_in_long_debounce #(
			.count_ticks	(125)
		) proc_inst_in_debounce125 (
			.clock       	(clock),
			.reset       	(reset),
			.plugin_enable	(int_proc_plugin_select[i] == 4 ? 1'b1 : 1'b0),
			.internal_out	(internal_out_array[4][i]),
			.internal_in	(internal_in[i]),
			.virtual_out	(virtual_out[i]),
			.virtual_in		(virtual_in_array[4][i]),
			.output_enable	(internal_oe_array[4][i]),
			.input_enable	(internal_ie_array[4][i])
		);			

		// *** END generated code ***	
	end
endgenerate		

// ********** User plugins ********** 
// *** BEGIN generated code ***
user_gpio #(
	.nr_virt_ios			(nr_virt_ios),
	.nr_backplane_ios		(nr_backplane_ios),
	.max_proc_plugins		(max_proc_plugins),
	.addr_bus_width			(addr_bus_width),
	.data_bus_width			(data_bus_width),
	.frontend_status_bits	(frontend_status_bits),
	.max_frontend_plugins	(max_frontend_plugins),
	.frontend_sel_bits		(frontend_sel_bits)
) user_inst_gpio (
		//General
	.clock       		(clock),
	.reset       		(reset),
		//Status
	.plugin_enable		(int_user_plugin_select == 0 ? 1'b1 : 1'b0),
	.frontend_error		(frontend_error),
	.frontend_sel		(frontend_plugin_select),
	.frontend_default	(frontend_plugin_default_selected),
	.frontend_status	(frontend_status),
		//I/O
	.virtual_in 		(virtual_in),
	.virtual_out		(virtual_out_array[0]),
	.proc_sel			(proc_plugin_select_array[0]), 
	.proc_default		(proc_plugin_default_selected),
	.backplane_in 		(backplane_in),
	.backplane_out		(backplane_out_array[0]),
	.backplane_dir		(backplane_dir_array[0]),
	.irq				(irq_array[0]),
		//Bus
	.addr				(SECaddr),
	.data_w				(SECdata_w),
	.data_r				(`slice(SECdata_r, `BB_ADDR_BUS_WIDTH, 0+nr_internal_bus_slaves)),
	.addr_strobe		(int_user_plugin_select == 0 ? USRaddr_strobe : 1'b0),
	.read_trg			(SECread_trg),
	.write_trg			(SECwrite_trg),
	.read_fin			(SECread_fin),
	.write_fin			(SECwrite_fin),
	.event_trg			(int_user_plugin_select == 0 ? SECevent_trg : 1'b0),
	.dtack				(SECdtack[0+nr_internal_bus_slaves]),
	.data_r_act			(SECdata_r_act[0+nr_internal_bus_slaves])
);
	
// *** END generated code ***	


//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                    TEST OUTPUTS                     *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

// SCU bus elements monitoring

assign test_out[0]	= clock;
assign test_out[1]	= reset;

assign test_out[2]	= addr_strobe;
assign test_out[3]	= INTaddr_strobe;
assign test_out[4]	= SECaddr_strobe;

assign test_out[5]	= write_trg;
assign test_out[6]	= SECwrite_trg;
assign test_out[7]	= read_trg;
assign test_out[8]	= SECread_trg;

assign test_out[9]	= dtack;
assign test_out[10]	= SECdtack[0];		// Status block
assign test_out[11]	= SECdtack[1];		// User plugin selector
assign test_out[12]	= data_r_act;
assign test_out[13]	= SECdata_r_act[0];	// Status block
assign test_out[14]	= SECdata_r_act[1];	// User plugin selector

assign test_out[15]	= SECaddr[9];


endmodule