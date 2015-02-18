library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

entity cross_fabric is
   generic (
      g_nodes : integer := 2);
   port (
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      mux_src_o   : out t_wrf_source_out;
      mux_src_i   : in  t_wrf_source_in;
      mux_snk_i   : in  t_wrf_sink_in_array(g_nodes-1 downto 0);
      mux_snk_o   : out t_wrf_sink_out_array(g_nodes-1 downto 0));
end cross_fabric;

architecture rtl of cross_fabric is

begin

--   transfer : process(clk_i) is
--   begin
--      if rst_n_i = '0' then
--         mux_snk_o(0)  <= c_dummy_snk_in;
--         mux_src_o     <= c_dummy_src_in;
--         mux_snk_o(1)  <= c_dummy_snk_in;
--      elsif rising_edge(clk_i) then
         mux_snk_o(0)   <= mux_src_i;
         mux_src_o      <= mux_snk_i(0);         
--      end if;
--   end process;

end rtl;
