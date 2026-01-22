`ifndef FLEX_HELPERS_VH
`define FLEX_HELPERS_VH


/*
	A macro for generating primary bus signals on module interface
*/
`define MAKE_PRI_BUS(_addr_bus_width, _data_bus_width) \
		input	wire	[ _addr_bus_width - 1 : 0 ]	addr, \
		input	wire	[ _data_bus_width - 1 : 0 ]	data_w, \
		output	wire	[ _data_bus_width - 1 : 0 ]	data_r, \
		input	wire	addr_strobe, \
		input	wire	read_trg, \
		input	wire	write_trg, \
		input	wire	read_fin, \
		input	wire	write_fin, \
		input	wire	event_trg, \
		output 	wire	dtack, \
		output 	wire	data_r_act 


/*
	A macro for generating a secondary bus with given prefix.
*/

`define MAKE_SEC_BUS(_prefix, _nr_slaves, _addr_bus_width, _data_bus_width) \
wire	[_addr_bus_width-1 : 0] 			_prefix``addr; \
wire	[_data_bus_width-1 : 0]				_prefix``data_w; \
wire	[(_data_bus_width*_nr_slaves)-1 : 0] _prefix``data_r; \
wire										_prefix``addr_strobe; \
wire										_prefix``read_trg; \
wire										_prefix``write_trg; \
wire										_prefix``read_fin; \
wire										_prefix``write_fin; \
wire										_prefix``event_trg; \
wire	[_nr_slaves-1 : 0] 					_prefix``dtack; \
wire	[_nr_slaves-1 : 0]					_prefix``data_r_act;

/*
	A macro for using a secondary bus in a slave module
	Most common case: without *_fin signals, without event_trg.
	These signals, if needed, must be added manually.
*/

`define USE_SEC_BUS(_port_prefix, _bus_prefix, _index, _data_bus_width) \
	.``_port_prefix``addr			( _bus_prefix``addr ), \
	.``_port_prefix``data_w			( _bus_prefix``data_w ), \
	.``_port_prefix``data_r			( `slice( _bus_prefix``data_r, _data_bus_width, _index ) ), \
	.``_port_prefix``addr_strobe	( _bus_prefix``addr_strobe ), \
	.``_port_prefix``read_trg		( _bus_prefix``read_trg ), \
	.``_port_prefix``write_trg		( _bus_prefix``write_trg ), \
	.``_port_prefix``dtack			( _bus_prefix``dtack[ _index ] ), \
	.``_port_prefix``data_r_act		( _bus_prefix``data_r_act[ _index ] )


/*
	These macros are to facilitate instantation of flex components.
	IMPORTANT: Bus signal names must be equal to the ones used in the interface (plus a prefix).
	The easiest way is to use above bus generation macro.
	Signals: clock and reset must exist and do their job!
*/

`define FLEX_IN(_index, _name, _bus_prefix, _base_addr, _nr_registers, _addr_bus_width, _data_bus_width, _bits) \
flex_in #( \
	.base_addr		( _base_addr ), \
	.nr_registers	( _nr_registers ), \
	.addr_bus_width	( _addr_bus_width ), \
	.data_bus_width	( _data_bus_width ) \
) _name ( \
	.clock			(clock), \
	.reset			(reset), \
	`USE_SEC_BUS(, _bus_prefix, _index, _data_bus_width), \
	.bits			( _bits ) \
);

`define FLEX_OUT(_index, _name, _bus_prefix, _base_addr, _nr_registers, _addr_bus_width, _data_bus_width, _bits) \
flex_out #( \
	.base_addr		( _base_addr ), \
	.nr_registers	( _nr_registers ), \
	.addr_bus_width	( _addr_bus_width ), \
	.data_bus_width	( _data_bus_width ) \
) _name ( \
	.clock			(clock), \
	.reset			(reset), \
	`USE_SEC_BUS(, _bus_prefix, _index, _data_bus_width), \
	.bits			( _bits ) \
);

`define FLEX_SUPERHUB(_name, _pri_bus_prefix, _sec_bus_prefix, _base_addr, _bits_to_pass, _buf_downstream, _buf_upstream, _nr_slaves, _addr_bus_width, _data_bus_width) \
flex_superhub #( \
	.addr_bus_width	( _addr_bus_width ), \
	.data_bus_width	( _data_bus_width ), \
	.base_addr		( _base_addr ), \
	.bits_to_pass	( _bits_to_pass ), \
	.buf_downstream	( _buf_downstream ), \
	.buf_upstream	( _buf_upstream ), \
	.nr_slaves		( _nr_slaves ) \
) _name ( \
	.clock			(clock), \
	.reset			(reset), \
	.addr			( _pri_bus_prefix``addr ), \
	.data_w			( _pri_bus_prefix``data_w ), \
	.data_r			( _pri_bus_prefix``data_r ), \
	.addr_strobe	( _pri_bus_prefix``addr_strobe ), \
	.read_trg		( _pri_bus_prefix``read_trg ), \
	.write_trg		( _pri_bus_prefix``write_trg ), \
	.read_fin		( _pri_bus_prefix``read_fin ), \
	.write_fin		( _pri_bus_prefix``write_fin ), \
	.event_trg		( _pri_bus_prefix``event_trg ), \
	.dtack			( _pri_bus_prefix``dtack ), \
	.data_r_act		( _pri_bus_prefix``data_r_act ), \
	.sec_addr		( _sec_bus_prefix``addr ), \
	.sec_data_w		( _sec_bus_prefix``data_w ), \
	.sec_data_r		( _sec_bus_prefix``data_r ), \
	.sec_addr_strobe( _sec_bus_prefix``addr_strobe ), \
	.sec_read_trg	( _sec_bus_prefix``read_trg ), \
	.sec_write_trg	( _sec_bus_prefix``write_trg ), \
	.sec_read_fin	( _sec_bus_prefix``read_fin ), \
	.sec_write_fin	( _sec_bus_prefix``write_fin ), \
	.sec_event_trg	( _sec_bus_prefix``event_trg ), \
	.sec_dtack		( _sec_bus_prefix``dtack ), \
	.sec_data_r_act	( _sec_bus_prefix``data_r_act ) \
);




`endif