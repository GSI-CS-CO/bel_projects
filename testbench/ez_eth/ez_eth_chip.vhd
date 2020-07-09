-- behavioral simulation of the FIFO slave interface
-- of an EZUSB chip. It redirects the signals into 
-- a pseudo terminal and allows real host software 
-- tools to access the simulation via this pseudo terminal
-- the name of the pseudo terminal (eg. /dev/pts/9) is 
-- written to stdout when the simulation starts


type t_wrf_source_out is record
    adr : std_logic_vector(1 downto 0);
    dat : std_logic_vector(15 downto 0);
    cyc : std_logic;
    stb : std_logic;
    we  : std_logic;
    sel : std_logic_vector(1 downto 0);
  end record;

  type t_wrf_source_in is record
    ack   : std_logic;
    stall : std_logic;
    err   : std_logic;
    rty   : std_logic;
  end record;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.file_access.all; 

entity ez_eth_chip is
	port (
      rstn_i    : in  std_logic;
      clk_i     : in  std_logic;
      
      rx_src_o : out t_wrf_source_out:
      rx_src_i : in t_wrf_source_in;
      tx_snk_i : out t_wrf_sink_in;
      tx_snk_o : in t_wrf_sink_out
    );
end entity;

architecture simulation of ez_eth_chip is
	signal out_value : std_logic_vector(7 downto 0) := (others => '0');
begin

	tx_snk_o.ack  <= tx_snk_i.cyc AND tx_snk_i.stb AND tx_snk_i.we after clk_50_period/2;

	tx: process 
		variable writeResult : integer;
	begin
		wait until falling_edge(clk_i);
			tx_snk_o.ack <= '0';
			tx_snk_o.err <= '0';
			if falling_edge(tx_snk_i.cyc) then
				result = flushPacket();
			elsif tx_snk_i.cyc = '1' AND tx_snk_i.stb = '1' AND tx_snk_i.we = '1' AND tx_snk_i.sel = '11' then
				dword = tx_snk_i.dat;
				result = writeWord(dword);
				if result = 0 then
					tx_snk_o.ack <= '1';
				else
					tx_snk_o.err <=	'1';
				end if;
			end if;			

	end process;


	rx: process 
		variable readResult : integer;
	begin
		wait until falling_edge(clk_i);
			tx_snk_o.ack <= '0';
			tx_snk_o.err <= '0';
			if falling_edge(tx_snk_i.cyc) then
				result = flushPacket();
			elsif tx_src_i.cyc = '1' AND tx_snk_i.stb = '1' AND tx_snk_i.we = '1' AND tx_snk_i.sel = '11' then
				dword = tx_snk_i.dat;
				result = writeWord(dword);
				if result = 0 then
					tx_snk_o.ack <= '1';
				else
					tx_snk_o.err <=	'1';
				end if;
			end if;			

	end process;



wait until falling_edge(slrdn_i);
						out_value <= std_logic_vector(to_signed(value_from_file, 8));

	main: process 
		variable value_from_file : integer;
		variable client_connected : boolean;
		variable stop_until_client_connects : boolean := true;
	begin
		-- initialization
		wait until rising_edge(rstn_i);
		fulln_o <= '1'; -- we are never full
		tx_snk_o.stall 	<= '0'; -- no stall. if the buffer is full before packet is complete, we have an error.
		tx_snk_o.rty 	<= '0'; -- retry is not used

		while true loop
			file_access_init(stop_until_client_connects);
			stop_until_client_connects := false;
			client_connected := true;

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


