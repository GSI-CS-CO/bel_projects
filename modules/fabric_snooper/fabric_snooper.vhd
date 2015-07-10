library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

entity fabric_snooper is
  port (
      clk_i     : in  std_logic;
      rstn_i    : in  std_logic;
      snk_i     : in  t_wrf_sink_in;
      slave_o   : out t_wishbone_slave_out;
      slave_i   : in  t_wishbone_slave_in
      );
end entity fabric_snooper;

architecture rtl of fabric_snooper is

  signal s_cntr_rx  : unsigned(31 downto 0);
  signal s_stb      : std_logic;
  signal s_toogle   : std_logic;
  signal s_new_rx   : std_logic;
  signal s_rst_cntr : std_logic;

begin

  cntr_rx : process(clk_i) is
  begin
    if rising_edge(clk_i) then
      if rstn_i = '0' then
        s_cntr_rx <= (others => '0');
        s_stb     <= '0';
        s_toogle  <= '0';
        s_new_rx  <= '0';
      else
        
        s_stb <= snk_i.stb;

        if s_stb = '0' and snk_i.stb = '1' and snk_i.cyc = '1' then
          if s_toogle = '1' then 
            s_new_rx  <= '1';
            s_toogle  <= '0';
          else
            s_new_rx  <= '0';
            s_toogle  <= '0';
          end if;
        else
            s_toogle  <= '1';
          s_new_rx    <= '0';
        end if;
        
        if s_rst_cntr = '1' then
          s_cntr_rx <= (others => '0');
        else
          if s_new_rx = '1' then
            s_cntr_rx <= s_cntr_rx + 1;
          else
            s_cntr_rx <= s_cntr_rx;
          end if;
        end if;
      end if;
    end if;
  end process;

  wb_if : process(clk_i) is
  begin
    if rstn_i = '0' then
      slave_o.ack <= '0';
      slave_o.dat <= (others => '0');
      s_rst_cntr  <= '0';
    else
      slave_o.ack <= slave_i.cyc and slave_i.stb;

      if slave_i.cyc = '1' and slave_i.stb = '1' then
        case slave_i.adr(5 downto 2) is 
          when "0000"   =>    -- cntr rx 
            slave_o.dat <= std_logic_vector(s_cntr_rx);
          when "0001"   =>    -- rst cntr rx
            s_rst_cntr  <= '1';
          when others   =>
            s_rst_cntr  <= '0';
        end case;
      else
        s_rst_cntr  <= '0';
      end if;
    end if;
  end process;

end architecture;
