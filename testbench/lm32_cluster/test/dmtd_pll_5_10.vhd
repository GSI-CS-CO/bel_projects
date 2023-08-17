--library ieee;
--use ieee.std_logic_1164.all;
--entity dmtd_pll is
--	port
--	(
--		areset :  in std_logic := '0';
--		inclk0 :  in std_logic := '0'; -- 20 MHz
--		c0     : out std_logic;        -- 62.5 MHz
--		locked : out std_logic 
--	);
--end entity;
--architecture simulation of dmtd_pll is
--	signal t_rising, t_falling, half_period : time := 1 ns;
--	signal clk  : std_logic := '1';
--	signal lock : std_logic := '0';
--begin
--	measure: process 
--	begin
--		wait until rising_edge(inclk0);
--		t_rising <= now;
--		wait until falling_edge(inclk0);
--		--report "now = " & time'image(now) & "    t_rising = " & time'image(t_rising);
--		half_period <= now - t_rising;
--	end process;

--	clk <= not clk after half_period * 8 / 25;
--	c0 <= clk;

--	pll_lock: process
--	begin
--		if areset = '1' then
--			lock <= '0';
--			wait until falling_edge(areset);
--		else 
--			for i in 1 to 10 loop
--				wait until rising_edge(inclk0);
--			end loop;
--			lock <= '1';
--		end if;
--	end process;
--	locked <= lock;
--end architecture;

library ieee;
use ieee.std_logic_1164.all;
entity dmtd_pll5 is
	port (
		rst      :  in std_logic := '0';
		refclk   :  in std_logic := '0'; -- 20 MHz
		outclk_0 : out std_logic;        -- 62.5 MHz
		locked   : out std_logic);
end entity;
architecture simulation of dmtd_pll5 is
begin
	pll: entity work.dmtd_pll port map (rst, refclk, outclk_0, locked);
end architecture;

library ieee;
use ieee.std_logic_1164.all;
entity dmtd_pll10 is
	port (
		rst      :  in std_logic := '0';
		refclk   :  in std_logic := '0'; -- 20 MHz
		outclk_0 : out std_logic;        -- 62.5 MHz
		locked   : out std_logic );
end entity;
architecture simulation of dmtd_pll10 is
begin
	pll: entity work.dmtd_pll port map (rst, refclk, outclk_0, locked);
end architecture;
