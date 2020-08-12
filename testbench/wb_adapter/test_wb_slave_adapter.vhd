library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  
use ieee.math_real.all;

use work.wishbone_pkg.all;

entity test_wb_slave_adapter is
    port(
        clk_i      :  in std_logic;
        rst_i      :  in std_logic;
    	sl_adr_i   : in  integer;
		sl_dat_i   : in  integer;
		sl_sel_i   : in  integer;
		sl_cyc_i   : in  std_logic;
		sl_stb_i   : in  std_logic;
		sl_we_i    : in  std_logic
	);
end entity;

architecture simulation of test_wb_slave_adapter is

	signal rst_n : std_logic;

	signal adr : std_logic_vector(31 downto 0);
	signal dat : std_logic_vector(31 downto 0);
	signal sel : std_logic_vector( 3 downto 0);


	signal sl_stall_o, sl_ack_o : std_logic;


	signal mosi : t_wishbone_master_out;
	signal miso : t_wishbone_master_in;

begin
	rst_n <= not rst_i;

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
	    rst_n_i  => rst_n,
		sl_adr_i => adr,
		sl_dat_i => dat,
		sl_sel_i => sel,
		sl_cyc_i => sl_cyc_i,
		sl_stb_i => sl_stb_i,
		sl_we_i  => sl_we_i,

		sl_ack_o => sl_ack_o,
		sl_stall_o => sl_stall_o,

		master_o => mosi,
		master_i => miso
	);

	slave : entity work.slave
	port map (
			clk_i      => clk_i,
			rst_i      => rst_i,

			cyc_i      => mosi.cyc,
			stb_i      => mosi.stb,
			we_i       => mosi.we,
			adr_i      => mosi.adr,
			dat_i      => mosi.dat,
			sel_i      => mosi.sel,

			ack_o      => miso.ack,
			stall_o    => miso.stall,
			dat_o      => miso.dat
		);

    -- psl statements for formal verification
    default clock is rising_edge(clk_i);
    
    assume always sl_cyc_i = '0' -> sl_stb_i = '0';                 -- make sure strobes outside cycles are not present in input 
    assert always rst_i = '0' and mosi.cyc = '0' -> mosi.stb = '0'; -- assert that no strobe is asserted outside of a cycle

    assume always sl_stb_i = '1' -> sl_cyc_i = '1';

    --assert always sl_stb_i = '1' and  sl_cyc_i = '1' -> {sl_stb_i = '0' and  sl_cyc_i = '1'} -> 


    assert always rst_i = '0' and mosi.cyc = '1' and mosi.stb = '1' -> eventually! miso.ack = '1';
    assert always rst_i = '0' and mosi.cyc = '1' and mosi.stb = '1' -> eventually! mosi.cyc = '0';

    --cover {(rst_i = '0' and mosi.cyc = '0'); 
    --       (rst_i = '0' and mosi.cyc = '1')[+1]; 
    --       (rst_i = '0' and mosi.cyc = '1' and mosi.stb = '1'); 
    --       --(rst_i = '0' and mosi.cyc = '1' and mosi.stb = '1')[*4]; 
    --       --(rst_i = '0' and mosi.cyc = '1')[*4]; 
    --       (rst_i = '0' and mosi.cyc = '0')};

    cover{
    	(rst_i = '0' and mosi.cyc = '0');
    	(rst_i = '0' and mosi.cyc = '1' and mosi.stb = '1' and mosi.dat /= x"affe0000");
    	--(rst_i = '0' and mosi.cyc = '1' and miso.ack = '1');
    	(rst_i = '0' and mosi.cyc = '0')
    };

end architecture;