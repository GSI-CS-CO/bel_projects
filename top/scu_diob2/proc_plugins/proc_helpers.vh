/*
	These macros are for defining default parameters and default ports for all proc plugins.
	Each proc plugin must implement these for correct connection to the Blackbox.
*/

`define STD_PROC_PARAMS \
		parameter dummy		= 0
		
`define STD_PROC_PORTS \
		input	wire	clock, \
		input	wire	reset, \
		input	wire	plugin_enable, \
		output	wire	internal_out, \
		input	wire	internal_in, \
		input	wire	virtual_out, \
		output	wire	virtual_in, \
		output	wire	output_enable, \
		output	wire	input_enable
		
/*
		input	wire	clock,				//   
		input	wire	reset,				//   

		input	wire	plugin_enable,		// Information if the plugin is in use at the time  

		output	wire	internal_out,		// Connection to frontend plugin part  
		input	wire	internal_in,		// Connection to frontend plugin part  
		
		input	wire	virtual_out,		// Connection to user  
		output	wire	virtual_in, 		// Connection to user  

		output	wire	output_enable,    	// Information that channel may be used as output  
		output	wire	input_enable    	// Information that channel may be used as input  		
*/