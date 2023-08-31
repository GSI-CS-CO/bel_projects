  --component ref_pll is   -- arria2
  --  port(
  --    areset : in  std_logic;
  --    inclk0 : in  std_logic := '0'; -- 125 MHz
  --    c0     : out std_logic;        -- 125 MHz
  --    c1     : out std_logic;        -- 200 MHz
  --    c2     : out std_logic;        --  25 MHz
  --    locked : out std_logic;
  --    scanclk            : in  std_logic;
  --    phasecounterselect : in  std_logic_vector(3 downto 0);
  --    phasestep          : in  std_logic;
  --    phaseupdown        : in  std_logic;
  --    phasedone          : out std_logic);
  --end component;

library ieee;
use ieee.std_logic_1164.all;
entity ref_pll5 is
	port (
      refclk     : in  std_logic := 'X'; -- 125 MHz
      outclk_0   : out std_logic;        -- 125 MHz
      outclk_1   : out std_logic;        -- 200 MHz
      outclk_2   : out std_logic;        --  25 MHz
      outclk_3   : out std_logic;        --1000 MHz
      outclk_4   : out std_logic;        -- 125 MHz, 1/8 duty cycle, -1.5ns phase
      rst        : in  std_logic := 'X';
      locked     : out std_logic;
      scanclk    : in  std_logic;
      cntsel     : in  std_logic_vector(4 downto 0);
      phase_en   : in  std_logic;
      updn       : in  std_logic;
      phase_done : out std_logic);
end entity;
architecture simulation of ref_pll5 is
	signal phasecounterselect : std_logic_vector(3 downto 0);
begin
	phasecounterselect <= cntsel(3 downto 0);
	pll: entity work.ref_pll 
	port map (
		areset             => rst, 
		inclk0             => refclk, 
		c0                 => outclk_0, 
		c1                 => outclk_1, 
		c2                 => outclk_2, 
		locked             => locked, 
		scanclk            => scanclk, 
		phasecounterselect => phasecounterselect, 
		phasestep          => phase_en, 
		phaseupdown        => updn, 
		phasedone          => phase_done);
	outclk_3 <= '0';
	outclk_4 <= '0';
end architecture;

--library ieee;
--use ieee.std_logic_1164.all;
--entity ref_pll10 is  -- arria10
--port(
--  refclk     : in  std_logic := 'X'; -- 125 MHz
--  --outclk_0   : out std_logic;        -- 125 MHz
--  --outclk_1   : out std_logic;        -- 200 MHz
--  --outclk_2   : out std_logic;        --  25 MHz
--  --outclk_3   : out std_logic;        --1000 MHz
--  --outclk_4   : out std_logic;        -- 125 MHz, 1/8 duty cycle, -1.5ns phase
--  outclk_2   : out std_logic; -- 125 MHz
--  outclk_3   : out std_logic; -- 200 MHz
--  outclk_4   : out std_logic; -- 25 MHz
--  lvds_clk   : out std_logic_vector(1 downto 0); --1000 MHz
--  loaden     : out std_logic_vector(1 downto 0); -- 125 MHz, 13% duty cycle, 7000ps phase
--  rst        : in  std_logic := 'X';
--  locked     : out std_logic;
--  scanclk    : in  std_logic;
--  cntsel     : in  std_logic_vector(4 downto 0);
--  phase_en   : in  std_logic;
--  updn       : in  std_logic;
--  phase_done : out std_logic);
--end entity;
--architecture simulation of ref_pll10 is
--	signal phasecounterselect : std_logic_vector(3 downto 0);
--begin
----..
--end architecture;


