-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-- PMC-TR Entity
entity pmc_tr_prog is
  Port (
    cdone    : in    std_logic;                    -- from FPGA, low during configuration
    confix   : inout std_logic;                    -- input from reset chip (on schematics CONFIG)
    config1  : in    std_logic;                    -- input from reset chip (on schematics CONFIG1)
    con      : out   std_logic_vector(5 downto 1); -- connection to/from fpga
    pgclk    : in    std_logic;                    -- clock from 50 MHz oscillator
    sel_clk  : inout std_logic_vector(1 downto 0); -- output to gbit switch
    hsw      : in    std_logic_vector(4 downto 1); -- input from hex switch
    m        : out   std_logic_vector(4 downto 0); -- config mode to FPGA 
    fpga_res : in    std_logic;                    -- output to FPGA, optional reset
    pled     : inout std_logic_vector(5 downto 1); -- 5 leds
    nstat    : in    std_logic;                    -- status to/from fpga
    mres     : in    std_logic;                    -- reset output to reset chip then to FPGA reconfig
    mres1    : in    std_logic;                    -- reset output to reset chip then to FPGA "nres"
    pbs1     : in    std_logic                     -- input from push button
  );
end pmc_tr_prog;

-- PMC-TR Architecture
architecture rtl of pmc_tr_prog is

  -- internal signals
  signal countx  : std_logic_vector(26 downto 0); -- counter
  
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
  pled(1) <= not(cdone); -- yellow
  pled(2) <= confix;     -- red
  pled(3) <= config1;    -- white
  pled(4) <= nstat;      -- blue
  pled(5) <= pgclk;      -- green
  
  -- connection to fpga
  con(4 downto 1) <= hsw;
  con(5)          <= pbs1;
  
end;
