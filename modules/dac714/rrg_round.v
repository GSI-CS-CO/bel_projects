/* This code for realtime ramp generator, in which it is executed to obtain come curvy edges while ramping on and out,
	and in between the normal slope ramping. 
	In addition the values could be changed anytime, as it is not a condition to be in steady state.
	
	The module is controlled over a mutiplexed I/O: reg_0 through reg_3 and outreg_0 through outreg_3 are used
	for data transfer while the control register is used to geave meaning to these data (see CMD_xxx defines below)
	
	For more help, see the doc directory in the rrg-project git repo, especially rrg_programming.md.

	Module parameters:
		RESOLUTION:	The resolution of calculations (up to 64 bits and down to DAC_WIDTH-2).
				The I/O is always 64-bit and left aligned, so changing the resolution doesn't
				affect the data format.
		DAC_WIDTH:	The bit width of the DAC port.				
		NR_DATASETS:	Number of the datasets to be used. Can be any value between 1 and 256.
		READOUT:	Readout capabilities. An  or-combination of the following flags:
				0x01: capable of reading Yis
				0x02: capable of reading Ris
				0x04: capable of reading Yset, Rset, RIset, ROset
				0x08: capable of reading the dataset information
				0x10: capable of reading NUM_CYCLE

	Module ports:
		clk:		System clock, 125 MHz or so
		clk_slow:	Slow clock, 10 MHz. Is actually used for generation
		nReset:		Reset signal, low active
		reg_control:	Control register to issue commands, ee CMD_xxx defines below
		reg_0-reg_3:	Input data registers foming a 64-bit data input
		outreg_0-outreg_3: Output data registers foming a 64-bit data output
		ext_dataset:	External dataset selection input
		dac_strobe:	Strobe signal for the DAC, single clk_slow cycle, high active
		dac_out:	Data for the DAC, updtaed every dac_strobe pulse

*/


 module rrg_round#(parameter 
	RESOLUTION = 40,
	DAC_WIDTH = 16, 
	NR_DATASETS = 1,
	READOUT = 1) 
 (
	input clk,
	input clk_slow,
	input nReset,
	input           	[15:0] reg_control,
	input 			[15:0] reg_0, 
	input 			[15:0] reg_1,  
	input 			[15:0] reg_2,  
	input 			[15:0] reg_3,
	output 			[15:0] outreg_0,
	output 			[15:0] outreg_1,
	output 			[15:0] outreg_2,
	output 			[15:0] outreg_3,
	input 			[7:0]  ext_dataset,				
	output reg 		       dac_strobe,
	output reg signed [DAC_WIDTH-1:0] dac_out
);

	localparam CMD_IDLE			= 0;
	localparam CMD_WRITE_YSET 		= 1;
	localparam CMD_WRITE_RSET 		= 2;
	localparam CMD_WRITE_RISET 		= 3;
	localparam CMD_WRITE_ROSET 		= 4;
	localparam CMD_UPDATE			= 5;
	localparam CMD_SW_DATASET 		= 6;
	localparam CMD_EXT_DATASET 		= 7;
	localparam CMD_NUM_CYCLE		= 8;
	localparam CMD_HALT			= 9;
	
	localparam CMD_READ_YSET		= 11;
	localparam CMD_READ_RSET 		= 12;
	localparam CMD_READ_RISET 		= 13;
	localparam CMD_READ_ROSET 		= 14;
	localparam CMD_READ_DATASET 		= 16;	//returns {47{1b'0}, use_ext_dataset, ext_dataset, current_dataset}
	localparam CMD_READ_NUM_CYCLE		= 18;
	
	localparam CMD_READ_YIS			= 24;
	localparam CMD_READ_RIS			= 25;
	
	localparam CMD_READ_PARAMS		= 31;	//returns {{(8){1'b0}}, RESOLUTION[7:0], DAC_WIDTH[7:0], NR_DATASETS[7:0], READOUT[7:0]	};
	

	localparam CAP_READ_YIS			= 1;
	localparam CAP_READ_RIS			= 2;
	localparam CAP_READ_SETVAL		= 4;
	localparam CAP_READ_DATASET		= 8;
	localparam CAP_READ_NUM_CYCLE		= 16;	

	integer i;
	
	reg 	[7:0] 		current_dataset;
	wire 	[7:0] 		write_dataset;
	wire 	[7:0] 		command;
	reg 			use_ext_dataset;
	
	wire [7:0] dataset_in_use;
	assign dataset_in_use = use_ext_dataset ? ext_dataset : current_dataset;
	
	reg signed [RESOLUTION-1 : 0] Ris;
	reg signed [RESOLUTION-1 : 0] Yis;
	

	
	reg 			time_step_tc;
	reg unsigned [31:0]	time_step;
	
		//Parameter input buffers
	
	reg signed [RESOLUTION-1 : 0] temp_Yset=0;
	reg signed [RESOLUTION-1 : 0] temp_Rset=0;  
	reg signed [RESOLUTION-1 : 0] temp_RIset=0;  
	reg signed [RESOLUTION-1 : 0] temp_ROset=0;

	reg input_ready;
		//Dataset buffers
		//For single dataset, they should be optimized out
	
	reg signed [RESOLUTION-1 : 0] Yset_buf[NR_DATASETS-1:0];
	reg signed [RESOLUTION-1 : 0] Rset_buf[NR_DATASETS-1:0];  
	reg signed [RESOLUTION-1 : 0] RIset_buf[NR_DATASETS-1:0];  
	reg signed [RESOLUTION-1 : 0] ROset_buf[NR_DATASETS-1:0];
	
	
		//Generator buffers
	
	reg signed [RESOLUTION-1 : 0] Yset;
	reg signed [RESOLUTION-1 : 0] Rset;  
	reg signed [RESOLUTION-1 : 0] RIset;  
	reg signed [RESOLUTION-1 : 0] ROset;
	
	reg        [RESOLUTION-1 : 0] num_cycle;
	
	reg signed [2*RESOLUTION-1 : 0] Yt_dash;
	reg signed [RESOLUTION-1 : 0] Ydiff;

		//Unsigned versions of some variables
		//TODO: Maybe cheat it by calculating minus just as bit inversion. We introduce only 1 LSB error!

	wire sgnYdiff;
	wire [RESOLUTION-1 : 0] 	absYdiff;
	wire [RESOLUTION-1 : 0] 	minusYdiff;
	//assign minusYdiff = -Ydiff;
	assign minusYdiff = ~Ydiff;
	assign sgnYdiff = Ydiff[RESOLUTION-1];
	assign absYdiff = sgnYdiff ? minusYdiff : Ydiff;
	
	wire sgnRis;
	wire [RESOLUTION-1 : 0] 	absRis;
	wire [RESOLUTION-1 : 0] 	minusRis;
	//assign minusRis = -Ris;
	assign minusRis = ~Ris;
	assign sgnRis = Ris[RESOLUTION-1];
	assign absRis = sgnRis ? minusRis : Ris;	
	
		//This construct is used for comparisons to decide if we are rounding in/out
		
	wire [RESOLUTION-1 : 0] 	roundComp;
	assign roundComp = (sgnYdiff ? ~Ris : Ris) + ~Rset;
	
		//Common multiplier
		//Used for calculating the round-out condition
		//to avoid additional buffering, here we mux needed values into mul_1 and mul_2
		
	wire  [RESOLUTION-1 : 0] mul_1;
	wire  [RESOLUTION-1 : 0] mul_2;
	reg 			mul_sel = 0;
	wire [2*RESOLUTION-1 : 0] mul_result;
	reg  [2*RESOLUTION-1 : 0] acc;
	assign mul_1 = mul_sel ? ROset : absRis;
	assign mul_2 = mul_sel ? absYdiff : absRis;
	assign mul_result = mul_1*mul_2;

		//The state machine

	reg[3:0] state;
	
	localparam STATE_IDLE			= 0;
	localparam STATE_CALC_YDIFF		= 1;
	localparam STATE_CHECK_YDIFF		= 2;
	localparam STATE_CALC_RO		= 3;
	localparam STATE_CHECK_RO		= 4;
	localparam STATE_STABLE			= 5;
	localparam STATE_ROUND_OUT		= 6;
	localparam STATE_ROUND_IN		= 7;
	localparam STATE_RAMP			= 8;
	localparam STATE_UPDATE_YIS		= 9;
	localparam STATE_CLAMP			= 10;
	localparam STATE_SET_DAC		= 11;
	
	//Process for the calculations syn. with the slow clk (10MHz)
	always @(posedge clk_slow) 
	begin
	
		if(nReset == 0)
		begin
			Yis		<= 0;
			Ris 	  	<= 0;
			
			Yset 		<= 0;
			Rset 		<= 0;
			RIset 		<= 0;
			ROset 		<= 0;
			
			state		<= STATE_IDLE;
			
			dac_out 	<= 0;
			dac_strobe 	<= 0;
		end
		else
		begin
		
			case (state)
				
				STATE_IDLE:
				begin
						//once the time_step_tc is high we begin calculations
						//this is done to slow down the calculation speed so that the DAC can recognize it.
					if(time_step_tc == 1)
					begin
							//Update DAC with previous cycle's value and trigger it
						dac_strobe 	<= 1;
						dac_out 	<= Yis[RESOLUTION-3 -: DAC_WIDTH];	

							//Update set values from dataset
						if (NR_DATASETS > 1)
						begin							
							Yset <= Yset_buf[dataset_in_use];
							Rset <= Rset_buf[dataset_in_use];
							RIset <= RIset_buf[dataset_in_use];
							ROset <= ROset_buf[dataset_in_use];			
						end
						else if (input_ready)
						begin
							Yset <= temp_Yset;
							Rset <= temp_Rset;
							RIset <= temp_RIset;
							ROset <= temp_ROset;			
						end	

							//Go to the next state
						state <= STATE_CALC_YDIFF;
					end			
				end
			
				STATE_CALC_YDIFF:
				begin
						//First, negate DAC trigger
					dac_strobe 	<= 0;
						
						//Calculate Ydiff
					//Ydiff <= Yset - Yis;
					Ydiff <= Yset + ~Yis;
					
						//Independently, check condition for immediate step
					if (ROset == 0 || RIset == 0 || Rset == 0) //Special case: step ramping
						state <= STATE_STABLE;
					else 
						state <=  STATE_CHECK_YDIFF;
				end
				
				STATE_CHECK_YDIFF:
				begin
					if ( absYdiff <= ROset && absRis <= ROset)	//Stable
						state <= STATE_STABLE;
					else
					begin
							//Here we start calculation of round-out condition.
							//According to Michal's maths, it should be:
							//1/2 Ris > Roset * absYdiff
						//mul_1 <= absRis;
						//mul_2 <= absRis;
						mul_sel <= 0;
						state <= STATE_CALC_RO;
					end	
				end
				
				STATE_CALC_RO:
				begin
					acc <= (mul_result >> 1);	// 1/2 Ris^2
					//mul_1 <= ROset;
					//mul_2 <= absYdiff;
					mul_sel <= 1;
					state <= STATE_CHECK_RO;
				end
									
				STATE_CHECK_RO:
				begin
					if (acc > mul_result)
						state <= STATE_ROUND_OUT;
					else
					begin
						//if ((sgnYdiff ? -Ris : Ris) - Rset < -RIset)
						//if ((sgnYdiff ? ~Ris : Ris) + ~Rset < ~RIset)
						if (roundComp < ~RIset)
							state <= STATE_ROUND_IN;					//Rounding In
						//else if ((sgnYdiff ? -Ris : Ris) - Rset > ROset)
						//else if ((sgnYdiff ? ~Ris : Ris) + ~Rset > ROset)
						else if (roundComp > ROset)
							state <= STATE_ROUND_IN;					//Rounding in (2)
						else
							state <= STATE_RAMP;						//Ramping
					end				
				end
				
				STATE_ROUND_OUT:
				begin
					//Ris = Ris - (sgnRis ? -ROset : ROset);	
					Ris = Ris + ~(sgnRis ? ~ROset : ROset);	
					state 		<= STATE_UPDATE_YIS;
				end
				
				STATE_ROUND_IN:
				begin
					//Ris = Ris + (sgnYdiff ? -RIset : RIset);
					Ris = Ris + (sgnYdiff ? ~RIset : RIset);
					state 		<= STATE_UPDATE_YIS;
				end
				
				STATE_RAMP:
				begin
					state 		<= STATE_UPDATE_YIS;
				end
				
				STATE_STABLE:
				begin
					Yis		<= Yset;						
					Ris 		<= 0;
					state 		<= STATE_UPDATE_YIS;
				end
				
				STATE_UPDATE_YIS:
				begin
					Yis 		<= Yis + Ris;
					state 		<= STATE_CLAMP; 
				end
				
				STATE_CLAMP:
				begin
					case (Yis[RESOLUTION-1 -:3])
						3'b001, 3'b010, 3'b011: //upper clamp range
						begin
							Yis <= {3'b000, {(RESOLUTION-3){1'b1}}};
							Ris <= 0;
						end
						3'b100, 3'b101, 3'b110: //lower clamp range
						begin
							Yis <= {3'b111, {(RESOLUTION-3){1'b0}}};
							Ris <= 0;
						end
					endcase
					state 		<= STATE_IDLE;
				end
		
			endcase
		
					
		end	
	end

	// ********* INPUT / OUTPUT *********
	
	wire 	   [RESOLUTION-1 : 0] temp_reg;
	reg signed [63 : 0]           outreg = 0;
	

	//reg unsigned [15:0] temp_num_cycle = 60000;

		//Assign output ports. If resolution is less than 64, ...
		//... fill LSBs with zeros
	//wire [63:0] outreg_ext;
	//assign outreg_ext = { outreg, {(64-RESOLUTION){1'b0}} };
	assign outreg_0 = outreg[15 -: 16];
	assign outreg_1 = outreg[31 -: 16];
	assign outreg_2 = outreg[47 -: 16];
	assign outreg_3 = outreg[63 -: 16];
	
		//Assign input ports
	wire [63:0] temp_reg_ext;
	assign temp_reg_ext = {reg_3, reg_2, reg_1, reg_0};
	assign temp_reg = temp_reg_ext[63 -: RESOLUTION];
	
		//Assign write dataset setting and command
	assign write_dataset = reg_control[15:8];
	assign command = reg_control[7:0];
	
	//Process to assign the variables depending on the control register
	//syn. with the slow clk
	//always @(posedge clk_slow) 
	always @(posedge clk) 
	begin
		if(nReset == 1'b0)
		begin
			current_dataset <= 0;
			use_ext_dataset <= 0;
			num_cycle <= 100;
			input_ready <= 0;
			if (NR_DATASETS > 1)
				for (i = 0; i < NR_DATASETS; i = i+1)
				begin
					Yset_buf[i] <= 0;
					Rset_buf[i] <= 0;
					RIset_buf[i] <= 0;
					ROset_buf[i] <= 0;	
				end			
		end
		else
		begin

			case (command)
				CMD_WRITE_YSET: 
				begin	
					temp_Yset = temp_reg;
					input_ready <= 0;
				end
				CMD_WRITE_RSET:	
				begin
					temp_Rset = temp_reg;
					input_ready <= 0;
				end
				CMD_WRITE_RISET:
				begin	
					temp_RIset = temp_reg;
					input_ready <= 0;
				end
				CMD_WRITE_ROSET:
				begin  
					temp_ROset = temp_reg;
					input_ready <= 0;
				end
				CMD_UPDATE:
				begin
					input_ready <= 1;
					if (NR_DATASETS > 1)
					begin
						Yset_buf[write_dataset] <= temp_Yset;
						Rset_buf[write_dataset] <= temp_Rset;
						RIset_buf[write_dataset] <= temp_RIset;
						ROset_buf[write_dataset] <= temp_ROset;		
					end				
				end
				CMD_SW_DATASET:
				begin
					if (NR_DATASETS > 1)
					begin
						current_dataset <= write_dataset;
						use_ext_dataset <= 0;
					end	
				end
				CMD_EXT_DATASET:
					if (NR_DATASETS > 1)
						use_ext_dataset <= 1;
				CMD_NUM_CYCLE:
					num_cycle <= temp_reg_ext[31:0];	//corrected @ rev. 6
				CMD_HALT:
				begin
					input_ready <= 1;
					if (NR_DATASETS > 1)
					begin
						Yset_buf[write_dataset] <= Yis;
						Rset_buf[write_dataset] <= temp_Rset;
						RIset_buf[write_dataset] <= temp_RIset;
						ROset_buf[write_dataset] <= temp_ROset;	
					end
					else
					begin
						temp_Yset <= Yis;
					end	
				end
				
				CMD_READ_YSET:
					if (READOUT & CAP_READ_SETVAL)
					begin
						if (NR_DATASETS > 1)
							outreg <= {Yset_buf[write_dataset], {(64-RESOLUTION){1'b0}} };
						else
							outreg <= {temp_Yset, {(64-RESOLUTION){1'b0}} };
					end		
				CMD_READ_RSET:
					if (READOUT & CAP_READ_SETVAL)
					begin
						if (NR_DATASETS > 1)
							outreg <= {Rset_buf[write_dataset], {(64-RESOLUTION){1'b0}} };
						else	
							outreg <= {temp_Rset, {(64-RESOLUTION){1'b0}} };
					end		
				CMD_READ_RISET:
					if (READOUT & CAP_READ_SETVAL)
					begin
						if (NR_DATASETS > 1)
							outreg <= {RIset_buf[write_dataset], {(64-RESOLUTION){1'b0}} };
						else
							outreg <= {temp_RIset, {(64-RESOLUTION){1'b0}} };
					end		
				CMD_READ_ROSET:
					if (READOUT & CAP_READ_SETVAL)
					begin
						if (NR_DATASETS > 1)
							outreg <= {ROset_buf[write_dataset], {(64-RESOLUTION){1'b0}} };
						else
							outreg <= {temp_ROset, {(64-RESOLUTION){1'b0}} };
					end		
						
				CMD_READ_DATASET:
					if (READOUT & CAP_READ_DATASET)
					begin
						if (NR_DATASETS > 1)
							outreg <= {{(47){1'b0}}, use_ext_dataset, ext_dataset, current_dataset};
						else
							outreg <= 0;
					end		
				CMD_READ_NUM_CYCLE:
					if (READOUT & CAP_READ_NUM_CYCLE)
						outreg <= {{(32){1'b0}}, num_cycle};			
			
				CMD_READ_YIS:
					if (READOUT & CAP_READ_YIS)
						outreg <= {Yis, {(64-RESOLUTION){1'b0}} };
				CMD_READ_RIS:
					if (READOUT & CAP_READ_RIS)
						outreg <= {Ris, {(64-RESOLUTION){1'b0}} };
						
				CMD_READ_PARAMS:
					outreg <= {{(8){1'b0}}, RESOLUTION[7:0], DAC_WIDTH[7:0], NR_DATASETS[7:0], READOUT[7:0]	};				
			endcase
		end
	end
	
	
	//process to control the calculations timing
	always @(posedge clk_slow) 
	begin
		if(nReset == 1'b0)
		begin
			time_step_tc = 0;
			time_step = 100;	//temp_num_cycle;
		end
		else if (time_step_tc == 1)
			time_step = num_cycle; 
      		else //if (clk_slow == 1) 
			time_step = time_step-1;	
		
		if (time_step == 0)
			time_step_tc = 1;
		else
			time_step_tc = 0;
	end
	

	
endmodule
