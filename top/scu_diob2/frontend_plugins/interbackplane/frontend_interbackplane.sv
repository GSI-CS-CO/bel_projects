/*
	Interbackplane frontend plugin
*/

`timescale 1 ps / 1 ps
`include "../../blackbox_defines.vh"
`include "../../blackbox_config.vh"
`include "../frontend_helpers.vh"




module frontend_interbackplane #(
		`STD_FRONTEND_PARAMS,
			// Custom parameters
		parameter		nr_cardlets						= 12
	)(
		`STD_FRONTEND_PORTS 
	);

// Check parameters

//generate
//	if (nr_status_bits < 128)
//		$error("Number of status bits insufficient!");	
//endgenerate


localparam base_pin			= 16;	// pin number for first I/O

integer 	i;
genvar		gi;

// ********** Define actual number of plugins **********
	
// *** BEGIN generated code ***
localparam nr_cardlet_plugins 		= 6;
// *** END generated code ***
localparam int_cardlet_sel_bits 	= $clog2(nr_cardlet_plugins);

// *** Cardlet signals ***
wire	[5:0]	cardlet_in	[nr_cardlets-1 : 0];
wire	[5:0]	cardlet_out	[nr_cardlets-1 : 0];
wire	[5:0]	cardlet_dir	[nr_cardlets-1 : 0];
reg		[nr_cardlets-1 : 0]	cardlet_mode;
reg		[1:0]	cardlet_str	[nr_cardlets-1 : 0];
reg		[7:0]	cardlet_type[nr_cardlets-1 : 0];
wire	[7:0]	cardlet_led1[nr_cardlets-1 : 0];
wire	[7:0]	cardlet_led2[nr_cardlets-1 : 0];
wire	[nr_cardlets-1 : 0]	cardlet_error;

// *** Common cardlet signals ***
reg	[1:0]	cardlets_oe;
reg	[7:0]	cardlets_ledb;
wire [7:0]	cardlets_ledb_in;
reg [7:0]	cardlets_ledb_dir;

// *** Multiplexer signals ***
wire	[5:0]	cardlet_out_array	[nr_cardlets-1 : 0][nr_cardlet_plugins-1 : 0];
wire	[5:0]	cardlet_dir_array	[nr_cardlets-1 : 0][nr_cardlet_plugins-1 : 0];
wire	[7:0]	cardlet_led1_array	[nr_cardlets-1 : 0][nr_cardlet_plugins-1 : 0];
wire	[7:0]	cardlet_led2_array	[nr_cardlets-1 : 0][nr_cardlet_plugins-1 : 0];
wire	[7:0]	internal_in_array	[nr_cardlets-1 : 0][nr_cardlet_plugins-1 : 0];
wire	[nr_cardlets-1 : 0]	cardlet_error_array [nr_cardlet_plugins-1 : 0];

	//Cardlet plugin selector
reg [int_cardlet_sel_bits-1: 0]	int_cardlet_plugin_select[nr_cardlets-1 : 0];
reg [nr_cardlets-1 : 0]			cardlet_plugin_default_selected;

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                 SIGNAL ASSIGNMENTS                  *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

//  *** Generate cardlet assignments based on json file ***
// *** BEGIN generated code ***
//-------------------- Slot 0 ----------------------
assign diob_out[68-base_pin]	= cardlet_mode[0];
assign diob_dir[68-base_pin]	= 1'b1;

assign diob_out[64-base_pin]	= cardlet_str[0][0];
assign diob_dir[64-base_pin]	= 1'b1;

assign diob_out[66-base_pin]	= cardlet_str[0][1];
assign diob_dir[66-base_pin]	= 1'b1;

assign diob_out[58-base_pin]	= cardlet_out[0][0];
assign diob_dir[58-base_pin]	= cardlet_dir[0][0];
assign cardlet_in[0][0]		= diob_in[58-base_pin];

assign diob_out[52-base_pin]	= cardlet_out[0][1];
assign diob_dir[52-base_pin]	= cardlet_dir[0][1];
assign cardlet_in[0][1]		= diob_in[52-base_pin];

assign diob_out[60-base_pin]	= cardlet_out[0][2];
assign diob_dir[60-base_pin]	= cardlet_dir[0][2];
assign cardlet_in[0][2]		= diob_in[60-base_pin];

assign diob_out[54-base_pin]	= cardlet_out[0][3];
assign diob_dir[54-base_pin]	= cardlet_dir[0][3];
assign cardlet_in[0][3]		= diob_in[54-base_pin];

assign diob_out[62-base_pin]	= cardlet_out[0][4];
assign diob_dir[62-base_pin]	= cardlet_dir[0][4];
assign cardlet_in[0][4]		= diob_in[62-base_pin];

