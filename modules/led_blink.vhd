library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;


entity led_blink is
	generic (
				count:				integer := 8;
				Clk_in_Hz:			integer := 100_000_000;
				blink_duration:		integer := 1_000_000_000		-- 1s
			);
	port (
			clk:	in std_logic;
			nrst:	in std_logic;
			led:	out std_logic_vector(count-1 downto 0)
		);
end entity;

architecture led_blink_arch of led_blink is



constant c_blink_d:				integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(blink_duration)));
constant c_blink_d_width:		integer := integer(floor(log2(real(c_blink_d)))) + 2;
signal	s_blink_d_cnt:			unsigned(c_blink_d_width-1 downto 0) := (others => '0');
signal s_led:					std_logic_vector(count-1 downto 0);

begin

	



	cnt: process(clk)
	begin
		if rising_edge(clk) then
			if nrst = '0' then
				s_blink_d_cnt <= to_unsigned(c_blink_d, c_blink_d_width);
			else
				s_blink_d_cnt <= s_blink_d_cnt - 1;
				if s_blink_d_cnt = 0 then
					s_blink_d_cnt <= to_unsigned(c_blink_d, c_blink_d_width);
					s_led <= not s_led;
				end if;
			end if;
		end if;
	end process;
	
	led <= s_led;

end architecture;