library IEEE;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.ALL;

LIBRARY lpm; 
USE lpm.lpm_components.all; 


entity var_delay is


	port (
		Signal_in:	in std_logic;
		
		en_100ns:	in std_logic;
		Clk:		in std_logic;
		Reset:		in std_logic;
		
		delay:		in std_logic_vector(15 downto 0);
		
		Signal_out:	out std_logic
		);
end var_delay;


architecture var_delay_arch of var_delay is

signal	cnt1_en:		std_logic;
signal	cnt2_en:		std_logic;
signal	cnt1_out:		std_logic_vector(15 downto 0);
signal	cnt2_out:		std_logic_vector(15 downto 0);
signal	s_cnt1_set:		std_logic;
signal	s_cnt2_set:		std_logic;
signal 	cnt1_run:		std_logic;
signal 	cnt2_run:		std_logic;

signal s_pos_edge:		std_logic;
signal s_neg_edge:		std_logic;

component edge_detection is
	port (
				Clk:		in std_logic;
				Reset:		in std_logic;
				input:		in std_logic;
				pos_edge:	out std_logic;
				neg_edge:	out std_logic
			);
end component;

begin

cnt1:	lpm_counter generic map ( 	LPM_WIDTH => 16,
									LPM_DIRECTION => "DOWN"
									
								)
					port map	(	clock => Clk,
									cnt_en => cnt1_en,
									data => delay,
									q => cnt1_out,
									sload => s_cnt1_set
								);
							
cnt2:	lpm_counter generic map ( 	LPM_WIDTH => 16,
									LPM_DIRECTION => "DOWN"
									
								)
					port map	(	clock => Clk,
									cnt_en => cnt2_en,
									data => delay,
									q => cnt2_out,
									sload => s_cnt2_set
								);
								
signal_pos:	edge_detection port map (	Clk => Clk,
										Reset => Reset,
										input => Signal_in,
										pos_edge => s_pos_edge,
										neg_edge => s_neg_edge
									);

cnt1_en <= cnt1_run and en_100ns;
cnt2_en <= cnt2_run and en_100ns;


cnt_ctrl: process (Clk, s_pos_edge, s_neg_edge)
begin
	if rising_edge(Clk) then
		if s_pos_edge = '1' then
			cnt1_run <= '1';
			s_cnt1_set <= '0';
		elsif s_neg_edge = '1' then
			cnt2_run <= '1';
			s_cnt2_set <= '0';
		elsif cnt1_out(cnt1_out'HIGH) = '1' then
			-- Puls high
			Signal_out <= '1';
			
			cnt1_run <= '0';
			s_cnt1_set <= '1';	-- cnt1 ruecksetzen
		elsif cnt2_out(cnt2_out'HIGH) = '1' then
			-- Puls low
			Signal_out <= '0';
			
			cnt2_run <= '0';
			s_cnt2_set <= '1';	-- cnt2 ruecksetzen
		elsif Reset = '1' then
			Signal_out <= '0';
			s_cnt1_set <= '1';
			s_cnt2_set <= '1';
			cnt1_run <= '0';
			cnt2_run <= '0';
		end if;
		
	end if;

end process;

						
end architecture;