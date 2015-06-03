-- libraries
-- -------------------------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.ALL;
use ieee.std_logic_unsigned.ALL;

-- entity 
-- -------------------------------------------------------------------------------------------------
entity exploder5_cpld is
  port (
    config   : inout std_logic;                    -- input from reset chip (on schematics CONFIG)
    cdone    : in    std_logic;                    -- from FPGA, low during configuration
    pgclk    : in    std_logic;                    -- clock from 50 MHz oscillator
    crc_er   : in    std_logic;                    -- CRC error (driven by FPGA)
    sel_clk  : inout std_logic_vector(3 downto 0); -- output to gbit switch
    m        : out   std_logic_vector(4 downto 0); -- config mode to FPGA 
    con      : inout std_logic_vector(9 downto 1); -- connection to/from fpga
    pled     : inout std_logic_vector(5 downto 1); -- 4 leds
    mres1    : in    std_logic;                    -- reset output to reset chip then to FPGA "nres"
    fpga_res : in    std_logic;                    -- output to FPGA, optional reset
    config1  : in    std_logic;                    -- input from reset chip (on schematics CONFIG1)
    nstat    : in    std_logic                     -- status to/from fpga
  );
end exploder5_cpld;

-- architecture 
-- -------------------------------------------------------------------------------------------------
architecture rtl of exploder5_cpld is
  signal countx : std_logic_vector(31 downto 0); -- counter
begin
  
  -- configuration pins
  m          <= b"10010"; -- master SPI
  sel_clk(0) <= '1';      -- in1 to q0 SW1
  sel_clk(1) <= '1';      -- in1 to q1 SW1
  sel_clk(2) <= '0';      -- in0 to q0 SW2
  sel_clk(3) <= '0';      -- in1 to q1 SW2
  config     <= '1';      -- immediaty ready
    
  -- counter
  process(pgclk) begin
    if (rising_edge(pgclk)) then 
      countx <= countx + 1;
    end if;
  end process;
  
  -- drive LEDs
  pled(1)  <= not(cdone);   -- yellow
  pled(2)  <= config;  -- red
  pled(3)  <= config1; -- white
  pled(4)  <= crc_er;  -- blue
  pled(5)  <= nstat;   -- green
  
  -- drive additional connections pins to fpga
  con(9 downto 1) <= (others => 'Z');
  
end;