assign diob_out[56-base_pin]	= cardlet_out[0][5];
assign diob_dir[56-base_pin]	= cardlet_dir[0][5];
assign cardlet_in[0][5]		= diob_in[56-base_pin];

//-------------------- Slot 1 ----------------------
assign diob_out[90-base_pin]	= cardlet_mode[1];
assign diob_dir[90-base_pin]	= 1'b1;

assign diob_out[104-base_pin]	= cardlet_str[1][0];
assign diob_dir[104-base_pin]	= 1'b1;

assign diob_out[106-base_pin]	= cardlet_str[1][1];
assign diob_dir[106-base_pin]	= 1'b1;

assign diob_out[98-base_pin]	= cardlet_out[1][0];
assign diob_dir[98-base_pin]	= cardlet_dir[1][0];
assign cardlet_in[1][0]		= diob_in[98-base_pin];

assign diob_out[92-base_pin]	= cardlet_out[1][1];
assign diob_dir[92-base_pin]	= cardlet_dir[1][1];
assign cardlet_in[1][1]		= diob_in[92-base_pin];

assign diob_out[100-base_pin]	= cardlet_out[1][2];
assign diob_dir[100-base_pin]	= cardlet_dir[1][2];
assign cardlet_in[1][2]		= diob_in[100-base_pin];

assign diob_out[94-base_pin]	= cardlet_out[1][3];
assign diob_dir[94-base_pin]	= cardlet_dir[1][3];
assign cardlet_in[1][3]		= diob_in[94-base_pin];

assign diob_out[102-base_pin]	= cardlet_out[1][4];
assign diob_dir[102-base_pin]	= cardlet_dir[1][4];
assign cardlet_in[1][4]		= diob_in[102-base_pin];

assign diob_out[96-base_pin]	= cardlet_out[1][5];
assign diob_dir[96-base_pin]	= cardlet_dir[1][5];
assign cardlet_in[1][5]		= diob_in[96-base_pin];

//-------------------- Slot 2 ----------------------
assign diob_out[85-base_pin]	= cardlet_mode[2];
assign diob_dir[85-base_pin]	= 1'b1;

assign diob_out[81-base_pin]	= cardlet_str[2][0];
assign diob_dir[81-base_pin]	= 1'b1;

assign diob_out[83-base_pin]	= cardlet_str[2][1];
assign diob_dir[83-base_pin]	= 1'b1;

assign diob_out[75-base_pin]	= cardlet_out[2][0];
assign diob_dir[75-base_pin]	= cardlet_dir[2][0];
assign cardlet_in[2][0]		= diob_in[75-base_pin];

assign diob_out[69-base_pin]	= cardlet_out[2][1];
assign diob_dir[69-base_pin]	= cardlet_dir[2][1];
assign cardlet_in[2][1]		= diob_in[69-base_pin];

assign diob_out[77-base_pin]	= cardlet_out[2][2];
assign diob_dir[77-base_pin]	= cardlet_dir[2][2];
assign cardlet_in[2][2]		= diob_in[77-base_pin];

assign diob_out[71-base_pin]	= cardlet_out[2][3];
assign diob_dir[71-base_pin]	= cardlet_dir[2][3];
assign cardlet_in[2][3]		= diob_in[71-base_pin];

assign diob_out[79-base_pin]	= cardlet_out[2][4];
assign diob_dir[79-base_pin]	= cardlet_dir[2][4];
assign cardlet_in[2][4]		= diob_in[79-base_pin];

assign diob_out[73-base_pin]	= cardlet_out[2][5];
assign diob_dir[73-base_pin]	= cardlet_dir[2][5];
assign cardlet_in[2][5]		= diob_in[73-base_pin];

//-------------------- Slot 3 ----------------------
assign diob_out[95-base_pin]	= cardlet_mode[3];
assign diob_dir[95-base_pin]	= 1'b1;

assign diob_out[97-base_pin]	= cardlet_str[3][0];
assign diob_dir[97-base_pin]	= 1'b1;

assign diob_out[99-base_pin]	= cardlet_str[3][1];
assign diob_dir[99-base_pin]	= 1'b1;

assign diob_out[89-base_pin]	= cardlet_out[3][0];
assign diob_dir[89-base_pin]	= cardlet_dir[3][0];
assign cardlet_in[3][0]		= diob_in[89-base_pin];

assign diob_out[105-base_pin]	= cardlet_out[3][1];
assign diob_dir[105-base_pin]	= cardlet_dir[3][1];
assign cardlet_in[3][1]		= diob_in[105-base_pin];

