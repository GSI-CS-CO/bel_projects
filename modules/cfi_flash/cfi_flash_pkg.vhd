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


end package;

