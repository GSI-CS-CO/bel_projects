-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

Library
UNISIM;
use UNISIM.vcomponents.all;

-- pexp-TR Entity
entity pexp_tr_prog is
  Port (
    pgclk_i         : in    std_logic;                    -- clock from 50 MHz oscillator
    CONFIG_SPV_i    : in    std_logic;                    -- input from reset chip (on schematics CONFIG_SPV)

    mres1_o         : out   std_logic;                    -- reset output to reset chip then to FPGA "nres"

    CONF_DONE_i     : in    std_logic;                    -- from FPGA, low during configuration
    nCONFIG_PROG_io : inout std_logic;                    -- input from reset chip (on schematics CONFIG)
    fpga_con_io     : out   std_logic_vector(5 downto 1); -- connection to/from fpga
    sel_clk_o       : out   std_logic_vector(1 downto 0); -- output to gbit switch
    config_mode_o   : out   std_logic_vector(4 downto 0); -- config mode to FPGA 
    fpga_res_o      : out   std_logic;                    -- output to FPGA, optional reset
    pled_o          : inout std_logic_vector(5 downto 1); -- 5 leds
    nstatus_i       : in    std_logic;                    -- status to/from fpga
    hsw_i           : in    std_logic_vector(4 downto 1); -- input from hex switch
    pbf1_i          : in    std_logic                     -- input from push button
  );
end pexp_tr_prog;

-- pexp-TR Architecture
architecture rtl of pexp_tr_prog is

  -- internal signals
  signal clk     : std_logic;
  signal countx  : std_logic_vector(26 downto 0); -- counter
  signal leds    : std_logic_vector(5 downto 1);  
  
begin

  i_BUFG_clk : BUFG
  port map (
    I => pgclk_i,  -- Clock buffer input
    O => clk -- Clock buffer output
  );
 
  
  -- fixed configuration
  config_mode_o   <= b"10010"; -- master SPI
  sel_clk_o(0)    <= '1';      -- in1 to q0 SW1
  sel_clk_o(1)    <= '1';      -- in1 to q1 SW1
  nCONFIG_PROG_io <= '1';      -- immediaty ready
  
  -- reset outputs
  mres1_o     <= '1';
  fpga_res_o  <= '1';
  
  -- counter
  process(clk) begin
    if (rising_edge(clk)) then 
      countx <= countx + 1;
    end if;
  end process;
  
  -- leds
  leds(1) <= not(CONF_DONE_i);  -- yellow
  leds(2) <= nCONFIG_PROG_io;   -- red
  leds(3) <= CONFIG_SPV_i;      -- white
  leds(4) <= nstatus_i;         -- blue
  leds(5) <= countx(26);        -- green

  
  -- when CPLD button pressed then show CPLD hex switch state
  pled_o <= (countx(24) & hsw_i) when pbf1_i = '0' else leds;
  
  -- connection to fpga
  fpga_con_io(4 downto 1) <= not hsw_i;
  fpga_con_io(5)          <= not pbf1_i;
  
end;