assign diob_out[91-base_pin]	= cardlet_out[3][2];
assign diob_dir[91-base_pin]	= cardlet_dir[3][2];
assign cardlet_in[3][2]		= diob_in[91-base_pin];

assign diob_out[103-base_pin]	= cardlet_out[3][3];
assign diob_dir[103-base_pin]	= cardlet_dir[3][3];
assign cardlet_in[3][3]		= diob_in[103-base_pin];

assign diob_out[93-base_pin]	= cardlet_out[3][4];
assign diob_dir[93-base_pin]	= cardlet_dir[3][4];
assign cardlet_in[3][4]		= diob_in[93-base_pin];

assign diob_out[101-base_pin]	= cardlet_out[3][5];
assign diob_dir[101-base_pin]	= cardlet_dir[3][5];
assign cardlet_in[3][5]		= diob_in[101-base_pin];

//-------------------- Slot 4 ----------------------
assign diob_out[65-base_pin]	= cardlet_mode[4];
assign diob_dir[65-base_pin]	= 1'b1;

assign diob_out[67-base_pin]	= cardlet_str[4][0];
assign diob_dir[67-base_pin]	= 1'b1;

assign diob_out[51-base_pin]	= cardlet_str[4][1];
assign diob_dir[51-base_pin]	= 1'b1;

assign diob_out[59-base_pin]	= cardlet_out[4][0];
assign diob_dir[59-base_pin]	= cardlet_dir[4][0];
assign cardlet_in[4][0]		= diob_in[59-base_pin];

assign diob_out[57-base_pin]	= cardlet_out[4][1];
assign diob_dir[57-base_pin]	= cardlet_dir[4][1];
assign cardlet_in[4][1]		= diob_in[57-base_pin];

assign diob_out[61-base_pin]	= cardlet_out[4][2];
assign diob_dir[61-base_pin]	= cardlet_dir[4][2];
assign cardlet_in[4][2]		= diob_in[61-base_pin];

assign diob_out[55-base_pin]	= cardlet_out[4][3];
assign diob_dir[55-base_pin]	= cardlet_dir[4][3];
assign cardlet_in[4][3]		= diob_in[55-base_pin];

assign diob_out[63-base_pin]	= cardlet_out[4][4];
assign diob_dir[63-base_pin]	= cardlet_dir[4][4];
assign cardlet_in[4][4]		= diob_in[63-base_pin];

assign diob_out[53-base_pin]	= cardlet_out[4][5];
assign diob_dir[53-base_pin]	= cardlet_dir[4][5];
assign cardlet_in[4][5]		= diob_in[53-base_pin];

//-------------------- Slot 5 ----------------------
assign diob_out[113-base_pin]	= cardlet_mode[5];
assign diob_dir[113-base_pin]	= 1'b1;

assign diob_out[115-base_pin]	= cardlet_str[5][0];
assign diob_dir[115-base_pin]	= 1'b1;

assign diob_out[117-base_pin]	= cardlet_str[5][1];
assign diob_dir[117-base_pin]	= 1'b1;

assign diob_out[107-base_pin]	= cardlet_out[5][0];
assign diob_dir[107-base_pin]	= cardlet_dir[5][0];
assign cardlet_in[5][0]		= diob_in[107-base_pin];

assign diob_out[123-base_pin]	= cardlet_out[5][1];
assign diob_dir[123-base_pin]	= cardlet_dir[5][1];
assign cardlet_in[5][1]		= diob_in[123-base_pin];

assign diob_out[109-base_pin]	= cardlet_out[5][2];
assign diob_dir[109-base_pin]	= cardlet_dir[5][2];
assign cardlet_in[5][2]		= diob_in[109-base_pin];

assign diob_out[121-base_pin]	= cardlet_out[5][3];
assign diob_dir[121-base_pin]	= cardlet_dir[5][3];
assign cardlet_in[5][3]		= diob_in[121-base_pin];

assign diob_out[111-base_pin]	= cardlet_out[5][4];
assign diob_dir[111-base_pin]	= cardlet_dir[5][4];
assign cardlet_in[5][4]		= diob_in[111-base_pin];

assign diob_out[119-base_pin]	= cardlet_out[5][5];
assign diob_dir[119-base_pin]	= cardlet_dir[5][5];
assign cardlet_in[5][5]		= diob_in[119-base_pin];

//-------------------- Slot 6 ----------------------
assign diob_out[47-base_pin]	= cardlet_mode[6];
assign diob_dir[47-base_pin]	= 1'b1;

assign diob_out[49-base_pin]	= cardlet_str[6][0];
assign diob_dir[49-base_pin]	= 1'b1;

