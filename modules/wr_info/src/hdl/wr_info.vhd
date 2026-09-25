library ieee;
use ieee.std_logic_1164.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.wr_info_pkg.all;

entity wr_info is
  port(
    clk_i             : in  std_logic;
    rst_n_i           : in  std_logic;
    wr_time_valid     : in  std_logic;
    wr_link_valid     : in  std_logic;
    wr_aux_time_valid : in  std_logic;
    wr_aux_link_valid : in  std_logic;
    slave_i           : in  t_wishbone_slave_in;
    slave_o           : out t_wishbone_slave_out);
end wr_info;

architecture rtl of wr_info is

  signal status_async : std_logic_vector(3 downto 0) := (others => '0');
  signal status_sync  : std_logic_vector(3 downto 0) := (others => '0');
  signal r_ack        : std_logic := '0';
  signal r_dat        : t_wishbone_data := (others => '0');

begin

  status_async <= wr_aux_link_valid & wr_aux_time_valid & wr_link_valid & wr_time_valid;

  sync_status : for i in status_async'range generate
    sync_bit : gc_sync
      port map (
        clk_i     => clk_i,
        rst_n_a_i => rst_n_i,
        d_i       => status_async(i),
        q_o       => status_sync(i));
  end generate;

  slave_o.dat   <= r_dat;
  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.rty   <= '0';

  wb_handler : process(clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      r_ack <= '0';
      r_dat <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');
      if slave_i.cyc = '1' and slave_i.stb = '1' and slave_i.we = '0' then
        case slave_i.adr(3 downto 2) is
          when "00" => -- 0x00: primary WR port
            r_dat(c_wr_info_time_valid_bit) <= status_sync(0);
            r_dat(c_wr_info_link_valid_bit) <= status_sync(1);
          when "01" => -- 0x04: auxiliary WR port
            r_dat(c_wr_info_time_valid_bit) <= status_sync(2);
            r_dat(c_wr_info_link_valid_bit) <= status_sync(3);
          when others => null;
        end case;
      end if;
    end if;
  end process;

end rtl;
