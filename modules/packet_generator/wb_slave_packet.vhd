--! Register Map ************************
--! 0x00, wr, enable/disable generator
--! 0x04, wr, set payload length(high four bits) eg.0x01f4****  Range:(46-1500byte)
--! 0x08, wr, destination address hb, ether type
--! 0x0C, wr, destination address lb
--! 0x10, wr, generated frames
--! 0x14, wr, set about packet generator rate
--! 0x18, wr, choose packet mode 0x0:continuous  0x1:discrete 
--! 0x1c,  r, counter valuee 
--! 0x20, wr, set protocol (default UDP) 

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.packet_pkg.all;

entity wb_slave_packet is
   port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      pg_stat_reg_i  : in  t_pg_stat_reg;
      pg_ctrl_reg_o  : out t_pg_ctrl_reg);
end wb_slave_packet;

architecture rtl of wb_slave_packet is

  signal s_pg_stat  : t_pg_stat_reg   := c_pg_stat_default;
  signal s_pg_ctrl  : t_pg_ctrl_reg   := c_pg_ctrl_default ;

begin

   -- this wb slave doesn't supoort them
  wb_slave_o.int <= '0';
  wb_slave_o.rty <= '0';
  wb_slave_o.err <= '0';

  wb_process : process(clk_i)
    begin

      if rising_edge(clk_i) then
        if rst_n_i = '0' then
          s_pg_stat   <= c_pg_stat_default;
          s_pg_ctrl   <= c_pg_ctrl_default;
          wb_slave_o.ack    <= '0';
          wb_slave_o.dat    <= (others => '0');
       else
         wb_slave_o.ack     <= wb_slave_i.cyc and wb_slave_i.stb;

         if wb_slave_i.cyc = '1' and wb_slave_i.stb = '1' then
           case wb_slave_i.adr(5 downto 2) is
             when "0000"   => -- enable/disable packet generator 0x0
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.en_pg        			<= wb_slave_i.dat(0);
               end if;
               wb_slave_o.dat(0) 			        <= s_pg_ctrl.en_pg;
               wb_slave_o.dat(31 downto 1) 		<= (others => '0');
             when "0001"   => -- ctrl about packet generator, payload 0x4
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.payload            <= wb_slave_i.dat(31 downto 16);
               end if;                  
               wb_slave_o.dat(31 downto 16)	  <= s_pg_ctrl.payload;
             when "0010"   => -- ctrl about packet generator mac_add hb/ethertype 0x8
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.eth_hdr.eth_des_addr(47 downto 32) <= wb_slave_i.dat(31 downto 16);
                 s_pg_ctrl.eth_hdr.eth_etherType    <= wb_slave_i.dat(15 downto 0);
               end if;                  
                 wb_slave_o.dat(31 downto 16) <= s_pg_ctrl.eth_hdr.eth_des_addr(47 downto 32);
                 wb_slave_o.dat(15 downto 0)	<= s_pg_ctrl.eth_hdr.eth_etherType;
             when "0011"   => -- ctrl about packet generator mac_add lb 0xC
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.eth_hdr.eth_des_addr(31 downto 0) <= wb_slave_i.dat(31 downto 0);
               end if;                  
               wb_slave_o.dat(31 downto 0) 	  <= s_pg_ctrl.eth_hdr.eth_des_addr(31 downto 0);
             when "0100"   => -- ctrl about packet generator 0x10
               if wb_slave_i.we = '1' then
                 s_pg_stat.frame_gen  		    <= wb_slave_i.dat;
               end if;
                 wb_slave_o.dat 			        <= s_pg_stat.frame_gen;
             when "0101"   => -- ctrl about packet generator rate 0x14
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.rate               <= wb_slave_i.dat(31 downto 0);
               end if;
                 wb_slave_o.dat(31 downto 0)  <= s_pg_ctrl.rate;
             when "0110"   => -- choose packet generator mode discrete/continuous 0x18
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.mode 			        <= wb_slave_i.dat(1 downto 0);
               end if;
               wb_slave_o.dat(1 downto 0) 	  <= s_pg_ctrl.mode;
               wb_slave_o.dat(31 downto 2) 		<= (others => '0');      
             when "0111"   => -- read counter 0x1c
               if wb_slave_i.we = '0' then
                 wb_slave_o.dat(31 downto 0)  <= s_pg_stat.count;
               end if;
             when "1000"   => -- set udp 0x20
               if wb_slave_i.we = '1' then
                 s_pg_ctrl.udp 			          <= wb_slave_i.dat(7 downto 0);
               end if;
               wb_slave_o.dat(7 downto 0) 		<= s_pg_ctrl.udp;
               wb_slave_o.dat(31 downto 8) 		<= (others => '0');
             when others =>
           end case;
         end if;      
       
         s_pg_stat      <= pg_stat_reg_i;
         pg_ctrl_reg_o  <= s_pg_ctrl;
       end if;
     end if;
   end process;

end rtl;
