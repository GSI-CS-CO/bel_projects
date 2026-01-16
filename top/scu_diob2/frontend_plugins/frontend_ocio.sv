/*
	A module without connections.
	To be used as dummy if no module is found.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "frontend_helpers.vh"

module frontend_ocio #(
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

wire  [7:0] pb;
wire  ca;
wire  cb;
wire  cc;
wire  cd;

wire [7:0] pc;
wire [7:0] pa;
wire [7:0] pd;

wire oe;
wire s_plugin_error;
// LEDS A_nLED0 - A_nLED7 assigned to  internal_out 0 - 4 (led_users not used)

assign select_led = internal_out[4];
assign dtack_led  = internal_out[3];
assign interrupt_led = internal_out[2];
assign powerup_led = internal_out[1];
assign clock_source_led = internal_out[0];
assign led_user = 3'b0;

//OUTPUTS PB 7-0, CA2, CB2, CC2, CD2  (OutReg1B0-B11)

assign pb = internal_out[12:5];
assign ca = internal_out[16];
assign cb = internal_out[17];
assign cc = internal_out[18];
assign cd = internal_out[19];



// diob_out 

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

assign diob_out[55 - base_pin] = pb[0];
assign diob_dir[55 - base_pin] = 1'b1;

assign diob_out[57 - base_pin] = pb[1];
assign diob_dir[57 - base_pin] = 1'b1;

assign diob_out[59 - base_pin] = pb[2];
assign diob_dir[59 - base_pin] = 1'b1;

assign diob_out[103 - base_pin] = pb[3];
assign diob_dir[103 - base_pin] = 1'b1;

assign diob_out[101 - base_pin] = pb[4];
assign diob_dir[101 - base_pin] = 1'b1;

assign diob_out[113 - base_pin] = pb[5];
assign diob_dir[113 - base_pin] = 1'b1;

assign diob_out[111 - base_pin] = pb[6];
assign diob_dir[111 - base_pin] = 1'b1;

assign diob_out[109 - base_pin] = pb[7];
assign diob_dir[109 - base_pin] = 1'b1;

assign diob_out[115 - base_pin] = ca;
assign diob_dir[115 - base_pin] = 1'b1;

assign diob_out[107 - base_pin] = cb;
assign diob_dir[107 - base_pin] = 1'b1;

assign diob_out[61 - base_pin] = cc;
assign diob_dir[61 - base_pin] = 1'b1;

assign diob_out[105 - base_pin] = cd;
assign diob_dir[105 - base_pin] = 1'b1;



/* INPUTS PC 7-0, PA 7-0 (InReg1B15-0)
   INPUTS PD 7-0         (InReg2B7-0)
*/

// diob_in

assign pa[0] = diob_in[93 - base_pin];
assign diob_dir[93 - base_pin] = 1'b0;

assign pa[1] = diob_in[95 - base_pin];
assign diob_dir[95 - base_pin] = 1'b0;

assign pa[2] = diob_in[97 - base_pin];
assign diob_dir[97 - base_pin] = 1'b0;

assign pa[3] = diob_in[99 - base_pin];
assign diob_dir[99 - base_pin] = 1'b0;

assign pa[4] = diob_in[121 - base_pin];
assign diob_dir[121 - base_pin] = 1'b0;

assign pa[5] = diob_in[123 - base_pin];
assign diob_dir[123 - base_pin] = 1'b0;

assign pa[6] = diob_in[125 - base_pin];
assign diob_dir[125 - base_pin] = 1'b0;

assign pa[7] = diob_in[127 - base_pin];
assign diob_dir[127 - base_pin] = 1'b0;

assign pc[0] = diob_in[43 - base_pin];
assign diob_dir[43 - base_pin] = 1'b0;

assign pc[1] = diob_in[41 - base_pin];
assign diob_dir[41 - base_pin] = 1'b0;

assign pc[2] = diob_in[39 - base_pin];
assign diob_dir[39 - base_pin] = 1'b0;

assign pc[3] = diob_in[37 - base_pin];
assign diob_dir[37 - base_pin] = 1'b0;

assign pc[4] = diob_in[53 - base_pin];
assign diob_dir[53 - base_pin] = 1'b0;

assign pc[5] = diob_in[51 - base_pin];
assign diob_dir[51 - base_pin] = 1'b0;

assign pc[6] = diob_in[47 - base_pin];
assign diob_dir[47 - base_pin] = 1'b0;

assign pc[7] = diob_in[45 - base_pin];
assign diob_dir[45 - base_pin] = 1'b0;

assign pd[0] = diob_in[81 - base_pin];
assign diob_dir[81 - base_pin] = 1'b0;

assign pd[1] = diob_in[83 - base_pin];
assign diob_dir[83 - base_pin] = 1'b0;

