/*
	Flexible input register bank.
	Allows free defining of:
		- Number of registers
		- Data bus (register) width: typ. 16 bits
		- Address bus width: typ. 16 bits.
	
	Base address MUST be aligned at a power of 2. 
	$clog2(<nr_registers>) LSBs of address are used for internal register selection 
	and remaining MSBs are compared against base address to select the device.
	
	Port is compatible with standard out_reg vhdl macro (however, unused *_fin signals are missing).
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "flex_helpers.vh"

module flex_in #(
		parameter		addr_bus_width 	= `BB_ADDR_BUS_WIDTH,
		parameter		data_bus_width 	= `BB_DATA_BUS_WIDTH,
		parameter		base_addr		= 0,
		parameter		nr_registers 	= 1,
		parameter		nr_bits			= nr_registers*data_bus_width
	)(
		input	wire	clock,             								//
		input	wire	reset,             								//   
		
		`MAKE_PRI_BUS(addr_bus_width, data_bus_width),
		
		input	wire	[nr_bits-1 : 0]	bits							//
	);
	
genvar		gi;
integer		i;

	// Number of bits used for register selection
localparam	reg_sel_bits = $clog2(nr_registers);

	// Input registers
wire [data_bus_width-1 : 0] in_registers[nr_registers-1 : 0];
	
	//Other registers
reg	reg_dtack;
reg reg_data_r_act;	
reg [data_bus_width-1 : 0] reg_data_r;

	// Assign input registers to linear input space
generate
	for (gi = 0; gi < nr_registers; gi = gi+1) begin : gen_registers
		assign in_registers[gi] = bits[(gi+1) * data_bus_width - 1 -: data_bus_width];	
	end
endgenerate

	// Other assignments
assign dtack			= reg_dtack;
assign data_r_act		= reg_data_r_act;	
assign data_r			= reg_data_r;

	// Slave selection
wire selected;
assign selected = (addr[addr_bus_width-1 : reg_sel_bits] == base_addr[addr_bus_width-1 : reg_sel_bits]) ? addr_strobe : 0;	

	//Register selection
wire [addr_bus_width-1 : 0]	reg_sel;
generate
	if (reg_sel_bits > 0)
		assign reg_sel = addr[reg_sel_bits-1 : 0];
	else
		assign reg_sel = 0;
endgenerate	

	// *** State machine ***

localparam STATE_IDLE	= 0;
localparam STATE_WAIT	= 1;

reg[0:0] state;

always @(posedge clock, posedge reset)
begin
	if (reset)
	begin
		reg_data_r <= 0;
		reg_dtack <= 0;
		reg_data_r_act <= 0;
		state = STATE_IDLE;
	end
	else
	begin
		case (state)
		
			STATE_IDLE:
			begin
				if (selected)
				begin
					if (read_trg)
					begin
						reg_data_r 		<= in_registers[reg_sel];
						reg_dtack 		<= 1;
						reg_data_r_act 	<= 1;
						state 			<= STATE_WAIT;
					end
					else if (write_trg)
					begin
						// No action, but the cycle is supported
						reg_dtack 		<= 1;
						state 			<= STATE_WAIT;				
					end
				end
			end

			STATE_WAIT:
			begin
				if ((!read_trg && !write_trg) || (!selected))
				begin
					reg_dtack 			<= 0;
					reg_data_r_act 		<= 0;
					state 				<= STATE_IDLE;
				end
			end
			
			default:
			begin
				state 					<= STATE_IDLE;
			end
		
		endcase
	end
end

endmodule