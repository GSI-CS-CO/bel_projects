-- behavioral simulation of a TAP interface (virtual ethernet device)
-- to WR Fabric interface bridge. Needs a persistent interface called tap2
-- if not existent, create as sudoer with
--
-- cli here
--




library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wr_fabric_pkg.all;
use work.file_access.all; 

entity ez_eth_chip is
	port (
      rstn_i    : in  std_logic;
      clk_i     : in  std_logic;
      
      rx_src_o : out t_wrf_source_out;
      rx_src_i : in t_wrf_source_in;
      tx_snk_i : in t_wrf_sink_in;
      tx_snk_o : out t_wrf_sink_out
    );
end entity;

architecture simulation of ez_eth_chip is
	signal s_link_up : integer := 0; -- cross process tracking TAP link status
	signal r_acc_err_cnt : integer := 0;	-- cross process tracking of incoming acks/errors at src
	signal r_stb_cnt : integer := 0;	-- cross process tracking of valid strobessrc
	signal r_src_o_cyc : std_logic := '0'; -- to allow readback of src cycle line
	signal r_src_o_stb : std_logic := '0'; -- to allow readback of src stb line
begin


	-- standard line behaviour
	tx_snk_o.stall 	<= '0'; -- no stall on TX snk. if the buffer is full before packet is complete, we have an error.
	tx_snk_o.rty 		<= '0'; -- retry is never used
	rx_src_o.we 	  <= '1'; -- sources always write.
	rx_src_o.sel 	  <= "11"; -- always use both bytes. unaligned packets are padded and length field is used.
	rx_src_o.cyc    <= r_src_o_cyc;
	rx_src_o.stb    <= r_src_o_stb;



	tx: process
		variable result : integer := 0;
		variable ack : std_logic := '0';
		variable err : std_logic := '0';
	begin
		tx_snk_o.ack <= '0';
		tx_snk_o.err <=	'0';
		wait until rstn_i = '1';
		report "TX: coming out of reset" severity warning;
	  while true loop
	  	wait until falling_edge(clk_i);
	  	if tx_snk_i.cyc = '1' and s_link_up = 1  then
	  		--report "TX: cycle started";
	  		while tx_snk_i.cyc = '1' loop

	  			ack := '0';
	  			err := '0';
					if tx_snk_i.cyc = '1' AND tx_snk_i.stb = '1' AND tx_snk_i.we = '1' then
					  if tx_snk_i.adr = c_WRF_DATA then
						  --report "TX: writing to SW";
							result := file_access_write(to_integer(unsigned(tx_snk_i.dat)));
							--snk ack/err
							
							if result = 1 then
								ack := '1';
							else
								err := '0';
							end if;
						else
							ack := '1';	
						end if;	
					end if;
					wait until rising_edge(clk_i);
					tx_snk_o.ack <= ack;
					tx_snk_o.err <= err;
					wait until falling_edge(clk_i);
				end loop; -- cycle high loop
				tx_snk_o.ack <= '0';
				tx_snk_o.err <=	'0';
				wait until rising_edge(clk_i);
				--report "TX: Packet written to buffer. Flushing";
				file_access_flush;
			end if;
		end loop;		
	end process;


	-- fetch network packets from kernel in regular intervals so rx buffer cannot overflow.
	fetch_packets : process
		variable interval : integer := 10;
		variable cnt : integer := 0;
		variable result : integer := 0;
	begin
		cnt := 0;
		wait until rising_edge(clk_i);
		while s_link_up = 1 and rstn_i = '1' loop
			wait until rising_edge(clk_i);
			while cnt < interval loop
					result := file_access_fetch_packet;
					cnt := cnt +1;
			end loop;
--			report "Polling packet fetch";
			cnt := 0;	
		end loop;
		cnt := 0;	
	end process;	


	rx_src_accerr_stb_counter : process(clk_i)
	begin
		if rising_edge(clk_i) then
			if r_src_o_cyc = '0' or rstn_i = '0' then
				r_acc_err_cnt <= 0;
				r_stb_cnt  <= 0;
			else
				if rx_src_i.ack = '1' or rx_src_i.err = '1' then
					r_acc_err_cnt <= r_acc_err_cnt + 1;
				end if;
				if r_src_o_stb = '1' and rx_src_i.stall = '0' then
					r_stb_cnt  <= r_stb_cnt  + 1;
				end if;
			end if;		
		end if;
	end process;		 	


	init_and_rx: process
		variable value_from_file : integer := 0;
		variable stop_until_1st_packet : boolean := true;
		variable stb_cnt : integer := 0;
	begin
		

				s_link_up <= 0;
				rx_src_o.adr <= c_WRF_DATA;
				r_src_o_cyc  <= '0';
				r_src_o_stb <= '0';
				rx_src_o.dat <= (others => '0');
				wait until rstn_i = '1';
				report "RX: Coming out of reset" severity warning;
			while true loop --> main rx loop
				-- reset and init code
				r_src_o_cyc <= '0';
				wait until falling_edge(clk_i);
			  if s_link_up = 0 then
			  	report "RX: Initialising network interface...";
			  	file_access_init(stop_until_1st_packet);
			  	wait until rising_edge(clk_i);
			  	s_link_up <= 1;
			  	wait until rising_edge(clk_i);
			  	report "RX: Init done";
			  else
					--rx code
					wait until rising_edge(clk_i);
					--report "RX:  polling";
					if file_access_pending >= 0 then
						--report "RX:  got a packet. Processing...";
						-- start packet cycle	
						value_from_file   := 0;
						--start cycle and do the wrf status hocus pocus
						
						r_src_o_cyc <= '1';
						r_src_o_stb <= '1';
						rx_src_o.adr <= c_WRF_STATUS;

--TODO: x"8004" as WRF_STATUS could be wrong, depending if incoming packet has a CRC!
-- see wr_fabric_pkg
--  type t_wrf_status_reg is record
--    is_hp       : std_logic;
--    has_smac    : std_logic;
--    has_crc     : std_logic;
--    error       : std_logic;
--    tag_me      : std_logic;
--    match_class : std_logic_vector(7 downto 0);
--  end record;

						rx_src_o.dat <= x"8004"; 
						wait until rising_edge(clk_i);
						--report "RX:  Set WRF status";
						
									--pass on data in this packet
						
						while value_from_file >= 0 loop --> stb loop
							value_from_file := file_access_read; 
							--report "RX:  Sread value";
							
							if value_from_file >= 0 then
									while rx_src_i.stall = '1' loop
										--report "RX:  waiting to be unstalled";
										wait until rising_edge(clk_i);
									end loop; 
									rx_src_o.adr <= c_WRF_DATA;
									r_src_o_stb <= '1';
									rx_src_o.dat <= std_logic_vector(to_unsigned(value_from_file, 16));
							else
								r_src_o_stb <= '0';
							end if;
							wait until rising_edge(clk_i);
						end loop;	--< stb loop	
						wait until rising_edge(clk_i); 
						r_src_o_stb <= '0';
						-- we're done with this packet. deassert stb and wait for all acks
						--report "RX:  Done. Waiting for ACKERR/STB counters";		
						while r_acc_err_cnt /= r_stb_cnt loop
							wait until rising_edge(clk_i);
						end loop;
						--report "RX: Packet complete.";		
						r_src_o_cyc <= '0';
						wait until rising_edge(clk_i);
						-- end packet cycle	
					end if;

				end if;	
			end loop;	--< main rx loop	
	end process;

end architecture;


