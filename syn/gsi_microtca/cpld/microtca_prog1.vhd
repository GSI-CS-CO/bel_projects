library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- for PEXARIA5
entity prog1 is
    Port (	
--........................................
		cdone	  	: in std_logic; -- from FPGA, low during configuration
		confix	: inout std_logic; -- input from reset chip (on schematics CONFIG)
		config1	: in std_logic; -- input from reset chip (on schematics CONFIG1)
		con	  	: in std_logic_vector(5 downto 1); -- connection to/from fpga
		pgclk 	: in std_logic; -- clock from 50 MHz oscillator
--
		sel_clk	: inout std_logic_vector(3 downto 0); -- output to gbit switch
--		
		hsw	  	: in std_logic_vector(4 downto 1); -- input from hex switch
		m	  		: out std_logic_vector(4 downto 0); -- config mode to FPGA 
		fpga_res	: in std_logic; -- output to FPGA, optional reset
		pled	  	: inout std_logic_vector(5 downto 1); -- 4 leds
		nstat 	: in std_logic; -- status to/from fpga
		mres	  	: in std_logic; -- reset output to reset chip then to FPGA reconfig
		mres1	  	: in std_logic; -- reset output to reset chip then to FPGA "nres"
		pbs1 		: in std_logic -- input from push button


-- AMC-TR Architecture
architecture rtl of pmc_tr_prog is

  -- internal signals
  signal countx  : std_logic_vector(26 downto 0); -- counter
  signal leds    : std_logic_vector(4 downto 0);  

begin
  
  -- fixed configuration
  m          <= b"10010"; -- master SPI
  sel_clk(0) <= '1';      -- in1 to q0 SW1
  sel_clk(1) <= '1';      -- in1 to q1 SW1
  confix     <= '1';      -- immediaty ready
  
  -- counter process
  process(pgclk) begin
    if (rising_edge(pgclk)) then 
      countx <= countx + 1;
    end if;
  end process;
  
  -- leds
  leds(1) <= not(cdone); -- yellow
  leds(2) <= confix;     -- red
  leds(3) <= config1;    -- white
  leds(4) <= nstat;      -- blue
  leds(5) <= countx(26); -- green
  
  -- when CPLD button pressed then show CPLD hex switch state
  pled <= (countx(24) & hsw) when pbs1 = '0' else leds;

  -- connection to fpga
  con(4 downto 1) <= not hsw;
  con(5)          <= not pbs1;
  
end;
