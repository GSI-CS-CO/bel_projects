module rrg_round(
	input clk,
	input nReset,
	input timepulse,
	input signed [15:0] Yset, 
	input signed[15:0] Rset,  
	input signed[15:0] RIset,  
	input signed[15:0] ROset,
	output reg DACStrobe,
	output reg signed [31:0] Yis,
	output reg signed [31:0] Ris
	);

	reg signed [15:0] Ydiff;
	reg [15:0] abs_Ydiff;
	reg [15:0] abs_Ris;
	reg signed [31:0] Yt;

	always @(posedge clk) 
	begin
	
		if(nReset == 1'b0)
			Yis = 32'd0;
		else
			begin
				DACStrobe =timepulse;
				Ydiff = Yset -Yis;
				if (Ydiff[15] == 1'b1)
						abs_Ydiff = {1'b0,Ydiff[14:0]};
				if (Ris[15] == 1'b1)
						abs_Ris = {1'b0,Ris[14:0]};//-Ris;
						
				if( abs_Ydiff <= ROset && abs_Ris <= ROset)
				begin
					Yis = Yset;
					Ris = 32'd0;
				end
				
				else
				begin
					if (Ydiff[15] == 1'b1)
					begin
						Yt = Yset +( ((Ris **2)/ (2* ROset)) );
						
						if (-1*(Ris -Yt) >0)
						begin
						if (Ris == 1'b1)
						begin
							Ris = Ris + (Ris *ROset);
							Yis = Yis +Ris;
							end
						else
						begin
							Ris = Ris - (Ris *ROset);
							Yis = Yis +Ris;
						end
						end
						
						else
						begin
						
							if (-Ris -Rset <-RIset)
							begin
								Ris = Ris - RIset;
								Yis = Yis +Ris;
							end
							
							else if (-Ris -Rset > ROset)
							begin
								Ris = Ris +RIset;
								Yis = Yis+Ris;
							end
							
							else
							begin
							Ris = -1*Rset;
							Yis = Yis +Ris;
							end
						end
					end
					else if (Ydiff[15] == 1'b0)
					begin
						Yt = Yset -( ((Ris **2)/ (2* ROset)) );
						if ((Ris -Yt) >0)
						begin
							Ris = Ris - (Ris *ROset);
							Yis = Yis +Ris;
						end
						
						else
						begin
						
							if (Ris -Rset <-RIset)
							begin
								Ris = Ris + RIset;
								Yis = Yis +Ris;
							end
							
							else if (Ris -Rset > ROset)
							begin
								Ris = Ris - RIset;
								Yis = Yis+Ris;
							end
							
							else
							begin
							Ris = Rset;
							Yis = Yis +Ris;
							end
						end
					end
						
				
				end
		end
	
	end

	
endmodule