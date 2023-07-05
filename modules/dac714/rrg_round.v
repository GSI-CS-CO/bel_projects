/* This code for realtime ramp generator, in which it is executed to obtain come curvy edges while ramping on and out,
	and in between the normal slop ramping.
	In addition the values could be changed anytime, as it is not a condition to be in 
	steady state.
	
	The inputs has one control register that control to which variable this input value will be assigned
	and has another 4 registers to write the required values to the variables.
*/


 module rrg_round(
	input clk,
	input clk_slow,
	input nReset,
	input timepulse,								//a pulse every 1us
	input [15:0] reg_control ,
	input signed [15:0] reg_0, 
	input signed[15:0] reg_1,  
	input signed[15:0] reg_2,  
	input signed[15:0] reg_3,
	input [15:0] num_cycle ,

	output reg DACStrobe,
	output reg signed [63:0] Yis,
	output reg signed [63:0] Ris
	);

	reg signed [63:0] switch_mode =64'h0001000000000000;
	reg signed [63:0] temp_reg =0;
	reg signed [63:0] temp_Yset=0;
	reg signed[63:0] temp_Rset=0;  
	reg signed[63:0] temp_RIset=0;  
	reg signed[63:0] temp_ROset=0;
	reg unsigned [63:0] temp_large_cycle = 1000;
	reg signed[63:0] switch_mode_reg=64'h0001000000000000;
	reg unsigned [63:0] large_cycle = 1000;
	
	reg signed [63:0] Yis_copy1=0;
	reg signed [63:0]	Yis_copy2=0;
	reg signed [63:0]	Yis_state=0;
	
	reg unsigned [3:0]time_step_tc=0;
	reg unsigned [63:0]time_step=1000;
	
	reg signed [63:0] Yset=0;
	reg signed[63:0] Rset=0;  
	reg signed[63:0] RIset=0;  
	reg signed[63:0] ROset=0;
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
		end
		else
		begin
			
			//once the time_step_tc is high we begin calculations
			//this is done to slow down the calculation speed as the DAC could recognize it.
				if(time_step_tc ==1)
				
				// Algorithm begins
				begin
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
						
				if( abs_Ydiff <= ROset && abs_Ris <= ROset)
				begin
				
					Yis_copy1= Yset;
			
					Yis_state = 64'h1000000000000000;
							
					Ris = 64'd0;
				end
				
				else
				begin
				
						Yt_dash = (Yset*ROset) -( Sign *((Ris **2)/ (2)) );
						if ((Sign *(Yis_copy1*ROset -Yt_dash))>0)
						begin
						

							Ris = Ris - (Sign_Ris * (ROset));
							
							Yis_copy1= Yis_copy1+Ris;
							
							Yis_state = 64'h0000000000000000;
						end
											
						else
						begin
						
							if ((Sign *Ris) -Rset <-1*RIset)
							begin
								Ris = Ris + Sign *RIset;
								
								Yis_copy1= Yis_copy1+Ris;

								Yis_state = 64'h3000000000000000;
							
							end
							
							else if ((Sign *Ris) -Rset > ROset)
							begin
								Ris = Ris - (Sign *RIset);
								
								Yis_copy1= Yis_copy1+Ris;
								
								Yis_state = 64'h4000000000000000;
							end
							
							
							else
							begin
							Ris = Sign *Rset;
							
							Yis_copy1= Yis_copy1+Ris;
							
							Yis_state = 64'h5000000000000000;
							
							end
						end
				end		
			end
		end
		
	end


	//Process to assign the variables depending on the control register
	//syn. with the slow clk
	always @(posedge clk_slow) 
	begin

	if(nReset == 1'b0)
	begin

		Yset = 0;
		Rset = 0;
		RIset = 0;
		ROset = 0;
		switch_mode =64'h0001000000000000;
	end
	
	else
begin

	temp_reg = {reg_3, reg_2, reg_1, reg_0};

	if (reg_control == 1) 
		temp_Yset = temp_reg;
		
	else if (reg_control == 2)
		temp_Rset = temp_reg;
		
	else if (reg_control == 3)
		temp_RIset = temp_reg;
		
	else if (reg_control == 4)
		temp_ROset = temp_reg;
		//added switch for debugging on the FPGAs	
	else if (reg_control ==6)
		switch_mode_reg = temp_reg;
	else if (reg_control ==7)
		temp_large_cycle = temp_reg;
	else if (reg_control == 5)
	begin
		Yset = temp_Yset;
		Rset = temp_Rset;
		RIset = temp_RIset;
		ROset = temp_ROset;
		large_cycle =temp_large_cycle;
		//switch_mode = switch_mode_reg;
	end
	

	
	end
	end
	
	
	//Process to buffer the output
	always @(posedge clk_slow) 
	begin
	
		if(nReset == 1'b0)
			Yis_copy2= 64'd0;

		
		else
		Yis_copy2 = Yis_copy1;
		
	end
	
	
	//process to control the calculations timing
		always @(posedge clk_slow) 
	begin
	
		if(nReset == 1'b0)
			time_step_tc= 64'd0;

		
		else if (time_step_tc == 1)
                time_step = large_cycle;    
         else if (clk_slow == 1) 
                time_step = time_step-1;
		if (time_step ==0)
			time_step_tc =1;
			else
			time_step_tc =0;
		
	end
	
	
	//Process to push the outputs
	
	always @(posedge clk) 
	begin
	
		if(nReset == 1'b0)
			Yis= 64'd0;

		else
		begin
			DACStrobe = time_step_tc;
			
			//if(switch_mode ==64'h0001000000000000)
			Yis = Yis_copy2;
			
			//this is added for testing
			/*else if (switch_mode ==64'h0002000000000000)
			Yis = Yis_state;*/
			
		end
	end
	
	
endmodule
