-- behavioral simulation of the FIFO slave interface
-- of an EZUSB chip. It redirects the signals into 
-- a pseudo terminal and allows real host software 
-- tools to access the simulation via this pseudo terminal
-- the name of the pseudo terminal (eg. /dev/pts/9) is 
-- written to stdout when the simulation starts
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.file_access.all; 

entity ez_usb_chip is
	generic (
		g_stop_until_client_connects : boolean := true;
		g_stop_when_idle_for_too_long: integer := 100
		);
	port (
      rstn_i    : in  std_logic; 
      ebcyc_o   : out std_logic := '0'; -- not really a line of ez-usb-chip but this is needed by etherbone slave to work
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
begin

	-- this will shutdown the simulation if usb is idle for too long
	abort_mechanism: if g_stop_when_idle_for_too_long > 0 generate
		clk_internal <= not clk_internal after 10 ns;
		process
			variable count : integer := 0;
		begin
			wait until rising_edge(clk_internal);
			if unlock_stop_mechanism then
				count := count + 1;
				--report "count = " & integer'image(count);
				if count = g_stop_when_idle_for_too_long then 
					assert false report "QUIT" severity failure;
				end if;
				if sloen_i = '0' or slrdn_i = '0' or slwrn_i = '0' then
					count := 0;
				end if;
			end if;
		end process;
	end generate;

	fd_io <= out_value when sloen_i = '0' else (others => 'Z');

	main: process 
		variable value_from_file : integer;
		variable client_connected : boolean;
		variable stop_until_client_connects : boolean := true;
	begin
		-- initialization
		wait until rising_edge(rstn_i);
		fulln_o <= '1'; -- we are never full
		readyn_o <= '0'; -- we are ready

		while true loop
			file_access_init(stop_until_client_connects);
			stop_until_client_connects := false;
			client_connected := true;
			unlock_stop_mechanism <= true;

			-- worker loop
			while client_connected loop
	  			-- reading from file and provide it to the master
				value_from_file := file_access_read(timeout_value=>0);
				if value_from_file = HANGUP then -- client disconnected
					ebcyc_o <= '0';
					wait until fifoadr_i = "00"; 
					wait until fifoadr_i = "01"; 
					client_connected := false;
				elsif value_from_file >= 0 then
					ebcyc_o <= '1';
					wait until fifoadr_i = "00";
					while value_from_file >= 0 loop
						emptyn_o <= '1'; -- show the master that there is data
						wait until falling_edge(slrdn_i);
						out_value <= std_logic_vector(to_signed(value_from_file, 8));
						wait until rising_edge(slrdn_i);
						value_from_file := file_access_read(timeout_value=>0);
					end loop;	
					emptyn_o <= '0'; -- show the master that all data was read - we're empty now
				end if;

				-- writing master data to file 
				wait until falling_edge(slwrn_i) or fifoadr_i = "00";
				if slwrn_i = '0' then		
					while pktendn_i = '1' loop
						wait until rising_edge(slwrn_i) or falling_edge(pktendn_i);
						if pktendn_i = '1' then
							file_access_write(to_integer(unsigned(fd_io)));
						end if;
					end loop;
					wait until rising_edge(pktendn_i);
					file_access_flush;
				end if;

			end loop;

		end loop;
	end process;

end architecture;


