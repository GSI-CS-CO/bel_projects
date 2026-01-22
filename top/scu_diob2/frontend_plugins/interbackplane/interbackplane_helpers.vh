/*
	These macros are for defining default parameters and default ports for all cardlet plugins.
	Each cardlet plugin must implement these for correct connection to the Interbackplane plugin.
*/

`define STD_CARDLET_PARAMS \
		parameter dummy		= 0
		
`define STD_CARDLET_PORTS \
		input	wire	clock, \
		input	wire	reset, \
		input	wire	plugin_enable, \
		output	wire	plugin_error, \
		input	wire	[5:0]		diob_in, \
		output	wire	[5:0]		diob_out, \
		output	wire	[5:0]		diob_dir, \
		output	wire	[7:0]		diob_led1, \
		output	wire	[7:0]		diob_led2, \
		input	wire	[7:0]   	internal_out, \
		output	wire	[7:0]   	internal_in, \
		input	wire	[7:0]   	output_enable, \
		input	wire	[7:0]   	input_enable, \
		input	wire	[7:0]   	output_act, \
		input	wire	[7:0]   	input_act
		
/*
		input	wire	clock,             						//   
		input	wire	reset,             						//   
		
		input	wire	plugin_enable,             				// Information if the plugin is in use at the time  
		output	wire	plugin_error,							// Module can inform about an error, oe set for input-ony channel

		input	wire	[5:0]		diob_in,   					// Connection to DIOB I/O
		output	wire	[5:0]		diob_out,   				// Connection to DIOB I/O
		output	wire	[5:0]		diob_dir,   				// Connection to DIOB I/O
		
		output	wire	[7:0]		diob_led1,					// Select LED						
		output	wire	[7:0]		diob_led2,					// Activity LED				
		
		input	wire	[7:0]   	internal_out,    			// Connection to proc plugin  
		output	wire	[7:0]   	internal_in,    			// Connection to proc plugin  

		input	wire	[7:0]   	output_enable,    			// Information that channel may be used as output  
		input	wire	[7:0]   	input_enable,    			// Information that channel may be used as input  
		input	wire	[7:0]   	output_act,    				// Information that input channel is active at the moment  
		input	wire	[7:0]   	input_act    				// Information that output channel is active at the moment  		
*/