assign diob_out[33-base_pin]	= cardlet_str[6][1];
assign diob_dir[33-base_pin]	= 1'b1;

assign diob_out[41-base_pin]	= cardlet_out[6][0];
assign diob_dir[41-base_pin]	= cardlet_dir[6][0];
assign cardlet_in[6][0]		= diob_in[41-base_pin];

assign diob_out[39-base_pin]	= cardlet_out[6][1];
assign diob_dir[39-base_pin]	= cardlet_dir[6][1];
assign cardlet_in[6][1]		= diob_in[39-base_pin];

assign diob_out[43-base_pin]	= cardlet_out[6][2];
assign diob_dir[43-base_pin]	= cardlet_dir[6][2];
assign cardlet_in[6][2]		= diob_in[43-base_pin];

assign diob_out[37-base_pin]	= cardlet_out[6][3];
assign diob_dir[37-base_pin]	= cardlet_dir[6][3];
assign cardlet_in[6][3]		= diob_in[37-base_pin];

assign diob_out[45-base_pin]	= cardlet_out[6][4];
assign diob_dir[45-base_pin]	= cardlet_dir[6][4];
assign cardlet_in[6][4]		= diob_in[45-base_pin];

assign diob_out[35-base_pin]	= cardlet_out[6][5];
assign diob_dir[35-base_pin]	= cardlet_dir[6][5];
assign cardlet_in[6][5]		= diob_in[35-base_pin];

//-------------------- Slot 7 ----------------------
assign diob_out[131-base_pin]	= cardlet_mode[7];
assign diob_dir[131-base_pin]	= 1'b1;

assign diob_out[133-base_pin]	= cardlet_str[7][0];
assign diob_dir[133-base_pin]	= 1'b1;

assign diob_out[135-base_pin]	= cardlet_str[7][1];
assign diob_dir[135-base_pin]	= 1'b1;

assign diob_out[125-base_pin]	= cardlet_out[7][0];
assign diob_dir[125-base_pin]	= cardlet_dir[7][0];
assign cardlet_in[7][0]		= diob_in[125-base_pin];

assign diob_out[141-base_pin]	= cardlet_out[7][1];
assign diob_dir[141-base_pin]	= cardlet_dir[7][1];
assign cardlet_in[7][1]		= diob_in[141-base_pin];

assign diob_out[127-base_pin]	= cardlet_out[7][2];
assign diob_dir[127-base_pin]	= cardlet_dir[7][2];
assign cardlet_in[7][2]		= diob_in[127-base_pin];

assign diob_out[139-base_pin]	= cardlet_out[7][3];
assign diob_dir[139-base_pin]	= cardlet_dir[7][3];
assign cardlet_in[7][3]		= diob_in[139-base_pin];

assign diob_out[129-base_pin]	= cardlet_out[7][4];
assign diob_dir[129-base_pin]	= cardlet_dir[7][4];
assign cardlet_in[7][4]		= diob_in[129-base_pin];

assign diob_out[137-base_pin]	= cardlet_out[7][5];
assign diob_dir[137-base_pin]	= cardlet_dir[7][5];
assign cardlet_in[7][5]		= diob_in[137-base_pin];

//-------------------- Slot 8 ----------------------
assign diob_out[18-base_pin]	= cardlet_mode[8];
assign diob_dir[18-base_pin]	= 1'b1;

assign diob_out[16-base_pin]	= cardlet_str[8][0];
assign diob_dir[16-base_pin]	= 1'b1;

assign diob_out[32-base_pin]	= cardlet_str[8][1];
assign diob_dir[32-base_pin]	= 1'b1;

assign diob_out[24-base_pin]	= cardlet_out[8][0];
assign diob_dir[24-base_pin]	= cardlet_dir[8][0];
assign cardlet_in[8][0]		= diob_in[24-base_pin];

assign diob_out[26-base_pin]	= cardlet_out[8][1];
assign diob_dir[26-base_pin]	= cardlet_dir[8][1];
assign cardlet_in[8][1]		= diob_in[26-base_pin];

assign diob_out[22-base_pin]	= cardlet_out[8][2];
assign diob_dir[22-base_pin]	= cardlet_dir[8][2];
assign cardlet_in[8][2]		= diob_in[22-base_pin];

assign diob_out[28-base_pin]	= cardlet_out[8][3];
assign diob_dir[28-base_pin]	= cardlet_dir[8][3];
assign cardlet_in[8][3]		= diob_in[28-base_pin];

assign diob_out[20-base_pin]	= cardlet_out[8][4];
assign diob_dir[20-base_pin]	= cardlet_dir[8][4];
assign cardlet_in[8][4]		= diob_in[20-base_pin];

