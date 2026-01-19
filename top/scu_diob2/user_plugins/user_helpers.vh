/*
	These macros are for defining default parameters and default ports for all user plugins.
	Each user plugin must implement these for correct connection to the Blackbox.
*/

`define STD_USER_PARAMS \
		parameter		nr_virt_ios 			= `BB_NR_VIRT_IOS, \
		parameter		nr_backplane_ios 		= `BB_NR_BACKPLANE_IOS,	\
		parameter		nr_irq_lines			= `BB_NR_IRQ_LINES, \
		parameter		max_proc_plugins 		= `BB_MAX_PROC_PLUGINS, \
		parameter		proc_sel_bits 			= $clog2(max_proc_plugins), \
		parameter		addr_bus_width 			= `BB_ADDR_BUS_WIDTH, \
		parameter		data_bus_width			= `BB_DATA_BUS_WIDTH, \
		parameter		frontend_status_bits	= `BB_FRONTEND_STATUS_BITS, \
		parameter		max_frontend_plugins 	= `BB_MAX_FRONTEND_PLUGINS, \
		parameter		frontend_sel_bits 		= $clog2(max_frontend_plugins)
		
`define STD_USER_PORTS \
		input	wire	clock, \
		input	wire	reset, \
		input	wire	plugin_enable, \
		input	wire	frontend_error, \
		input	wire	[frontend_sel_bits-1:0]	frontend_sel, \
		input	wire	frontend_default, \
		input	wire 	[frontend_status_bits-1:0]	frontend_status, \
		input	wire	[nr_virt_ios-1:0]		virtual_in, \
		output	wire	[nr_virt_ios-1:0]		virtual_out, \
		output	wire	[nr_virt_ios*proc_sel_bits-1:0]	proc_sel, \
		input	wire	[nr_virt_ios-1:0]		proc_default, \
		input	wire	[nr_backplane_ios-1:0]	backplane_in, \
		output	wire	[nr_backplane_ios-1:0]	backplane_out, \
		output	wire	[nr_backplane_ios-1:0]	backplane_dir, \
		output	wire	[nr_irq_lines-1:0]		irq, \
		`MAKE_PRI_BUS(addr_bus_width, data_bus_width)	
		
/*
			//General
		input	wire	clock,             													//   
		input	wire	reset,             													//   
		
			//Status information
		input	wire	plugin_enable,             											// Information if the plugin is in use at the time  
		input	wire	frontend_error,														// Frontend error flag
		input	wire	[frontend_sel_bits-1 : 0]	frontend_sel,							// ID of frontend as read out from hardware,	
		input	wire	frontend_default, 													// Default frontend plugin is in use
		input	wire 	[frontend_status_bits-1:0]	frontend_status,						// Additional frontend plugin status information

			//I/O
		input	wire	[nr_virt_ios-1:0]		virtual_in,   								// Input connections
		output	wire	[nr_virt_ios-1:0]		virtual_out,   								// Output connections
		
		output	wire	[nr_virt_ios*proc_sel_bits-1:0]	proc_sel,							// Processing plugin selection - flattened clog2(nr_proc_plugins) * nr_ios bits
		input	wire	[nr_virt_ios-1:0]		proc_default, 								// Default proc plugin is in use

		input	wire	[nr_backplane_ios-1:0]	backplane_in,   							// Backplane input connections
		output	wire	[nr_backplane_ios-1:0]	backplane_out,   							// Backplane output connections
		output	wire	[nr_backplane_ios-1:0]	backplane_dir,   							// Backplane direction control

			// SCU-bus
		`MAKE_PRI_BUS(addr_bus_width, data_bus_width)	
*/