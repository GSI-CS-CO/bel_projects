/*
	A module without connections.
	To be used as dummy if no module is found.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "frontend_helpers.vh"

module frontend_ocin #(
		`STD_FRONTEND_PARAMS
	)(
		`STD_FRONTEND_PORTS
	);


localparam base_pin			= 16;	// pin number for first I/O
wire [2:0] led_user;
wire  select_led;
wire  dtack_led;
wire  interrupt_led;
wire  powerup_led;
wire  clock_source_led;


wire [7:0] pa;
wire [5:0] pb;
wire [7:0] pc;
wire [7:0] pd;

wire f1ca2; // ok-c4
wire f1cb2; // ok-c3
wire s2ca2; // ok-c2
wire s2cb2; // ok-c1
// wire [3:0] ok-e; ok-e and ok-c have the same inputs
wire s_plugin_error;

assign select_led = internal_out[4];
assign dtack_led  = internal_out[3];
assign interrupt_led = internal_out[2];
assign powerup_led = internal_out[1];
assign clock_source_led = internal_out[0];
assign led_user = 3'b0;

assign s2cb2 = internal_out[5];
assign s2ca2 = internal_out[6];
assign f1cb2 = internal_out[7];
assign f1ca2 = internal_out[8];

assign internal_in[16:9] = pa;
assign internal_in[22:17] = pb;
assign internal_in[30:23] = pc;
assign internal_in[38:31] = pd;

// leds

assign diob_out[31 - base_pin] = clock_source_led;
assign diob_dir[31 - base_pin] = 1'b1;

assign diob_out[29 - base_pin] = led_user[2];
assign diob_dir[29 - base_pin] = 1'b1;

assign diob_out[27 - base_pin] = led_user[1];
assign diob_dir[27 - base_pin] = 1'b1;

assign diob_out[25 - base_pin] = led_user[0];
assign diob_dir[25 - base_pin] = 1'b1;

assign diob_out[23 - base_pin] = powerup_led;
assign diob_dir[23 - base_pin] =1'b1;

assign diob_out[21 - base_pin] = interrupt_led;
assign diob_dir[21 - base_pin] = 1'b1;

assign diob_out[19 - base_pin] = dtack_led;
assign diob_dir[19 - base_pin] = 1'b1;

assign diob_out[17 - base_pin] = select_led;
assign diob_dir[17 - base_pin] = 1'b1;

// outputs

assign diob_out[43 - base_pin] = f1ca2;
assign diob_dir[43 - base_pin] = 1'b1;

assign diob_out[45 - base_pin] = f1cb2;
assign diob_dir[45 - base_pin] = 1'b1;

assign diob_out[47 - base_pin] = s2ca2;
assign diob_dir[47 - base_pin] = 1'b1;

assign diob_out[49 - base_pin] = s2cb2;
assign diob_dir[49 - base_pin] = 1'b1;

// inputs

assign pb[5] = diob_in[73 - base_pin];
assign diob_dir[73 - base_pin] = 1'b0;

assign pb[4] = diob_in[75 - base_pin];
assign diob_dir[75 - base_pin] = 1'b0;

assign pd[5] = diob_in[77 - base_pin];
assign diob_dir[77 - base_pin] = 1'b0;

assign pd[6] = diob_in[79 - base_pin];
assign diob_dir[79 - base_pin] = 1'b0;

assign pd[7] = diob_in[81 - base_pin];
assign diob_dir[81 - base_pin] = 1'b0;

assign pd[4] = diob_in[83 - base_pin];
assign diob_dir[83 - base_pin] = 1'b0;

assign pd[3] = diob_in[85 - base_pin];
assign diob_dir[85 - base_pin] = 1'b0;

assign pd[2] = diob_in[87 - base_pin];
assign diob_dir[87 - base_pin] = 1'b0;

assign pd[1] = diob_in[89 - base_pin];
assign diob_dir[89 - base_pin] = 1'b0;

assign pd[0] = diob_in[91 - base_pin];
assign diob_dir[91 - base_pin] = 1'b0;

assign pb[3] = diob_in[93 - base_pin];
assign diob_dir[93 - base_pin] = 1'b0;

assign pb[2] = diob_in[95 - base_pin];
assign diob_dir[95 - base_pin] = 1'b0;

assign pb[1] = diob_in[97 - base_pin];
assign diob_dir[97 - base_pin] = 1'b0;

assign pb[0] = diob_in[99 - base_pin];
assign diob_dir[99 - base_pin] = 1'b0;

assign pc[7] = diob_in[109 - base_pin];
assign diob_dir[109 - base_pin] = 1'b0;

assign pc[6] = diob_in[111 - base_pin];
assign diob_dir[111 - base_pin] = 1'b0;

assign pc[5] = diob_in[113 - base_pin];
assign diob_dir[113 - base_pin] = 1'b0;

assign pc[4] = diob_in[115 - base_pin];
assign diob_dir[115 - base_pin] = 1'b0;

assign pa[7] = diob_in[117 - base_pin];
assign diob_dir[117 - base_pin] = 1'b0;

assign pa[6] = diob_in[119 - base_pin];
assign diob_dir[119 - base_pin] = 1'b0;

assign pa[5] = diob_in[121 - base_pin];
assign diob_dir[121 - base_pin] = 1'b0;

assign pa[4] = diob_in[123 - base_pin];
assign diob_dir[123 - base_pin] = 1'b0;

assign pc[3] = diob_in[125 - base_pin];
assign diob_dir[125 - base_pin] = 1'b0;

assign pc[2] = diob_in[127 - base_pin];
assign diob_dir[127 - base_pin] = 1'b0;

assign pc[1] = diob_in[129 - base_pin];
assign diob_dir[129 - base_pin] = 1'b0;

assign pc[0] = diob_in[131 - base_pin];
assign diob_dir[131 - base_pin] = 1'b0;

assign pa[3] = diob_in[133 - base_pin];
assign diob_dir[133 - base_pin] = 1'b0;

assign pa[2] = diob_in[135 - base_pin];
assign diob_dir[135 - base_pin] = 1'b0;

assign pa[1] = diob_in[137 - base_pin];
assign diob_dir[137 - base_pin] = 1'b0;

assign pa[0] = diob_in[139 - base_pin];
assign diob_dir[139 - base_pin] = 1'b0;

/*	// Generate module error if any of internal_in signals 2-20 has output enabled but input disabled, ...
	// or  any of internal_out 19-0 has input enabled but output disabled */
