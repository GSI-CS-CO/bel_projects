library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  
use ieee.math_real.all;

use work.wishbone_pkg.all;

entity test_wb_slave_adapter is
    port(
        clk_i      :  in std_logic;
        rst_n_i    :  in std_logic;
    	sl_adr_i   : in  integer;
		sl_dat_i   : in  integer;
		sl_sel_i   : in  integer;
		sl_cyc_i   : in  std_logic;
		sl_stb_i   : in  std_logic;
		sl_we_i    : in  std_logic
	);
end entity;

architecture simulation of test_wb_slave_adapter is

	signal adr : std_logic_vector(31 downto 0);
	signal dat : std_logic_vector(31 downto 0);
	signal sel : std_logic_vector( 3 downto 0);

begin

	adr <= std_logic_vector(to_signed(sl_adr_i, 32));
	dat <= std_logic_vector(to_signed(sl_dat_i, 32));
	sel <= std_logic_vector(to_signed(sl_sel_i, 4));


	adapter : entity work.wb_slave_adapter
	generic map (
		g_slave_use_struct   => false,
		g_slave_mode         => PIPELINED,
		g_slave_granularity  => WORD,
		g_master_use_struct  => true,
		g_master_mode        => PIPELINED,
		g_master_granularity => WORD
	)
	port map (
	    clk_sys_i => clk_i,
	    rst_n_i  => rst_n_i,
		sl_adr_i => adr,
		sl_dat_i => dat,
		sl_sel_i => sel,
		sl_cyc_i => sl_cyc_i,
		sl_stb_i => sl_stb_i,
		sl_we_i  => sl_we_i
	);

end architecture;