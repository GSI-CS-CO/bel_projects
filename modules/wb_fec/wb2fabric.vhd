library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.genram_pkg.all;

entity wb2fabric is
  port(
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    dec_src_i       : in  t_wrf_source_in;
    dec_src_o       : out t_wrf_source_out;
    wb_wbf_slave_i  : in  t_wishbone_slave_in;
    wb_wbf_slave_o  : out t_wishbone_slave_out);
end wb2fabric;

architecture rtl of wb2fabric is
begin

--  bridge : process(clk_i)
--  begin
--    if rising_edge(clk_i) then
--      if rst_n_i = '0' then
--          dec_src_o.cyc <= '0';
--          dec_src_o.stb <= '0';
--          dec_src_o.dat <= (others <= '0'); 
--          wb_wbf_slave_o.stall <= '1';
--          wb_wbf_slave_o.err   <= '0';
--          wb_wbf_slave_o.ack   <= '0';
--      else
--        if (wb_wbf_slave_i.cyc = '1' and wb_wbf_slave_i.stb = '1') then
--          -- wb to fabric
--          dec_src_o.cyc <= wb_wbf_slave_i.cyc;
--          dec_src_o.stb <= wb_wbf_slave_i.stb;
--          dec_src_o.dat <= wb_wbf_slave_i.dat(15 downto 0); 
--          -- fabric to wb
--          wb_wbf_slave_o.stall <= dec_src_i.stall;
--          wb_wbf_slave_o.ack   <= dec_src_i.ack;
--        end if;
--     end if;
--  end process;

  -- wb to fabric
  dec_src_o.cyc <= wb_wbf_slave_i.cyc;
  dec_src_o.stb <= wb_wbf_slave_i.stb;
  dec_src_o.dat <= wb_wbf_slave_i.dat(15 downto 0); 
  -- fabric to wb
  --wb_wbf_slave_o.stall <= dec_src_i.stall;
  wb_wbf_slave_o.stall <= '0';
  --wb_wbf_slave_o.ack   <= dec_src_i.ack;
  wb_wbf_slave_o.ack   <= wb_wbf_slave_i.cyc and wb_wbf_slave_i.stb;
  
  dec_src_o.adr <= c_WRF_DATA;
  dec_src_o.sel <= "11";
  dec_src_o.we  <= '1';


end rtl;
