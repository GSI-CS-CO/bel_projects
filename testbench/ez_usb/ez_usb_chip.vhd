library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.file_access.all; 

entity ez_usb_chip is
	generic (
		PTS_NUMBER : integer
	);
	port (
      rstn_i    : in  std_logic; 
      ebcyc_o   : out std_logic := '0'; 
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
	signal counter   : integer := 0;
	constant max_counter_cycle_termination : integer := 5;
begin

	fd_io <= out_value when sloen_i = '0' else (others => 'Z');

	main: process 
		variable value_from_file : integer;
	begin
		-- initialization
		wait until rising_edge(rstn_i);
		fulln_o <= '1'; -- we are never full
		readyn_o <= '0'; -- we are ready
		file_access_init(PTS_NUMBER);

		-- worker loop
		while true loop
			-- reset the cycle indicator for the master
			counter <= counter + 1;
			if counter >= max_counter_cycle_termination then 
				counter <= 0;
				ebcyc_o <= '0';
				wait until fifoadr_i = "00"; 
				wait until fifoadr_i = "01"; 
			end if;

  			-- reading from file and provide it to the master
			value_from_file := file_access_read(timeout=>0);
			if value_from_file >= 0 then
				ebcyc_o <= '1';
				wait until fifoadr_i = "00";
			end if;
			while value_from_file >= 0 loop
				counter <= 0;
				emptyn_o <= '1'; -- show the master that there is data
				wait until falling_edge(slrdn_i);
				out_value <= std_logic_vector(to_signed(value_from_file, 8));
				wait until rising_edge(slrdn_i);
				value_from_file := file_access_read(timeout=>0);
			end loop;	
			emptyn_o <= '0'; -- show the master that all data was read - we're empty now

			-- writing master data to file 
			wait until falling_edge(slwrn_i) or fifoadr_i = "00";
			if slwrn_i = '0' then		
				counter <= 0;
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
	end process;

end architecture;