assign s_plugin_error = 	( |((output_enable[38:9] & ~input_enable[38:9]) & 'h1FFFFFFFF) ) | 
						( |((input_enable[8:0] & ~output_enable[8:0]) & 'h1FF));
assign plugin_error = s_plugin_error;

assign plugin_status = { 13'b0, plugin_enable , 8'b0, pc ,11'b0, pb, 8'b0, pa, 15'b0, s_plugin_error, 10'b0, pb , 7'b0, s2cb2, 7'b0, s2ca2, 7'b0, f1cb2, 7'b0, f1ca2 }; 
/*
PLUGIN_STATUS:
outputs:
0 1ca2; // ok-c4
8 1cb2; // ok-c3
16 2ca2; // ok-c2
24 2cb2; // ok-c1
32     s_plugin_error

inputs:
55-48 pa
68-64  pb
87-80  pc
103-96  pd
112    plugin_enable
*/
assign internal_in [8:0] = 9'b0;
assign internal_in [127:39] = 89'b0;

//

assign diob_out[126:34] = 93'b0;
assign diob_dir[126:124] = 3'b1;
assign diob_dir[122] = 1'b1;
assign diob_dir[120] = 1'b1;
assign diob_dir[118] = 1'b1;
assign diob_dir[116] = 1'b1;
assign diob_dir[114] = 1'b1;
assign diob_dir[112] = 1'b1;
assign diob_dir[110] = 1'b1;
assign diob_dir[108] = 1'b1;
assign diob_dir[106] = 1'b1;
assign diob_dir[104] = 1'b1;
assign diob_dir[102] = 1'b1;
assign diob_dir[100] = 1'b1;
assign diob_dir[98] = 1'b1;
assign diob_dir[96] = 1'b1;
assign diob_dir[94] = 1'b1;
assign diob_dir[92:84] = 9'b1;
assign diob_dir[82] = 1'b1;
assign diob_dir[80] = 1'b1;
assign diob_dir[78] = 1'b1;
assign diob_dir[76] = 1'b1;
assign diob_dir[74] = 1'b1;
assign diob_dir[72] = 1'b1;
assign diob_dir[70] = 1'b1;
assign diob_dir[68] = 1'b1;
assign diob_dir[66] = 1'b1;
assign diob_dir[64] = 1'b1;
assign diob_dir[62] = 1'b1;
assign diob_dir[60] = 1'b1;
assign diob_dir[58] = 1'b1;
assign diob_dir[56:54] = 3'b1;

assign diob_out[32] = 1'b0;
assign diob_dir[32] = 1'b1;

assign diob_out[30] = 1'b0;
assign diob_dir[30] = 1'b1;

assign diob_out[28] = 1'b0;
assign diob_dir[28] = 1'b1;

assign diob_out[26:16] = 11'b0;
assign diob_dir[26:16] = 1'b1;

assign diob_out[14] = 1'b0;
assign diob_dir[14] = 1'b1;

assign diob_out[12] = 1'b0;
assign diob_dir[12] = 1'b1;

assign diob_out[10] = 1'b0;
assign diob_dir[10] = 1'b1;

assign diob_out[8] = 1'b0;
assign diob_dir[8] = 1'b1;

assign diob_out[6] = 1'b0;
assign diob_dir[6] = 1'b1;

assign diob_out[4] = 1'b0;
assign diob_dir[4] = 1'b1;

assign diob_out[2] = 1'b0;
assign diob_dir[2] = 1'b1;

assign diob_out[0] = 1'b0;
assign diob_dir[0] = 1'b1;



endmodule