assign diob_out[30-base_pin]	= cardlet_out[8][5];
assign diob_dir[30-base_pin]	= cardlet_dir[8][5];
assign cardlet_in[8][5]		= diob_in[30-base_pin];

//-------------------- Slot 9 ----------------------
assign diob_out[136-base_pin]	= cardlet_mode[9];
assign diob_dir[136-base_pin]	= 1'b1;

assign diob_out[134-base_pin]	= cardlet_str[9][0];
assign diob_dir[134-base_pin]	= 1'b1;

assign diob_out[132-base_pin]	= cardlet_str[9][1];
assign diob_dir[132-base_pin]	= 1'b1;

assign diob_out[142-base_pin]	= cardlet_out[9][0];
assign diob_dir[142-base_pin]	= cardlet_dir[9][0];
assign cardlet_in[9][0]		= diob_in[142-base_pin];

assign diob_out[126-base_pin]	= cardlet_out[9][1];
assign diob_dir[126-base_pin]	= cardlet_dir[9][1];
assign cardlet_in[9][1]		= diob_in[126-base_pin];

assign diob_out[140-base_pin]	= cardlet_out[9][2];
assign diob_dir[140-base_pin]	= cardlet_dir[9][2];
assign cardlet_in[9][2]		= diob_in[140-base_pin];

assign diob_out[128-base_pin]	= cardlet_out[9][3];
assign diob_dir[128-base_pin]	= cardlet_dir[9][3];
assign cardlet_in[9][3]		= diob_in[128-base_pin];

assign diob_out[138-base_pin]	= cardlet_out[9][4];
assign diob_dir[138-base_pin]	= cardlet_dir[9][4];
assign cardlet_in[9][4]		= diob_in[138-base_pin];

assign diob_out[130-base_pin]	= cardlet_out[9][5];
assign diob_dir[130-base_pin]	= cardlet_dir[9][5];
assign cardlet_in[9][5]		= diob_in[130-base_pin];

//-------------------- Slot 10 ----------------------
assign diob_out[36-base_pin]	= cardlet_mode[10];
assign diob_dir[36-base_pin]	= 1'b1;

assign diob_out[34-base_pin]	= cardlet_str[10][0];
assign diob_dir[34-base_pin]	= 1'b1;

assign diob_out[50-base_pin]	= cardlet_str[10][1];
assign diob_dir[50-base_pin]	= 1'b1;

assign diob_out[42-base_pin]	= cardlet_out[10][0];
assign diob_dir[42-base_pin]	= cardlet_dir[10][0];
assign cardlet_in[10][0]		= diob_in[42-base_pin];

assign diob_out[44-base_pin]	= cardlet_out[10][1];
assign diob_dir[44-base_pin]	= cardlet_dir[10][1];
assign cardlet_in[10][1]		= diob_in[44-base_pin];

assign diob_out[40-base_pin]	= cardlet_out[10][2];
assign diob_dir[40-base_pin]	= cardlet_dir[10][2];
assign cardlet_in[10][2]		= diob_in[40-base_pin];

assign diob_out[46-base_pin]	= cardlet_out[10][3];
assign diob_dir[46-base_pin]	= cardlet_dir[10][3];
assign cardlet_in[10][3]		= diob_in[46-base_pin];

assign diob_out[38-base_pin]	= cardlet_out[10][4];
assign diob_dir[38-base_pin]	= cardlet_dir[10][4];
assign cardlet_in[10][4]		= diob_in[38-base_pin];

assign diob_out[48-base_pin]	= cardlet_out[10][5];
assign diob_dir[48-base_pin]	= cardlet_dir[10][5];
assign cardlet_in[10][5]		= diob_in[48-base_pin];

//-------------------- Slot 11 ----------------------
assign diob_out[118-base_pin]	= cardlet_mode[11];
assign diob_dir[118-base_pin]	= 1'b1;

assign diob_out[116-base_pin]	= cardlet_str[11][0];
assign diob_dir[116-base_pin]	= 1'b1;

assign diob_out[114-base_pin]	= cardlet_str[11][1];
assign diob_dir[114-base_pin]	= 1'b1;

assign diob_out[124-base_pin]	= cardlet_out[11][0];
assign diob_dir[124-base_pin]	= cardlet_dir[11][0];
assign cardlet_in[11][0]		= diob_in[124-base_pin];

assign diob_out[108-base_pin]	= cardlet_out[11][1];
assign diob_dir[108-base_pin]	= cardlet_dir[11][1];
assign cardlet_in[11][1]		= diob_in[108-base_pin];

