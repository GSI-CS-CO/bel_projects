library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.temp_sensor_pkg.all;

entity wbs_temp_sense is
	generic (
      		g_address_size  : natural := 32;  -- in bit(s)
      		g_data_size     : natural := 32;  -- in bit(s)
      		g_spi_data_size : natural := 8    -- in bit(s)
   		 );
    	port (
      	-- generic system interface
      		clk_sys_i  : in  std_logic;
      		rst_n_i    : in  std_logic;
      	-- wishbone slave interface
      		slave_i    : in  t_wishbone_slave_in;
      		slave_o    : out t_wishbone_slave_out;
		clr_out	   : out std_logic
	     );
end wbs_temp_sense;

architecture rtl of wbs_temp_sense is

--wishbone signals
	signal s_wb_cyc		:std_logic ;
	signal s_wb_stb		:std_logic ;
	signal s_wb_we		:std_logic ;
	signal s_wb_adr		:std_logic_vector(31 downto 0);
	signal s_wb_sel		:std_logic_vector(3 downto 0);
	signal s_wb_ack 	:std_logic := '0';
	signal s_wb_stall	:std_logic := '0';
	signal s_wb_dat		:std_logic_vector(31 downto 0) :=x"00000000";
	signal s_wb_dat_i       :std_logic_vector(31 downto 0) :=x"00000000";

--temperature sensor signals
	signal s_ce		:std_logic := '1';
	signal s_clr		:std_logic := '0';
	signal s_tsdcaldone	:std_logic := '0';
	signal s_tsdcalo	:std_logic_vector(7 downto 0);

--internal signals
	signal s_clr_out	:std_logic;
	signal s_count		:integer range 0 to 200 := 0;
--constants
	constant c_address_tx_data	:    std_logic_vector (1 downto 0):= "00";

begin
	s_wb_cyc	<= slave_i.cyc;
	s_wb_stb	<= slave_i.stb;
	s_wb_we		<= slave_i.we;
	s_wb_adr	<= slave_i.adr;
	s_wb_dat_i	<= slave_i.dat;
	s_wb_sel	<= slave_i.sel;
	slave_o.ack	<= s_wb_ack;
	slave_o.stall	<= s_wb_stall;
	slave_o.dat	<= s_wb_dat;
	slave_o.err	<= '0';
	slave_o.int	<= '0';
	slave_o.rty	<= '0';

	clr_out		<= s_clr_out;

p_clk_process1: process(clk_sys_i)

begin
        if rising_edge (clk_sys_i) then

                if (s_tsdcaldone='1') then
                        s_clr_out <= '1';
                        s_count   <= 0;
                else
                        s_count   <= s_count + 1;
                        s_clr_out <= '0';
                end if;
        end if;
end process;


temperature_sensor : temp_sens

port map (
	ce		=> s_ce,
	clk		=> clk_sys_i,
	clr		=> s_clr_out,
	tsdcaldone	=> s_tsdcaldone,
	tsdcalo		=> s_tsdcalo
	);

p_clk_process2: process (clk_sys_i)

begin

if rising_edge(clk_sys_i) then

	if (rst_n_i='0') then

		s_wb_ack	<= '0';
	--	s_wb_stall	<= '1';

	else

		s_wb_ack        <= s_wb_cyc and s_wb_stb;
          --      s_wb_stall      <= '0';

	end if;

	--if (s_wb_cyc='1' and s_wb_stb='1'and s_wb_sel="1111") then
	if (s_wb_cyc='1' and s_wb_stb='1') then

		case s_wb_adr(3 downto 2) is
			
			when c_address_tx_data =>
				if (s_wb_we='0') then
	
					s_wb_dat(7 downto 0) 	<= s_tsdcalo;
					s_wb_dat(31 downto 8)	<= x"000000";
		
				else
	
--					s_wb_dat <= s_wb_dat_i;
					s_wb_dat <= x"DEADC0DE";
	
				end if;
	
			when others =>
	
					s_wb_dat <= x"DEADC0DE";
		end case;		
				
	end if;

end if;
end process;
end rtl;
