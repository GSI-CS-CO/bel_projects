--! Register Map
-- Ctrl Enc/Dec
--! 0x00, wr, enable/disable encoder 
--! 0x04, wr, enable/disable decoder
-- Stat Enc/Dec
--! 0x08, wr, number of frames encoded
--! 0x0C, wr, number of frames decoded
-- Ctrl Packet Generator
--! 0x10, wr, enable/disable generator
--! 0x14, wr, payload/
--! 0x18  wr, rate
--! 0x1C, wr, destination address hb, ethertype
--! 0x20, wr, destination address lb
--! 0x24, wr, generated frames
-- Decoder Latency Statitcis 
--! 0x28, wr, decoder last latency
--! 0x2C, wr, decoder max latency
--! 0x30, wr, decoder min latency
--! 0x34, wr, decoder latency accumulated
--! 0x38, wr, decoder number of latency measured
-- Ctrl Reset Stat Registers

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.fec_pkg.all;

entity wb_slave_fec is
   port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      fec_stat_reg_i : in  t_fec_stat_reg;
      fec_ctrl_reg_o : out t_fec_ctrl_reg;
      time_code_i    : in  t_time_code;
      pg_stat_reg_i  : in  t_pg_stat_reg;
      pg_ctrl_reg_o  : out t_pg_ctrl_reg);
end wb_slave_fec;

architecture rtl of wb_slave_fec is

   signal s_fec_stat : t_fec_stat_reg  := c_stat_reg_default;
   signal s_fec_ctrl : t_fec_ctrl_reg  := c_ctrl_reg_default;
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
         s_fec_stat  <= c_stat_reg_default;
         s_fec_ctrl  <= c_ctrl_reg_default;
         s_pg_stat   <= c_pg_stat_default;
         s_pg_ctrl   <= c_pg_ctrl_default;

         wb_slave_o.ack    <= '0';
         wb_slave_o.dat    <= (others => '0');

      else
         wb_slave_o.ack <= wb_slave_i.cyc and wb_slave_i.stb;

         if wb_slave_i.cyc = '1' and wb_slave_i.stb = '1' then
            case wb_slave_i.adr(5 downto 2) is
               when "0000"    =>  -- enable/disable encoder 0x0
                  if wb_slave_i.we = '1' then
                     s_fec_ctrl.en_enc <= wb_slave_i.dat(0);
                  end if;
                  wb_slave_o.dat(0) <= s_fec_ctrl.en_enc;
                  wb_slave_o.dat(31 downto 1) <= (others => '0');
               when "0001"    => -- enable/disable decoder 0x4
                  if wb_slave_i.we = '1' then
                     s_fec_ctrl.en_dec <= wb_slave_i.dat(0);
                  end if;
                  wb_slave_o.dat(0) <= s_fec_ctrl.en_dec;
                  wb_slave_o.dat(31 downto 1) <= (others => '0');
               when "0010"    => -- encoded frames 0x8
                  if wb_slave_i.we = '1' then
                     s_fec_stat.stat_enc.frame_enc <= wb_slave_i.dat; -- it'd be set to 0
                  end if;
                  wb_slave_o.dat <= s_fec_stat.stat_enc.frame_enc;
               when "0011"    => -- decoded frames 0xC
                  if wb_slave_i.we = '1' then
                     s_fec_stat.stat_dec.err_dec <= wb_slave_i.dat; -- it'd be set to 0
                  end if;
                  wb_slave_o.dat <= s_fec_stat.stat_dec.err_dec;
               when "0100"   => -- enable/disable packet generator 0x10
                  if wb_slave_i.we = '1' then
                     s_pg_ctrl.en_pg <= wb_slave_i.dat(0);
                  end if;
                  wb_slave_o.dat(0) <= s_pg_ctrl.en_pg;
                  wb_slave_o.dat(31 downto 1) <= (others => '0');
               when "0101"   => -- ctrl about packet generator, payload 0x14
                  if wb_slave_i.we = '1' then
                     s_pg_ctrl.payload <= wb_slave_i.dat(15 downto 0);
                  end if;
                  wb_slave_o.dat(15 downto 0) <= s_pg_ctrl.payload;
               when "0110"   => -- ctrl about packet generator, rate 0x18
                  if wb_slave_i.we = '1' then
                     s_pg_ctrl.rate <= wb_slave_i.dat;
                  end if;
                  wb_slave_o.dat <= s_pg_ctrl.rate;
               when "0111"   => -- ctrl about packet generator, mac add hb ethertype 0x1C
                  if wb_slave_i.we = '1' then
                     s_pg_ctrl.eth_hdr.eth_des_addr(47 downto 32) <= wb_slave_i.dat(31 downto 16);
                     s_pg_ctrl.eth_hdr.eth_etherType              <= wb_slave_i.dat(15 downto 0);
                  end if;                  
                  wb_slave_o.dat(31 downto 16) <= s_pg_ctrl.eth_hdr.eth_des_addr(47 downto 32);
                  wb_slave_o.dat(15 downto 0)  <= s_pg_ctrl.eth_hdr.eth_etherType;
               when "1000"   => -- ctrl about packet generator, mac addr lb  0x20
                  if wb_slave_i.we = '1' then
                     s_pg_ctrl.eth_hdr.eth_des_addr(31 downto 0) <= wb_slave_i.dat(31 downto 0);
                  end if;                  
                  wb_slave_o.dat(31 downto 0) <= s_pg_ctrl.eth_hdr.eth_des_addr(31 downto 0);
               when "1001"   => -- ctrl about packet generator, generated frames 0x24
                  if wb_slave_i.we = '1' then
                     s_pg_stat.frame_gen  <= wb_slave_i.dat;
                  end if;
                  wb_slave_o.dat <= s_pg_stat.frame_gen;
               when "1010"   =>  -- decoder, last latency measured 0x28
                  wb_slave_o.dat <= s_fec_stat.stat_enc.latency_enc.fec_lat;
               when "1011"   =>  -- decoder, max latency measured  0x2c
                  wb_slave_o.dat <= s_fec_stat.stat_enc.latency_enc.fec_lat_max;
               when "1100"   =>  -- decoder, min latency measured  0x30
                  wb_slave_o.dat <= s_fec_stat.stat_enc.latency_enc.fec_lat_min;
               when "1101"   =>  -- decoder, latency accumulated   0x34
                  wb_slave_o.dat <= s_fec_stat.stat_enc.latency_enc.fec_lat_acc;
               when "1110"   =>  -- decoder, number of samples     0x38
                  wb_slave_o.dat <= s_fec_stat.stat_enc.latency_enc.fec_num_lat;
               when others =>
            end case;
         end if;

	       s_fec_ctrl.time_code <= time_code_i;
         fec_ctrl_reg_o <= s_fec_ctrl;
         s_fec_stat     <= fec_stat_reg_i;

	       s_pg_ctrl.time_code  <= time_code_i;
         pg_ctrl_reg_o  <= s_pg_ctrl;
         s_pg_stat      <= pg_stat_reg_i;
      end if;
   end if;
   end process;

end rtl;