assign diob_out[122-base_pin]	= cardlet_out[11][2];
assign diob_dir[122-base_pin]	= cardlet_dir[11][2];
assign cardlet_in[11][2]		= diob_in[122-base_pin];

assign diob_out[110-base_pin]	= cardlet_out[11][3];
assign diob_dir[110-base_pin]	= cardlet_dir[11][3];
assign cardlet_in[11][3]		= diob_in[110-base_pin];

assign diob_out[120-base_pin]	= cardlet_out[11][4];
assign diob_dir[120-base_pin]	= cardlet_dir[11][4];
assign cardlet_in[11][4]		= diob_in[120-base_pin];

assign diob_out[112-base_pin]	= cardlet_out[11][5];
assign diob_dir[112-base_pin]	= cardlet_dir[11][5];
assign cardlet_in[11][5]		= diob_in[112-base_pin];

//----------------------------------------------------
// *** END generated code ***

//  *** Generate common signal assignments based on json file ***
// *** BEGIN generated code ***
assign diob_out[86-base_pin]	= cardlets_oe[0];
assign diob_dir[86-base_pin]	= 1'b1;
assign diob_out[88-base_pin]	= cardlets_oe[1];
assign diob_dir[88-base_pin]	= 1'b1;

assign diob_out[84-base_pin]	= cardlets_ledb[0];
assign diob_dir[84-base_pin]	= cardlets_ledb_dir[0];
assign cardlets_ledb_in[0]		= diob_in[84-base_pin];
assign diob_out[82-base_pin]	= cardlets_ledb[1];
assign diob_dir[82-base_pin]	= cardlets_ledb_dir[1];
assign cardlets_ledb_in[1]		= diob_in[82-base_pin];
assign diob_out[80-base_pin]	= cardlets_ledb[2];
assign diob_dir[80-base_pin]	= cardlets_ledb_dir[2];
assign cardlets_ledb_in[2]		= diob_in[80-base_pin];
assign diob_out[78-base_pin]	= cardlets_ledb[3];
assign diob_dir[78-base_pin]	= cardlets_ledb_dir[3];
assign cardlets_ledb_in[3]		= diob_in[78-base_pin];
assign diob_out[76-base_pin]	= cardlets_ledb[4];
assign diob_dir[76-base_pin]	= cardlets_ledb_dir[4];
assign cardlets_ledb_in[4]		= diob_in[76-base_pin];
assign diob_out[74-base_pin]	= cardlets_ledb[5];
assign diob_dir[74-base_pin]	= cardlets_ledb_dir[5];
assign cardlets_ledb_in[5]		= diob_in[74-base_pin];
assign diob_out[72-base_pin]	= cardlets_ledb[6];
assign diob_dir[72-base_pin]	= cardlets_ledb_dir[6];
assign cardlets_ledb_in[6]		= diob_in[72-base_pin];
assign diob_out[70-base_pin]	= cardlets_ledb[7];
assign diob_dir[70-base_pin]	= cardlets_ledb_dir[7];
assign cardlets_ledb_in[7]		= diob_in[70-base_pin];

// *** END generated code ***

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                   LEDB SEQUENCER                    *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 


reg[2:0]	ledb_state;
reg[$clog2(nr_cardlets)-1 : 0]	ledb_cardlet_nr;	//Current cardlet communicated
reg[6:0]	ledb_prescaler;		//For limiting loop speed to about 1 MHz

localparam LEDB_STATE_START			= 0;
localparam LEDB_STATE_ID_PREPARE	= 1;
localparam LEDB_STATE_ID_READ		= 2;
localparam LEDB_STATE_STR1_1		= 3;
localparam LEDB_STATE_STR1_2		= 4;
localparam LEDB_STATE_STR2_1		= 5;
localparam LEDB_STATE_STR2_2		= 6;
localparam LEDB_STATE_LOOP			= 7;



