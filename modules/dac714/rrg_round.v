//// a simpler code than the below one

module rrg_round(
	input clk,
	input clk_slow,
	input nReset,
	input timepulse,
	input [15:0] reg_control ,
	input signed [15:0] reg_0, 
	input signed[15:0] reg_1,  
	input signed[15:0] reg_2,  
	input signed[15:0] reg_3,

	output reg DACStrobe,
	output reg signed [63:0] Yis,
	output reg signed [63:0] Ris
	);

	reg signed [63:0] temp_reg =0;
	reg signed [63:0] temp_Yset=0;
	reg signed[63:0] temp_Rset=0;  
	reg signed[63:0] temp_RIset=0;  
	reg signed[63:0] temp_ROset=0;
	
	reg signed [63:0] Yis_copy1;
	reg signed [63:0]	Yis_copy2;
	
	reg signed [63:0] Yset;
	reg signed[63:0] Rset;  
	reg signed[63:0] RIset;  
	reg signed[63:0] ROset;
	reg signed [63:0] Ydiff;
	reg [63:0] abs_Ydiff;
	reg [63:0] abs_Ris;
	reg signed [127:0] Yt_dash;

	reg signed [63:0] Sign =1;
	reg signed [63:0] Sign_Ris =1;

	always @(posedge clk_slow) 
	begin
	
		if(nReset == 1'b0)
		begin
			Yis_copy1= 64'd0;
			Ris = 64'd0;
			Sign =1;
			Sign_Ris=1;
		end
		else
		begin
			
			
				//DACStrobe =timepulse;
				Ydiff = Yset -Yis;
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
					Ris = 64'd0;
				end
				
				else
				begin

						Yt_dash = (Yset*ROset) -( Sign *((Ris * Ris)/ (2)) );
						if ((Sign *((Yis*ROset) -Yt_dash)) >0)
						begin
						
						if (Ris !=0)
						begin
							Ris = Ris - (Sign_Ris * (ROset));
							Yis_copy1= Yis_copy1+Ris;
						end
						end
						
							
						
						
						else
						begin
						
							if ((Sign *Ris) -Rset <-1*RIset)
							begin
								Ris = Ris + Sign *RIset;
								Yis_copy1= Yis_copy1+Ris;
							end
							
							else if ((Sign *Ris) -Rset > ROset)
							begin
								Ris = Ris - (Sign *RIset);
								Yis_copy1= Yis+Ris;
							end
							
							
							else
							begin
							Ris = Sign *Rset;
							Yis_copy1= Yis_copy1+Ris;
							end
						end
				end		////////////////////////////////
			
		end
		
	end



	always @(posedge clk_slow) 
	begin

	if(nReset == 1'b0)
	begin

		Yset = 0;
		Rset = 0;
		RIset = 0;
		ROset = 0;
	end
	else
begin

	temp_reg = {reg_3, reg_2, reg_1, reg_0};

	if (reg_control == 1) //1
	begin
		temp_Yset = temp_reg;
	end
	else if (reg_control == 2)//2
	begin
		temp_Rset = temp_reg;
	end
	else if (reg_control == 3)//4
	begin
		temp_RIset = temp_reg;
	end
	else if (reg_control == 4)//3
	begin
		temp_ROset = temp_reg;
	end
	else if (reg_control == 5)//7
	begin
		Yset = temp_Yset;
		Rset = temp_Rset;
		RIset = temp_RIset;
		ROset = temp_ROset;
	end
	end
	end
	
	
	always @(posedge clk_slow) 
	begin

	Yis_copy2 = Yis_copy1;
	end
	
	
	always @(posedge clk) 
	begin
	DACStrobe =timepulse;
	Yis = Yis_copy2;
	end
	
endmodule


// in this code the if condition is divided to two main parts
// the first when the Ydiff is positive and the other when it
// is negative
/*module rrg_round(
	input clk,
	input nReset,
	input timepulse,
	input [15:0] reg_control ,
	input signed [15:0] reg_0, 
	input signed[15:0] reg_1,  
	input signed[15:0] reg_2,  
	input signed[15:0] reg_3,

	output reg signed [63:0] Yset,
	output reg signed[63:0] Rset,
	output reg signed[63:0] RIset,
	output reg signed[63:0] ROset,
	output reg signed[63:0] Yt,


	output reg DACStrobe,
	output reg signed [63:0] Yis,
	output reg signed [63:0] Ris
	);

	reg signed [63:0] temp_reg;
	reg signed [63:0] temp_Yset;
	reg signed[63:0] temp_Rset;  
	reg signed[63:0] temp_RIset;  
	reg signed[63:0] temp_ROset;
	/reg signed [63:0] Yset; -- needed for testbench
	//reg signed[63:0] Rset; -- needed for testbench 
	//reg signed[63:0] RIset; -- needed for testbench 
	//reg signed[63:0] ROset;-- needed for testbench
	reg signed [63:0] Ydiff;
	reg [63:0] abs_Ydiff;
	reg [63:0] abs_Ris;
	//reg signed [63:0] Yt;-- needed for testbench

	always @(posedge clk) 
	begin
	
		if(nReset == 1'b0)
begin
			Yis_copy1= 64'd0;
			Ris = 64'd0;
end
		else
			begin
			
			
				DACStrobe =timepulse;
				if (timepulse ==1)
				begin
				Ydiff = Yset -Yis;
				if (Ydiff[63] == 1'b1)
						abs_Ydiff = -1*Ydiff;
				else
						abs_Ydiff = Ydiff;
				if (Ris[63] == 1'b1)
						abs_Ris = -1*Ris;//-Ris;
				else
						abs_Ris = Ris;
						
				if( abs_Ydiff <= ROset && abs_Ris <= ROset)
				begin
					Yis_copy1= Yset;
					Ris = 64'd0;
				end
				
				else
				begin
					if (Ydiff[63] == 1'b1)
					begin
						Yt = Yset +( ((Ris *Ris)/ (2* ROset)) );
						
						if (-1*(Yis_copy1-Yt) >0)
						begin

						if (Ris <0)
						begin
							Ris = Ris + (ROset);
							Yis_copy1= Yis_copy1+Ris;
							end
						else if (Ris >0)
						begin
							Ris = Ris - (ROset);
							Yis_copy1= Yis_copy1+Ris;
						end
							else if (Ris ==0)
							begin
								if (Ydiff < 0)
								begin
									Ris = Ris - (ROset);
									Yis_copy1= Yis_copy1+Ris;
								end

								else if (Ydiff >= 0)
								begin
									Ris = Ris + (ROset);
									Yis_copy1= Yis_copy1+Ris;
								end
							end
						end
						
						else
						begin
						
							if (-Ris -Rset <-RIset)
							begin
								Ris = Ris - RIset;
								Yis_copy1= Yis_copy1+Ris;
							end
							
							else if (-Ris -Rset > ROset)
							begin
								Ris = Ris +RIset;
								Yis_copy1= Yis+Ris;
							end
							
							else
							begin
							Ris = -1*Rset;
							Yis_copy1= Yis_copy1+Ris;
							end
						end
					end
					else if (Ydiff[63] == 1'b0)
					begin
						Yt = Yset -( ((Ris * Ris)/ (2* ROset)) );
						if ((Yis_copy1-Yt) >0)
						begin

						if (Ris <0)
						begin
							Ris = Ris + (ROset);
							Yis_copy1= Yis_copy1+Ris;
							end
						else if (Ris >0)
						begin
							Ris = Ris - (ROset);
							Yis_copy1= Yis_copy1+Ris;
						end
						else if (Ris ==0)
						begin
							if (Ydiff < 0)
							begin
							Ris = Ris - (ROset);
							Yis_copy1= Yis_copy1+Ris;
							end

							else if (Ydiff >= 0)
							begin
							Ris = Ris + (ROset);
							Yis_copy1= Yis_copy1+Ris;
							end
						end
						end
						
						else
						begin
						
							if (Ris -Rset <-1*RIset)
							begin
								Ris = Ris + RIset;
								Yis_copy1= Yis_copy1+Ris;
							end
							
							else if (Ris -Rset > ROset)
							begin
								Ris = Ris - RIset;
								Yis_copy1= Yis+Ris;
							end
							
							
							else
							begin
							Ris = Rset;
							Yis_copy1= Yis_copy1+Ris;
							end
						end
					end
						
				
				end
			end
		end
	
	end


// loop for passing the input to the code
	always @(posedge clk) 
	begin

	temp_reg = {reg_3, reg_2, reg_1, reg_0};

	if (reg_control == 1)
	begin
		temp_Yset = temp_reg;
	end
	else if (reg_control == 2)
	begin
		temp_Rset = temp_reg;
	end
	else if (reg_control == 3)
	begin
		temp_RIset = temp_reg;
	end
	else if (reg_control == 4)
	begin
		temp_ROset = temp_reg;
	end
	else if (reg_control == 5)
	begin
		Yset = temp_Yset;
		Rset = temp_Rset;
		RIset = temp_RIset;
		ROset = temp_ROset;
	end
	end
	
endmodule*/