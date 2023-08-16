--library ieee;
--use ieee.std_logic_1164.all;
--entity sys_pll is
--	port
--	(
--		areset	: in std_logic  := '0';
--		inclk0	: in std_logic  := '0'; -- 125
--		c0		: out std_logic ; --62.5 *1/2
--		c1		: out std_logic ; --100  *4/5
--		c2		: out std_logic ; --20   *4/25
--		c3		: out std_logic ; --10   *2/25
--		locked	: out std_logic 
--	);
--end sys_pll;
--architecture simulation of sys_pll is
--	signal t_rising, t_falling, half_period : time := 1 ns;
--	signal clk  : std_logic_vector(0 to 3) := (others => '1');
--	signal lock : std_logic := '0';
--begin
--	clk(0) <= not clk(0) after half_period *  2 / 1;
--	clk(1) <= not clk(1) after half_period *  5 / 4;
--	clk(2) <= not clk(2) after half_period * 25 / 4;
--	clk(3) <= not clk(3) after half_period * 25 / 2;
--	c0 <= clk(0);
--	c1 <= clk(1);
--	c2 <= clk(2);
--	c3 <= clk(3);
--	measure: process 
--	begin
--		wait until rising_edge(inclk0);
--		t_rising <= now;
--		wait until falling_edge(inclk0);
--		--report "now = " & time'image(now) & "    t_rising = " & time'image(t_rising);
--		half_period <= now - t_rising;
--	end process;
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
entity sys_pll5 is  -- arria5
    port(
      refclk   : in  std_logic; 
      outclk_0 : out std_logic; 
      outclk_1 : out std_logic; 
      outclk_2 : out std_logic; 
      outclk_3 : out std_logic; 
      outclk_4 : out std_logic; 
      rst      : in  std_logic;
      locked   : out std_logic);
end entity;
architecture simulation of sys_pll5 is
	signal out1 : std_logic;
begin
	pll : entity work.sys_pll port map(rst,refclk,outclk_0,out1,outclk_2,outclk_3,locked);
	outclk_1 <= out1;
	outclk_4 <= out1;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
entity sys_pll10 is  -- arria5
    port(
      refclk   : in  std_logic; 
      outclk_0 : out std_logic; 
      outclk_1 : out std_logic; 
      outclk_2 : out std_logic; 
      outclk_3 : out std_logic; 
      outclk_4 : out std_logic; 
      rst      : in  std_logic;
      locked   : out std_logic);
end entity;
architecture simulation of sys_pll10 is
	signal out1 : std_logic;
begin
	pll : entity work.sys_pll port map(rst,refclk,outclk_0,out1,outclk_2,outclk_3,locked);
	outclk_1 <= out1;
	outclk_4 <= out1;
end architecture;