always @(posedge clock, posedge reset)
begin
	if (reset)
	begin
		for (i = 0; i < nr_cardlets; i = i+1)
		begin
			cardlet_type[i] <= 'hFF;
			cardlet_str[i]	<= 'b11;		// For LED initialization
			cardlet_mode[i]	<= 'b1;
		end	
		cardlets_oe			<= 'b00;		// Assert output enables
		cardlets_ledb		<= 0;			// For switching off all LEDs (TODO: think about keeping them on for half a second or so)	
		cardlets_ledb_dir	<= 'hFF;
		ledb_prescaler		<= 'h7F;
		ledb_cardlet_nr		<= nr_cardlets-1;
		ledb_state			<= LEDB_STATE_START;
	end
	else
	begin
		ledb_prescaler <= ledb_prescaler - 1;
		if (ledb_prescaler == 0)
			case (ledb_state)
				LEDB_STATE_START:		// *** Finish LED reset ***
				begin
					for (i = 0; i < nr_cardlets; i = i+1)
						cardlet_str[i]	<= 'b00;		// Finish LED initialization
					ledb_state			<= LEDB_STATE_LOOP;	
				end
				
				LEDB_STATE_ID_PREPARE: 	// *** Setup signals for ID readout ***
				begin
					cardlets_ledb_dir	<= 'h00;
					cardlet_mode[ledb_cardlet_nr]	<= 1'b0;		
					ledb_state			<= LEDB_STATE_ID_READ;	
				end
				
				LEDB_STATE_ID_READ:		// *** Reado out cardlet ID ***
				begin
					cardlet_type[ledb_cardlet_nr]	<= cardlets_ledb_in;
					cardlet_mode[ledb_cardlet_nr]	<= 1'b1;	
					ledb_state			<= LEDB_STATE_STR1_1;
				end
				
				LEDB_STATE_STR1_1:		// *** Set LEDs with strobe 1 - phase 1
				begin
					cardlets_ledb_dir	<= 'hFF;
					cardlets_ledb		<= ~cardlet_led1[ledb_cardlet_nr];
					cardlet_str[ledb_cardlet_nr]	<= 'b01;
					ledb_state			<= LEDB_STATE_STR1_2;
				end
				
				LEDB_STATE_STR1_2:		// *** Set LEDs with strobe 1 - phase 2
				begin
					cardlet_str[ledb_cardlet_nr]	<= 'b00;
					ledb_state			<= LEDB_STATE_STR2_1;
				end

				LEDB_STATE_STR2_1:		// *** Set LEDs with strobe 2 - phase 1
				begin
					cardlets_ledb_dir	<= 'hFF;
					cardlets_ledb		<= ~cardlet_led2[ledb_cardlet_nr];
					cardlet_str[ledb_cardlet_nr]	<= 'b10;
					ledb_state			<= LEDB_STATE_STR2_2;
				end
				
				LEDB_STATE_STR2_2:		// *** Set LEDs with strobe 2 - phase 2
				begin
					cardlet_str[ledb_cardlet_nr]	<= 'b00;
					ledb_state			<= LEDB_STATE_LOOP;
				end
				
				LEDB_STATE_LOOP:		// *** Bus precharge + close the loop
				begin
					if (ledb_cardlet_nr == (nr_cardlets-1))
						ledb_cardlet_nr	<= 0;
					else
						ledb_cardlet_nr	<= ledb_cardlet_nr + 1;
					cardlets_ledb		<= 'hFF;	// Bus precharge
					ledb_state			<= LEDB_STATE_ID_PREPARE;
				end
				
				default:
				begin
					ledb_state			<= LEDB_STATE_START;
				end
						
			endcase
	end
end

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*              PLUGIN SELECT REMAPPING                *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

// ********** Remap (external) plugin select signals to internal (sqeezed) selects which (probably) need less bits ********** 

generate
	for (gi = 0; gi < nr_cardlets; gi = gi + 1) 
	begin : gen_plugin_select	
		always @(*)
		begin
			case (cardlet_type[gi])
				// *** BEGIN generated code ***
					255:		// default
				begin	
					int_cardlet_plugin_select[gi] <= 0;	// empty
					cardlet_plugin_default_selected[gi] <= 0;
				end	
					1,		// lwlio
					2:		// lemoio
				begin	
					int_cardlet_plugin_select[gi] <= 1;	// typ_5in1out
					cardlet_plugin_default_selected[gi] <= 0;
				end	
					7,		// digin
					9:		// optoin
				begin	
					int_cardlet_plugin_select[gi] <= 2;	// typ_in
					cardlet_plugin_default_selected[gi] <= 0;
				end	
					3:		// lemoin
				begin	
					int_cardlet_plugin_select[gi] <= 3;	// inverted_in
					cardlet_plugin_default_selected[gi] <= 0;
				end	
					4:		// lwlin
				begin	
					int_cardlet_plugin_select[gi] <= 4;	// lwl_in
					cardlet_plugin_default_selected[gi] <= 0;
				end	
					5,		// lwlout
					6,		// lemoout
					8:		// ssrout
				begin	
					int_cardlet_plugin_select[gi] <= 5;	// typ_out
					cardlet_plugin_default_selected[gi] <= 0;
				end	
				default: 
				begin
					int_cardlet_plugin_select[gi] <= 0;	// default: empty
					cardlet_plugin_default_selected[gi] <= 1;
				end	
				// *** END generated code ***
			endcase
		end	//always
	end	//for
