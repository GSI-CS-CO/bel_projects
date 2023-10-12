/* This code for realtime ramp generator, in which it is executed to obtain come curvy edges while ramping on and out,
	and in between the normal slop ramping.
	In addition the values could be changed anytime, as it is not a condition to be in 
	steady state.
	
	The inputs has one control register that control to which variable this input value will be assigned
	and has another 4 registers to write the required values to the variables.
*/


 module rrg_round#(parameter DAC_WIDTH = 16, NR_DATASETS = 2) 
 (
	input clk,
	input clk_slow,
	input nReset,
	input timepulse,								//a pulse every 1us
	input [15:0] reg_control,
	input unsigned [15:0] reg_0, 
	input unsigned[15:0] reg_1,  
	input unsigned[15:0] reg_2,  
	input signed[15:0] reg_3,
	input unsigned [7:0] ext_dataset,				
	output reg DACStrobe,
	output reg signed [DAC_WIDTH-1:0] Yis
);

	localparam CMD_IDLE			= 0;
	localparam CMD_WRITE_YSET 	= 1;
	localparam CMD_WRITE_RSET 	= 2;
	localparam CMD_WRITE_RISET = 3;
	localparam CMD_WRITE_ROSET = 4;
	localparam CMD_UPDATE		= 5;
	localparam CMD_SW_DATASET 	= 6;
	localparam CMD_EXT_DATASET = 7;
	localparam CMD_NUM_CYCLE	= 8;

	integer i;
	
	reg [7:0] current_dataset	= 0;
	reg [7:0] write_dataset		= 0;
	reg use_ext_dataset 			= 0;
	
	reg signed [63:0] Ris=0;
	reg signed [63:0] temp_reg =0;
	reg signed [63:0] temp_Yset=0;
	reg signed[63:0] temp_Rset=0;  
	reg signed[63:0] temp_RIset=0;  
	reg signed[63:0] temp_ROset=0;
	//reg unsigned [15:0] temp_num_cycle = 60000;
	
	reg signed [63:0] Yis_copy1=0;
	reg signed [DAC_WIDTH-1:0]	Yis_copy2=0;
	//reg signed [63:0]	Yis_state=0;
	
	reg 			time_step_tc=0;
	reg unsigned [31:0]time_step=0;
	
	reg signed[63:0] Yset=0;
	reg signed[63:0] Rset=0;  
	reg signed[63:0] RIset=0;  
	reg signed[63:0] ROset=0;
	reg [31:0] num_cycle=0;
	
	reg signed[63:0] Yset_buf[NR_DATASETS-1:0];
	reg signed[63:0] Rset_buf[NR_DATASETS-1:0];  
	reg signed[63:0] RIset_buf[NR_DATASETS-1:0];  
	reg signed[63:0] ROset_buf[NR_DATASETS-1:0];
	
	reg signed [63:0] Ydiff=0;
	reg [63:0] abs_Ydiff=0;
	reg [63:0] abs_Ris=0;
	reg signed [127:0] Yt_dash=0;

	reg signed [63:0] Sign =1;
	reg signed [63:0] Sign_Ris =1;

	
	//Process for the calculations syn. with the slow clk (10MHz)
	always @(posedge clk_slow) 
	begin
	
		if(nReset == 1'b0)
		begin
			Yis_copy1= 64'd0;
			Ris = 64'd0;
			Sign =0;
			Sign_Ris=0;
			
			Yset = 0;
			Rset = 0;
			RIset = 0;
			ROset = 0;
		end
		else
		begin
			//once the time_step_tc is high we begin calculations
			//this is done to slow down the calculation speed so that the DAC can recognize it.
			if(time_step_tc == 1)
			begin
					//Update set values from dataset
				if (use_ext_dataset)
				begin
					Yset = Yset_buf[ext_dataset];
					Rset = Rset_buf[ext_dataset];
					RIset = RIset_buf[ext_dataset];
					ROset = ROset_buf[ext_dataset];			
				end
				else
				begin	
					Yset = Yset_buf[current_dataset];
					Rset = Rset_buf[current_dataset];
					RIset = RIset_buf[current_dataset];
					ROset = ROset_buf[current_dataset];					
				end

				// Algorithm begins
				Ydiff = Yset -Yis_copy1;
				if (Ydiff <0)
				begin
					Sign = -1;
					abs_Ydiff = -1*Ydiff;
				end
				else
				begin
					Sign = 1;
					abs_Ydiff = Ydiff;
				end
				
				if (Ris < 0)
				begin
					abs_Ris = -1*Ris;//-Ris;
					Sign_Ris = -1;
				end
				else
				begin
					Sign_Ris = 1;
					abs_Ris = Ris;
				end			
				
				if (ROset == 0 || RIset == 0 || Rset == 0)	//Special case: step ramping
				begin
					Yis_copy1 = Yset;
				end
				else if( abs_Ydiff <= ROset && abs_Ris <= ROset)	//Stable
				begin			
					Yis_copy1= Yset;						
					Ris = 64'd0;
				end			
				else
				begin
					Yt_dash = (Yset*ROset) - ( Sign *((Ris **2)/ (2)) );
					if ((Sign *(Yis_copy1*ROset - Yt_dash))>0)
					begin															//Rounding Out
						Ris = Ris - (Sign_Ris * (ROset));
						Yis_copy1 = Yis_copy1+Ris;
					end
					else
					begin	
						if ((Sign *Ris) -Rset <-1*RIset)
						begin														//Rounding In
							Ris = Ris + Sign *RIset;
							Yis_copy1 = Yis_copy1+Ris;
						end
						else if ((Sign *Ris) -Rset > ROset)
						begin														//Rounding in (2)
							Ris = Ris - (Sign *RIset);
							Yis_copy1 = Yis_copy1+Ris;
						end
						else
						begin
							Ris = Sign *Rset;									//Ramping
							Yis_copy1 = Yis_copy1+Ris;
						end
					end
				end		
			end
		end	
	end


	//Process to assign the variables depending on the control register
	//syn. with the slow clk
	//always @(posedge clk_slow) 
	always @(posedge clk) 
	begin
		if(nReset == 1'b0)
		begin
			current_dataset = 0;
			use_ext_dataset = 0;
			num_cycle = 1000;
			for (i = 0; i < NR_DATASETS; i = 1+1)
			begin
				Yset_buf[i] = 0;
				Rset_buf[i] = 0;
				RIset_buf[i] = 0;
				ROset_buf[i] = 0;	
			end			
		end
		else
		begin
			temp_reg = {reg_3, reg_2, reg_1, reg_0};
			
			//if (reg_control[15:8] < NR_DATASETS)
				write_dataset = reg_control[15:8];
			//else
			//	write_dataset = 0;

			case (reg_control[7:0])
				CMD_WRITE_YSET: 	
					temp_Yset = temp_reg;
				CMD_WRITE_RSET:	
					temp_Rset = temp_reg;
				CMD_WRITE_RISET:	
					temp_RIset = temp_reg;
				CMD_WRITE_ROSET:  
					temp_ROset = temp_reg;
				CMD_UPDATE:
				begin
					Yset_buf[write_dataset] = temp_Yset;
					Rset_buf[write_dataset] = temp_Rset;
					RIset_buf[write_dataset] = temp_RIset;
					ROset_buf[write_dataset] = temp_ROset;					
				end
				CMD_SW_DATASET:
				begin
					current_dataset = write_dataset;
					use_ext_dataset = 0;
				end
				CMD_EXT_DATASET:
					use_ext_dataset = 1;
				CMD_NUM_CYCLE:
					num_cycle = temp_reg[31:0];
			endcase
		end
	end
	
	//Process to buffer and clamp the output
	always @(posedge clk_slow) 
	begin
		if(nReset == 1'b0)
			Yis_copy2 = 0;
		else
			//Clamp Yis to half range
		case (Yis_copy1[63-:3])
			3'b001, 3'b010, 3'b011: //upper clamp range
				Yis_copy2 = {1'b0, {(DAC_WIDTH-1){1'b1}}};
			3'b100, 3'b101, 3'b110: //lower clamp range
				Yis_copy2 = {1'b1, {(DAC_WIDTH-1){1'b0}}};
			default: //correct range
				Yis_copy2 = Yis_copy1[61-:DAC_WIDTH];			
		endcase
	end
	
	//process to control the calculations timing
	always @(posedge clk_slow) 
	begin
		if(nReset == 1'b0)
		begin
			time_step_tc = 0;
			time_step = 1000;	//temp_num_cycle;
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
	
	
	//Process to push the outputs
	
	always @(posedge clk) 
	begin
		if(nReset == 1'b0)
			Yis= 0;
		else
		begin
			DACStrobe = time_step_tc;
			Yis = Yis_copy2;
		end
	end
	
endmodule
