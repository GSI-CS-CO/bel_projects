library IEEE;
USE ieee.std_logic_1164.all;

entity aux_signals is
	port (
			beam_control_in:	in std_logic_vector(15 downto 0);
			beam_control_out: 	in std_logic_vector(15 downto 0);
			aux_out:			out std_logic_vector(15 downto 0)
		);
end entity;

architecture aux_signals_arch of aux_signals is
begin
	
	aux_out <= 	'0' & '0' & '0' & '0' & '0' & Beam_Control_Out(10) & Beam_Control_Out(9) & Beam_Control_Out(8) &
				Beam_Control_Out(7) & Beam_Control_Out(6) & Beam_Control_Out(5) & Beam_Control_Out(4) & 
				beam_control_out(3) & beam_control_out(0) & beam_control_in(1) & beam_control_in(0);


end architecture;