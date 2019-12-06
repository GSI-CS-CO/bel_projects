library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity top is
  port (
    clk_base_i        : in    std_logic;
    --rst_n_i           : in    std_logic;  
    scu_cb_revision   : in    std_logic_vector(3 downto 0); -- must be assigned with weak pull ups
    fpga_con_io       : inout std_logic_vector(7 downto 0); -- Connection to Arria 10
    --I2C to COMX
    i2c_scl           : in    std_logic;
    i2c_sda           : inout std_logic;
    
    nCB_rst           : in    std_logic;                    -- Reset from COMX
    nSCUext_rst_in    : in    std_logic;                    -- Reset form SCU-Bus Extension
    nExt_rst_in       : in    std_logic;                    -- Reset form Extension Connector
    nPB_rst_in        : in    std_logic;                    -- Reset form Push Button
    nFPHA_rst_in      : in    std_logic;                    -- Reset from Arria10
    nSYS_rst          : out   std_logic;                    -- Reset Out
    nPCI_rst_out      : out   std_logic;                    -- PCI Reset Out
    nExt_rst_out      : out    std_logic;                    -- Reset to Extension Connector
    led_status_o      : out   std_logic_vector(2 downto 0)
  );
end top;
--
architecture rtl of top is

  signal countx  : std_logic_vector(15 downto 0);
  signal rst_n   : std_logic;

  begin


  

  process(clk_base_i, rst_n) begin
    if (rst_n = '0') then
      countx <= (others => '0');
    elsif (rising_edge(clk_base_i)) then
      countx <= countx + 1;
    end if;
  end process;

  led_status_o(0) <= countx(15);
  led_status_o(1) <= countx(0);

  fpga_con_io <= (others => 'Z');


  rst_n <= nSCUext_rst_in and nExt_rst_in and nPB_rst_in and nFPHA_rst_in;

  nSYS_rst <= rst_n;
  nPCI_rst_out <= nCB_rst;
  nExt_rst_out <= nCB_rst;

end;
