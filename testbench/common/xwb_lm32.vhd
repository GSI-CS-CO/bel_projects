library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;
use work.interface_lm32.all;

entity xwb_lm32 is
generic(
	g_profile      : string := "dontcare";
	g_reset_vector : std_logic_vector(31 downto 0) := x"00000000";
	g_sdb_address  : std_logic_vector(31 downto 0) := x"00000000"
);
port(
	clk_sys_i : in  std_logic;
	rst_n_i   : in  std_logic;
	irq_i     : in  std_logic_vector(31 downto 0);
	dwb_o     : out t_wishbone_master_out;
	dwb_i     : in  t_wishbone_master_in;
	iwb_o     : out t_wishbone_master_out;
	iwb_i     : in  t_wishbone_master_in
);
end entity;

architecture verilator_interface of xwb_lm32 is
  signal lm32_idx         : integer   := interface_lm32_init;
  signal rst              : std_logic := '0';
begin
  
  rst <= not rst_n_i;

  main: process
  begin
    wait until rising_edge(clk_sys_i);
    interface_lm32_reset(lm32_idx, rst);
    interface_lm32_clock(lm32_idx, clk_sys_i);

    -- get the output from verilator simulation
    iwb_o <= interface_lm32_i_wb_mosi(lm32_idx); -- instruction bus
    dwb_o <= interface_lm32_d_wb_mosi(lm32_idx); -- data bus

    wait until falling_edge(clk_sys_i);
    interface_lm32_i_wb_miso(lm32_idx, iwb_i); -- instruction bus
    interface_lm32_d_wb_miso(lm32_idx, dwb_i); -- data bus

    interface_lm32_interrupt(lm32_idx, irq_i); -- interrupts

    interface_lm32_clock(lm32_idx, clk_sys_i);
  end process;
	
end architecture;