endgenerate

//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                SIGNAL DEMULTIPLEXING                *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

generate
	for (gi = 0; gi < nr_cardlets; gi = gi + 1) 
	begin : gen_signal_mux	
		assign cardlet_out[gi]  = cardlet_out_array		[gi][int_cardlet_plugin_select[gi]];
		assign cardlet_dir[gi]  = cardlet_dir_array		[gi][int_cardlet_plugin_select[gi]];
		assign cardlet_led1[gi] = cardlet_led1_array	[gi][int_cardlet_plugin_select[gi]];
		assign cardlet_led2[gi] = cardlet_led2_array	[gi][int_cardlet_plugin_select[gi]];
		assign `slice(internal_in, 8, gi) = internal_in_array[gi][int_cardlet_plugin_select[gi]];
		assign cardlet_error[gi] = cardlet_error_array	[int_cardlet_plugin_select[gi]][gi];
	end	//for
endgenerate


//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                      PLUGINS                        *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

generate
	for (gi = 0; gi < nr_cardlets; gi = gi + 1) 
		begin : gen_cardlets
		// *** BEGIN generated code ***
		ibpl_empty cardlet_inst_empty (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 0 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[0][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][0]),
			.diob_dir		(cardlet_dir_array[gi][0]),
			.diob_led1		(cardlet_led1_array[gi][0]),
			.diob_led2		(cardlet_led2_array[gi][0]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][0]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		ibpl_5in1out cardlet_inst_typ_5in1out (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 1 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[1][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][1]),
			.diob_dir		(cardlet_dir_array[gi][1]),
			.diob_led1		(cardlet_led1_array[gi][1]),
			.diob_led2		(cardlet_led2_array[gi][1]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][1]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		ibpl_in cardlet_inst_typ_in (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 2 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[2][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][2]),
			.diob_dir		(cardlet_dir_array[gi][2]),
			.diob_led1		(cardlet_led1_array[gi][2]),
			.diob_led2		(cardlet_led2_array[gi][2]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][2]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		ibpl_in #(
			.invert_signals	(1)
		) cardlet_inst_inverted_in (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 3 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[3][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][3]),
			.diob_dir		(cardlet_dir_array[gi][3]),
			.diob_led1		(cardlet_led1_array[gi][3]),
			.diob_led2		(cardlet_led2_array[gi][3]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][3]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		ibpl_lwlin cardlet_inst_lwl_in (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 4 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[4][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][4]),
			.diob_dir		(cardlet_dir_array[gi][4]),
			.diob_led1		(cardlet_led1_array[gi][4]),
			.diob_led2		(cardlet_led2_array[gi][4]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][4]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		ibpl_out cardlet_inst_typ_out (
			.clock       	(clock),
			.reset       	(reset),

			.plugin_enable	(int_cardlet_plugin_select[gi] == 5 ? 1'b1 : 1'b0),
			.plugin_error	(cardlet_error_array[5][gi]),
		
			.diob_in		(cardlet_in[gi]),
			.diob_out		(cardlet_out_array[gi][5]),
			.diob_dir		(cardlet_dir_array[gi][5]),
			.diob_led1		(cardlet_led1_array[gi][5]),
			.diob_led2		(cardlet_led2_array[gi][5]),
		
			.internal_out	(`slice(internal_out, 8, gi)),
			.internal_in	(internal_in_array[gi][5]),
			.output_enable	(`slice(output_enable, 8, gi)),
			.input_enable	(`slice(input_enable, 8, gi)),
			.output_act		(`slice(output_act, 8, gi)),
			.input_act		(`slice(input_act, 8, gi))

		);			

		// *** END generated code ***	
	end
endgenerate	


//             /-------------------------------------------------------\ 
//             |*******************************************************| 
//             |*                  STATUS OUTPUTS                     *| 
//             |*******************************************************| 
//             \-------------------------------------------------------/ 

	//Assert plugin error if any cardlet asserted error or default was selected (lacking ID)
assign plugin_error 					= (|cardlet_error) | (|cardlet_plugin_default_selected);

generate
	for (gi = 0; gi < nr_cardlets; gi = gi + 1) 
	begin : gen_plugin_status		
		assign `slice(plugin_status, 8, gi) = cardlet_type[gi];		//12 LSBs - cardlet types (96 bits = 6 registers)
	end	//for
endgenerate

assign `slice(plugin_status, 16, 6) = cardlet_plugin_default_selected;	// 7th register: default selected flags
assign `slice(plugin_status, 16, 7) = cardlet_error;					// 8th register: cardlet errors flag
 
endmodule