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
      
      rx_src_o : out t_wrf_source_out:
      rx_src_i : in t_wrf_source_in;
      tx_snk_i : out t_wrf_sink_in;
      tx_snk_o : in t_wrf_sink_out
    );
end entity;

architecture simulation of ez_eth_chip is
	signal s_link_up : integer := 0; -- cross process tracking TAP link status
	signal r_acc_err_cnt : integer := 0;	-- cross process tracking of incoming acks/errors at src
	signal r_src_o_cyc : std_logic := '0'; -- to allow readback of src cycle line
begin


	-- standard line behaviour
	tx_snk_o.stall 	<= '0'; -- no stall on TX snk. if the buffer is full before packet is complete, we have an error.
	tx_snk_o.rty 		<= '0'; -- retry is never used
	rx_src_o.we 	  <= '1'; -- sources always write.
	rx_src_o.sel 	  <= "11"; -- always use both bytes. unaligned packets are padded and length field is used.
	rx_src_o.cyc    <= r_src_o_cyc;


	tx: process(rstn_i, clk_i, s_link_up)
		variable result : integer := 0;
	begin
		tx_snk_o.ack <= '0';
		tx_snk_o.err <=	'0';
		wait until rstn_i = '1';

	  while s_link_up = 1 loop
	  	while tx_snk_i.cyc = '1' loop
				wait until falling_edge(clk_i);
				if tx_snk_i.adr = c_WRF_DATA then
					if tx_snk_i.cyc = '1' AND tx_snk_i.stb = '1' AND tx_snk_i.we = '1 then
						result = file_access_write(tx_snk_i.dat);
						--snk ack/err
						wait until rising_edge(clk_i);
						if result = 0 then
							tx_snk_o.ack <= '1';
							tx_snk_o.err <= '0';
						else
							tx_snk_o.ack <= '0';
							tx_snk_o.err <=	'1';
						end if;
					end if;	
				end if;
			end loop; -- cycle high loop			
			file_access_flush();
		end loop;	
	end process;


	-- fetch network packets from kernel in regular intervals so rx buffer cannot overflow.
	process fetch_packets : process(rstn_i, clk_i, s_link_up)
		variable interval : integer := 10;
		variable cnt : integer := 0;
	begin
		cnt := 0;
		while s_link_up = '1'loop
			while cnt < interval loop
				if rstn_i = '1' then
					wait until rising_edge(clk_i);
					file_access_fetch_packet();
					cnt := cnt +1;
				end if;
			end loop;
			cnt := 0;	
		end loop;
	end process;	


	rx_src_acc_counter : process(clk_i)
	begin
		if falling_edge(clk_i) then
			if r_src_o_cyc = '0' or rstn_i = '0' then
				r_acc_err_cnt <= 0;
			elsif rx_src_i.ack = '1' or rx_src_i.err = '1'then
				r_acc_err_cnt <= r_acc_err_cnt + 1;
			end if;
		end if;
	end process;		 	


	init_and_rx: process(clk_i, rstn_i, rx_src_i.stall)
		variable value_from_file : integer := 0;
		variable stop_until_1st_packet : boolean := true;
		variable stb_cnt : integer := 0;
	begin
		

			while true loop --> main rx loop
				-- reset and init code
				r_src_o_cyc <= '0';
				wait until rstn_i = '1';
			  if s_link_up = 0 then
			  	s_link_up <= file_access_init(stop_until_1st_packet)
			  else
					--rx code
					if file_access_pending() = 1 then
						-- start packet cycle	
						value_from_file   := 0;
						stb_cnt := 0;
						acc_cnt := 0;
						wait until rising_edge(clk_i);
						--start cycle and do the wrf status hocus pocus
						r_src_o_cyc <= '1';
						rx_src_o.stb <= '1';
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
						wait until falling_edge(clk_i);
						wait until rx_src_i.stall = '0'; 	
						stb_cnt := stb_cnt + 1;	--wait until there's no stall before inc stb count

	
						--pass on data in this packet
						while value_from_file >= 0 loop --> stb loop
							value_from_file := file_access_read(); 
							if value_from_file >= 0 then 
								wait until rising_edge(clk_i);
									rx_src_o.adr <= c_WRF_DATA;
									rx_src_o.stb <= '1'
									rx_src_o.dat <= std_logic_vector(to_unsigned(value_from_file, 16));
									wait until falling_edge(clk_i); -- ensure minimum wait of half period for stall
									wait until rx_src_i.stall = '0'; 	
									stb_cnt := stb_cnt + 1;	--wait until there's no stall before inc stb count
							end if;	
						end loop;	--< stb loop	
						wait until rising_edge(clk_i); 
						rx_src_o.stb <= '0';
						-- we're done with this packet. deassert stb and wait for all acks	
						wait until r_acc_err_cnt = stb_cnt;
						wait until rising_edge(clk_i);	
						r_src_o_cyc <= '0';
						-- end packet cycle	
					end if;

				end if;	
			end loop;	--< main rx loop	
	end process;

end architecture;


