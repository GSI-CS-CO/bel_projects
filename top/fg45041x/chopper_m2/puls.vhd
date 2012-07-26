library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.MATH_REAL.ALL;
use IEEE.std_logic_arith.ALL;

library std;
use std.textio.all;

LIBRARY lpm; 
USE lpm.lpm_components.all; 


entity Puls is
		generic	(
					delay_cnt:		integer := 10
				);
		port (
					Pos_Edge:		IN std_logic;
					Clk:			IN std_logic;
					Reset:			IN std_logic;
				
					Puls:			Out std_logic
			);
		
	constant c_delay : integer := delay_cnt - 2;				
	constant C_LPM_WIDTH:	integer	:=	integer(floor(log2(real(c_delay)))) + 2;
end Puls;

architecture Puls_arch of Puls is

signal		s_sync_pos1, s_sync_pos2, s_sset: std_logic;
signal		Q: std_logic;
signal   	counter_q: std_logic_vector(C_LPM_WIDTH-1 downto 0);

 


begin

	Sync:	process(Clk, Reset)
	begin
		if Reset = '1' then
			s_sync_pos1 <= '0';
			s_sync_pos2 <= '0';
		elsif rising_edge(Clk) then
			s_sync_pos1 <= Pos_Edge;
			s_sync_pos2 <= s_sync_pos1;
		end if;
	end process;
	
	s_sset <= not s_sync_pos2 and s_sync_pos1;
	
	cnt: lpm_counter GENERIC MAP ( 
											LPM_WIDTH => C_LPM_WIDTH,
											LPM_TYPE => "LPM_COUNTER",
											LPM_SVALUE => integer'image(c_delay),
											LPM_DIRECTION => "DOWN"
										)
							PORT MAP	(	clock => Clk,
						 					cnt_en => Q,
											q => counter_q,
											sset => s_sset
										);
										
	SRFF:	process(Clk, Reset)
	begin
		if Reset = '1' then
			Q <= '0';
		elsif rising_edge(Clk) then
			if s_sset = '1' and counter_q(counter_q'high) = '0' then
				Q <= '1';
			elsif counter_q(counter_q'high) = '1' then
				Q <=  '0';
			end if;
		end if;
	end process;
	
	
	 Puls <= Q;
	

end Puls_arch;