-- behavioral simulation of the FIFO slave interface
-- of an EZUSB chip. It redirects the signals into 
-- a pseudo terminal and allows real host software 
-- tools to access the simulation via this pseudo terminal
-- the name of the pseudo terminal (eg. /dev/pts/9) is 
-- written to stdout when the simulation starts
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.ez_usb_dev.all; 

entity ez_usb_chip is
	generic (
		g_stop_until_client_connects : boolean := false;
		g_stop_when_idle_for_too_long: integer := 0
		);
	port (
      rstn_i    : in  std_logic; 
      wu2_o     : out std_logic := '0'; -- not really a line of ez-usb-chip but this is needed by etherbone slave to work
      readyn_o  : out std_logic := '0'; 
      fifoadr_i : in  std_logic_vector(1 downto 0); 
      fulln_o   : out std_logic := '1'; 
      emptyn_o  : out std_logic := '0'; 
      sloen_i   : in  std_logic; 
      slrdn_i   : in  std_logic; 
      slwrn_i   : in  std_logic; 
      pktendn_i : in  std_logic; 
      fd_io     : inout std_logic_vector(7 downto 0) := (others => 'Z')
    );
end entity;

architecture simulation of ez_usb_chip is
	signal out_value : std_logic_vector(7 downto 0) := (others => '0');
	signal clk_internal : std_logic := '1';
	signal unlock_stop_mechanism : boolean := false;

	type state_t is (s_init, s_work);
	signal state : state_t := s_init;

	signal slrdn_1   : std_logic;
	signal slwrn_1   : std_logic;
	signal pktendn_1 : std_logic;
	signal fifoadr_1 : std_logic_vector(1 downto 0);
begin

	-- this will shutdown the simulation if usb is idle for too long
	--abort_mechanism: if g_stop_when_idle_for_too_long > 0 generate
	--	clk_internal <= not clk_internal after 10 ns;
	--	process
	--		variable count : integer := 0;
	--	begin
	--		wait until rising_edge(clk_internal);
	--		if unlock_stop_mechanism then
	--			count := count + 1;
	--			--report "count = " & integer'image(count);
	--			if count = g_stop_when_idle_for_too_long then 
	--				assert false report "QUIT" severity failure;
	--			end if;
	--			if sloen_i = '0' or slrdn_i = '0' or slwrn_i = '0' then
	--				count := 0;
	--			end if;
	--		end if;
	--	end process;
	--end generate;

	clk_internal <= not clk_internal after 5 ns;

	fd_io <= out_value when sloen_i = '0' else (others => 'Z');


	fulln_o <= '1'; -- we are never full
	readyn_o <= '0'; -- we are always ready
	dev: process 
		variable value_from_file : integer;
		variable client_connected : boolean;
		variable stop_until_client_connects : boolean := g_stop_until_client_connects;
	begin

		wait until rising_edge(clk_internal);
		slrdn_1   <= slrdn_i;
		slwrn_1   <= slwrn_i;
		fifoadr_1 <= fifoadr_i;
		pktendn_1 <= pktendn_i;

		emptyn_o  <= '0';
		if rstn_i = '1' then
			if client_connected then 
				-- read value from file unless we already have a valid value
				if value_from_file < 0 then
					value_from_file := ez_usb_dev_read(timeout_value => 0);
				end if;
				-- change state based on value
				if value_from_file = HANGUP then 
					wu2_o            <= '0';
					client_connected := false;
				elsif value_from_file >= 0 and fifoadr_i = "00" then -- valid value
					wu2_o    <= '1';
					emptyn_o <= '1'; -- we are no longer empty and have data to send
				end if;
				-- communication with connected hardware
				if slwrn_1 = '1' and slwrn_i = '0' then -- falling edge on slwrn_i
					ez_usb_dev_write(to_integer(unsigned(fd_io)));
				elsif slrdn_1 = '1' and slrdn_i = '0' then -- falling edge on slrdn_i
					out_value <= std_logic_vector(to_signed(value_from_file, 8));
				elsif slrdn_1 = '0' and slrdn_i = '1' then -- falling edge on slrdn_i
					value_from_file := -1;
				end if;
				-- write send written data
				if pktendn_1 = '1' and pktendn_i = '0' then
					ez_usb_dev_flush;
				end if;
			else 
				ez_usb_dev_init(stop_until_client_connects);
				stop_until_client_connects := false;
				client_connected := true;
				unlock_stop_mechanism <= true;
			end if; -- client_connected
		end if; -- rstn_i = '1'

	end process;

end architecture;


