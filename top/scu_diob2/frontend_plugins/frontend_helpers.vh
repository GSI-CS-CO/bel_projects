/*
	These macros are for defining default parameters and default ports for all frontend plugins.
	Each frontend plugin must implement these for correct connection to the Blackbox.
*/

`define STD_FRONTEND_PARAMS \
		parameter		nr_diob_ios 					= `BB_NR_DIOB_IOS, \
		parameter		nr_virt_ios 					= `BB_NR_VIRT_IOS, \
		parameter		nr_status_bits					= `BB_FRONTEND_STATUS_BITS
		
`define STD_FRONTEND_PORTS \
		input	wire	clock, \
		input	wire	reset, \
		input	wire	plugin_enable, \
		output	wire	plugin_error, \
		output	wire	[nr_status_bits-1:0]	plugin_status, \
		input	wire	[nr_diob_ios-1:0]		diob_in, \
		output	wire	[nr_diob_ios-1:0]		diob_out, \
		output	wire	[nr_diob_ios-1:0]		diob_dir, \
		input	wire	[nr_virt_ios-1:0]   	internal_out, \
		output	wire	[nr_virt_ios-1:0]   	internal_in, \
		input	wire	[nr_virt_ios-1:0]   	output_enable, \
		input	wire	[nr_virt_ios-1:0]   	input_enable, \
		input	wire	[nr_virt_ios-1:0]   	output_act, \
		input	wire	[nr_virt_ios-1:0]   	input_act
		
/*
		input	wire	clock, \
		input	wire	reset, \
		\
		input	wire	plugin_enable, \            										// Information if the plugin is in use at the time  
		output	wire	plugin_error, \														// Module can inform about an error, oe set for input-ony channel
		output	wire	[nr_status_bits-1:0]	plugin_status, \							// Custom status information
		\
		input	wire	[nr_diob_ios-1:0]		diob_in, \									// Connection to DIOB I/O
		output	wire	[nr_diob_ios-1:0]		diob_out, \									// Connection to DIOB I/O
		output	wire	[nr_diob_ios-1:0]		diob_dir, \									// Connection to DIOB I/O
		\
		input	wire	[nr_virt_ios-1:0]   	internal_out, \								// Connection to proc plugin  
		output	wire	[nr_virt_ios-1:0]   	internal_in, \	 							// Connection to proc plugin  
		\
		input	wire	[nr_virt_ios-1:0]   	output_enable, \							// Information that channel may be used as output  
		input	wire	[nr_virt_ios-1:0]   	input_enable, \								// Information that channel may be used as input  
		input	wire	[nr_virt_ios-1:0]   	output_act, \								// Information that input channel is active at the moment  
		input	wire	[nr_virt_ios-1:0]   	input_act \									// Information that output channel is active at the moment  		
*/