library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity top is
  port (
    clk_base_i        : in    std_logic;
    fx2_clk           : in    std_logic;                      -- Clock from FX2 USB Controller
    scu_cb_revision   : in    std_logic_vector(3 downto 0);   -- must be assigned with weak pull ups
    fpga_con_io       : inout std_logic_vector(7 downto 0);   -- Connection to Arria 10
    --I2C to COMX
    i2c_scl           : in    std_logic;
    i2c_sda           : inout std_logic;
    smb_scl           : in    std_logic;                      --SMB clock
    smb_sda           : inout std_logic :='Z';                --SMB data

    pGood             : in    std_logic_vector (3 downto 0);  -- Power good (0.95V, 1.8V, 3.3V, 5V)
    nPfail            : in    std_logic;                      -- 12V Rail Powerfail
    jtag_present	  	: in		std_logic;							        -- JTAG TMS Signal
    nPB_user_in       : in    std_logic;                      -- User Push-Button In
    wdt               : in    std_logic;                      -- Watchdog COMX
    --Arria 10 status
    CONF_DONE         : in    std_logic;
    INIT_DONE         : in    std_logic;
    nSTATUS           : in    std_logic;

    --Reset In
    nCB_rst           : in    std_logic;                    -- Reset from COMX
    nSCUext_rst_in    : in    std_logic;                    -- Reset form SCU-Bus Extension
    nExt_rst_in       : in    std_logic;                    -- Reset form Extension Connector
    nPB_rst_in        : in    std_logic;                    -- Reset form Push Button
    nFPGA_rst_in      : in    std_logic;                    -- Reset from Arria10
    --Reset Out
    nSYS_rst          : out   std_logic :='0';              -- Reset Out
    nPCI_rst_out      : out   std_logic :='0';              -- PCI Reset Out
    nExt_rst_out      : out   std_logic :='0';              -- Reset to Extension Connector
    --Power
    core_en           : out   std_logic :='0';              -- Enable 0.95V Core voltage
    volt_1_8_en       : out   std_logic :='0';              -- Enable 1.8V Rail
    volt_1_8_IO_en    : out   std_logic :='0';              -- Enable IO 1.8V Rail (MOSFET)
    volt_5_en         : out   std_logic :='0';              -- Enable 5V Rail
    pwr_ok				    : out	  std_logic :='0';				      -- Power Ok COMX
    wake              : out   std_logic :='Z';              -- PCIe Wake 

    IO_enable         : out   std_logic;                    -- Enable Levelshifter 1.8V  ->  3.3V
    nPB_user_out      : out   std_logic;                    -- User Push-Button Out

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


  rst_n <= nSCUext_rst_in and nExt_rst_in and nPB_rst_in and nFPGA_rst_in;

  nSYS_rst <= rst_n;
  nPCI_rst_out <= nCB_rst;
  nExt_rst_out <= nCB_rst;

  IO_enable <= INIT_DONE;
  nPB_user_out <= nPB_user_in;

end;
