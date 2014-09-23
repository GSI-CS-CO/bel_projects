library ieee;
use ieee.std_logic_1164.all;

library work;
use work.wishbone_pkg.all;

package cfi_flash_pkg is

component cfi_ctrl
	port (
    wb_clk_i        : in std_logic;
    wb_rst_i        : in std_logic;
 
    wb_dat_i        : in std_logic_vector(31 downto 0);
    wb_adr_i        : in std_logic_vector(31 downto 0);
    wb_stb_i        : in std_logic;
    wb_cyc_i        : in std_logic;
    wb_we_i         : in std_logic;
    wb_sel_i        : in std_logic_vector(3 downto 0);
    wb_dat_o        : out std_logic_vector(31 downto 0);
    wb_ack_o        : out std_logic;
    wb_err_o        : out std_logic;
    wb_rty_o        : out std_logic;
    wb_stall_o      : out std_logic;
  
  
    flash_dq_io     : inout std_logic_vector(15 downto 0);
    flash_adr_o     : out std_logic_vector(23 downto 0);
    flash_adv_n_o   : out std_logic;
    flash_ce_n_o    : out std_logic;
    flash_clk_o     : out std_logic;
    flash_oe_n_o    : out std_logic;
    flash_rst_n_o   : out std_logic;
    flash_wait_i    : in std_logic;
    flash_we_n_o    : out std_logic;
    flash_wp_n_o    : out std_logic

	);
end component;

component XWB_CFI_WRAPPER

  port(
    clk_i      : in    std_logic;
    rst_n_i    : in    std_logic;

    -- Wishbone
    slave_i    : in    t_wishbone_slave_in;    -- to Slave
    slave_o    : out   t_wishbone_slave_out;   -- to WB

  -- External Parallel Flash Pins
    AD         : out   std_logic_vector(25 downto 1);
    DF         : inout std_logic_vector(15 downto 0);
    ADV_FSH    : out   std_logic;
    nCE_FSH    : out   std_logic;
    CLK_FSH    : out   std_logic;
    nWE_FSH    : out   std_logic;
    nOE_FSH    : out   std_logic;
    nRST_FSH   : out   std_logic;
    WAIT_FSH   : in    std_logic

   );
end component;--XWB_CFI_WRAPPER

constant c_wb_CfiPFlash_sdb : t_sdb_device := (        -- defined in wishbone_pkg
    abi_class        => x"0000",                       -- undocumented device
    abi_ver_major    => x"01",
    abi_ver_minor    => x"00",
    wbd_endian       => c_sdb_endian_big,
    wbd_width        => x"7",                          -- x2=16bit port granularity (x1=8bit,x4=32bit, x7=8/16/32bit)  
    sdb_component => (
    addr_first       => x"0000000000000000",           -- this is relativ, absolute will be calculated
    addr_last        => x"000000000FFFFFFF",           -- Address space 2exp 25 bytes = 2 exp 24  Words
    product => (
    vendor_id        => x"0000000000000651",           -- ID for GSI
    device_id        => x"12122121",                   -- ID can be chosen, but no collision with existing ones
                                                       -- Same ID to be used with find_device in C-Code for mini.sdb
    version          => x"00000001",
    date             => x"20140910",
    name             => "WR_CfiPFlash       ")));

end package;

