-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-- pexp-TR Entity
entity pexp_tr_prog is
  Port (
    cdone    : in    std_logic;                    -- from FPGA, low during configuration
    confix   : inout std_logic;                    -- input from reset chip (on schematics CONFIG)
    config1  : in    std_logic;                    -- input from reset chip (on schematics CONFIG1)
    con      : out   std_logic_vector(5 downto 1); -- connection to/from fpga
    pgclk_i  : in    std_logic;                    -- clock from 50 MHz oscillator
    sel_clk_o: inout std_logic_vector(1 downto 0); -- output to gbit switch
    hsw      : in    std_logic_vector(4 downto 1); -- input from hex switch
    m        : out   std_logic_vector(4 downto 0); -- config mode to FPGA
    fpga_res : in    std_logic;                    -- output to FPGA, optional reset
    pled     : inout std_logic_vector(5 downto 1); -- 5 leds
    nstat    : in    std_logic;                    -- status to/from fpga
    mres     : in    std_logic;                    -- reset output to reset chip then to FPGA reconfig
    mres1    : in    std_logic;                    -- reset output to reset chip then to FPGA "nres"
    pbs1     : in    std_logic                     -- input from push button
  );
end pexp_tr_prog;

-- pexp-TR Architecture
architecture rtl of pexp_tr_prog is

  -- internal signals
  signal countx  : std_logic_vector(26 downto 0); -- counter

begin

  -- fixed configuration
  m          <= b"10010"; -- master SPI
  sel_clk_o(0) <= '1';      -- in1 to q0 SW1
  sel_clk_o(1) <= '1';      -- in1 to q1 SW1
  confix     <= '1';      -- immediaty ready

  -- counter process
  process(pgclk_i) begin
    if (rising_edge(pgclk_i)) then
      countx <= countx + 1;
    end if;
  end process;

  -- leds
  pled(1) <= not(cdone); -- yellow
  pled(2) <= confix;     -- red
  pled(3) <= config1;    -- white
  pled(4) <= nstat;      -- blue
  pled(5) <= pgclk_i;      -- green

  -- connection to fpga
  con(4 downto 1) <= hsw;
  con(5)          <= pbs1;

end;
