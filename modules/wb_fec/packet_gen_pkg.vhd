library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_fabric_pkg.all;
use work.fec_pkg.all;

package packet_gen_pkg is 
   type t_pg_ctrl_reg is
   record
      en_pg       : std_logic;
      frame_size  : integer;
      rate        : integer;
      eth_hdr     : t_eth_frame_header;
   end record;

   type t_pg_stat_reg is
   record
      frame_gen   :  integer;
   end record;
   
   type t_pg_state is
   record
      gen_packet   : std_logic;
      cyc_ended    : std_logic;
      halt         : std_logic;
   end record;
   
   constant c_pg_stat_default    : t_pg_stat_reg   := (
      frame_gen   => 0);
   
   constant c_pg_ctrl_default    : t_pg_ctrl_reg   := (
      en_pg       => '0',
      frame_size  => 512,
      rate        => 1000,
      eth_hdr     => c_eth_frame_header_default);
   
   constant c_pg_state_default   : t_pg_state      := (
      gen_packet  => '0',
      cyc_ended   => '0',
      halt        => '0');

   component packet_gen is
      port (
         clk_i       : in  std_logic;
         rst_i       : in  std_logic;
         ctrl_reg_i  : in  t_pg_ctrl_reg;
         stat_reg_o  : out t_pg_stat_reg;
         pg_src_i    : in  t_wrf_sink_in;
         pg_src_o    : out t_wrf_sink_out);
   end component;

   function f_eth_hdr( eth_hdr : t_eth_frame_header)
         return t_eth_hdr;

end package packet_gen_pkg;


package body packet_gen_pkg is

   -- Functions
   function f_eth_hdr( eth_hdr : t_eth_frame_header)
         return t_eth_hdr is

         variable hdr  : t_eth_hdr := (others => '0');
   begin
      hdr := eth_hdr.eth_src_addr &
             eth_hdr.eth_des_addr &
             eth_hdr.eth_etherType;
      return hdr;

   end f_eth_hdr;

end packet_gen_pkg; 