assign pd[2] = diob_in[85 - base_pin];
assign diob_dir[85 - base_pin] = 1'b0;

assign pd[3] = diob_in[87 - base_pin];
assign diob_dir[87 - base_pin] = 1'b0;

assign pd[4] = diob_in[117 - base_pin];
assign diob_dir[117 - base_pin] = 1'b0;

assign pd[5] = diob_in[119 - base_pin];
assign diob_dir[119 - base_pin] = 1'b0;

assign pd[6] = diob_in[91 - base_pin];
assign diob_dir[91 - base_pin] = 1'b0;

assign pd[7] = diob_in[89 - base_pin];
assign diob_dir[89 - base_pin] = 1'b0;

//
assign internal_in[27:20] = pa;
assign internal_in[35:28] = pd;
assign internal_in[43:36] = pc;


// output enable 

assign oe = internal_out[44];
assign internal_in[44] =1'b0;

assign diob_out[77 - base_pin] = oe; 			// for OCIO1
assign diob_dir[77 - base_pin] = 1'b1;

assign diob_out[79 - base_pin] = oe;			// for OCIO2
assign diob_dir[79 - base_pin] = 1'b1;

/*	// Generate module error if any of internal_in signals 2-20 has output enabled but input disabled, ...
	// or  any of internal_out 19-0 has input enabled but output disabled */
assign s_plugin_error = 	( |((output_enable[44:20] & ~input_enable[44:20]) & 'h3FFFFFF) ) | 
						( |((input_enable[19:0] & ~output_enable[19:0]) & 'hFFF));
assign plugin_error = s_plugin_error;

assign plugin_status = { 15'b0, plugin_enable , 8'b0, pd, 8'b0, pc, 8'b0, pa, 15'b0, s_plugin_error, 8'b0, pb , 7'b0, cd, 7'b0, cc, 7'b0, cb, 7'b0, ca }; 
/*
PLUGIN_STATUS:
outputs:
0      ca
8      cb
16     cc
24     cd
39-32  pb
48     s_plugin_error

inputs:
71-64  pa
87-80  pc
104-96  pd
112    plugin_enable
*/

assign internal_in [19:0] = 20'b0;
assign internal_in [127:45] = 83'b0;

assign diob_out[126:100] = 27'b0;
assign diob_dir[126:112] = 15'b1;
assign diob_dir[110] = 1'b1;
assign diob_dir[108] = 1'b1;
assign diob_dir[106] = 1'b1;
assign diob_dir[104] = 1'b1;
assign diob_dir[102] = 1'b1;
assign diob_dir[100] = 1'b1;

assign diob_out[98] = 1'b0;
assign diob_dir[98] = 1'b1;

assign diob_out[96] = 1'b0;
assign diob_dir[96] = 1'b1;

assign diob_out[94] = 1'b0;
assign diob_dir[94] = 1'b1;

assign diob_out[92] = 1'b0;
assign diob_dir[92] = 1'b1;

assign diob_out[90] = 1'b0;
assign diob_dir[90] = 1'b1;

assign diob_out[88] = 1'b0;
assign diob_dir[88] = 1'b1;

assign diob_out[86] = 1'b0;
assign diob_dir[86] = 1'b1;

assign diob_out[84:64] = 21'b0;
assign diob_dir[84]= 1'b1;
assign diob_dir[82]= 1'b1;
assign diob_dir[80]= 1'b1;
assign diob_dir[78]= 1'b1;
assign diob_dir[76]= 1'b1;
assign diob_dir[74]= 1'b1;
assign diob_dir[72]= 1'b1;
assign diob_dir[70]= 1'b1;
assign diob_dir[68]= 1'b1;
assign diob_dir[66]= 1'b1;
assign diob_dir[64]= 1'b1;

assign diob_out[62] = 1'b0;
assign diob_dir[62] = 1'b1;

assign diob_out[60:46] = 15'b0;
assign diob_dir[60:46] = 15'b1;

assign diob_out[44] = 1'b0;
assign diob_dir[44] = 1'b1;

assign diob_out[42] = 1'b0;
assign diob_dir[42] = 1'b1;

assign diob_out[40] = 1'b0;
assign diob_dir[40] = 1'b1;

assign diob_out[38:16] = 23'b0;
assign diob_dir[38] = 1'b1;
assign diob_dir[36] = 1'b1;
assign diob_dir[34:32] = 3'b1;
assign diob_dir[30] = 1'b1;
assign diob_dir[28] = 1'b1;
assign diob_dir[26] = 1'b1;
assign diob_dir[24] = 1'b1;
assign diob_dir[22] = 1'b1;
assign diob_dir[20:16] = 5'b1;

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