library ieee;
use ieee.std_logic_1164.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.wr_info_pkg.all;
use work.eca_internals_pkg.all;

entity wr_info is
  generic(
    g_dual_port_wr    : boolean := false);
  port(
    clk_i             : in  std_logic;
    rst_n_i           : in  std_logic;
    wr_time_valid     : in  std_logic;
    wr_link_valid     : in  std_logic;
    wr_aux_time_valid : in  std_logic;
    wr_aux_link_valid : in  std_logic;
    clk_ref_aux_i     : in  std_logic := '0'; -- clock of the auxiliary WR port, tai and cycles are in this domain
    wr_aux_tai_i      : in  std_logic_vector(39 downto 0) := (others => '0');
    wr_aux_cycles_i   : in  std_logic_vector(27 downto 0) := (others => '0');
    slave_i           : in  t_wishbone_slave_in;
    slave_o           : out t_wishbone_slave_out);
end wr_info;

architecture rtl of wr_info is
  -- as in eca.vhd: keep the synchronizer registers
  attribute altera_attribute : string;
  attribute altera_attribute of rtl : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF";

  signal status_async : std_logic_vector(3 downto 0) := (others => '0');
  signal status_sync  : std_logic_vector(3 downto 0) := (others => '0');
  signal r_ack        : std_logic := '0';
  signal r_dat        : t_wishbone_data := (others => '0');

  -- time of the auxiliary WR port [ns]; crossing to clk_i as done in eca.vhd
  signal rst_ref_aux_n : std_logic;
  signal s_aux_time    : t_time;
  signal ra_time       : t_time;
  signal ra_time_gray  : t_time;
  signal rc_time_gray0 : t_time;
  signal rc_time_gray1 : t_time;
  signal rc_time       : t_time;
  signal r_aux_time    : t_time := (others => '0');

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

  aux_time_y : if g_dual_port_wr generate
    -- reset for the clk_ref_aux_i domain: asserted asynchronously, released synchronously
    sync_rst_ref_aux : gc_sync
      port map (
        clk_i     => clk_ref_aux_i,
        rst_n_a_i => rst_n_i,
        d_i       => '1',
        q_o       => rst_ref_aux_n);

    -- tai and cycles to ns, same as for the primary WR port in monster
    aux_eca_time : eca_wr_time
      port map(
        clk_i     => clk_ref_aux_i,
        rst_n_i   => rst_ref_aux_n,
        tai_i     => wr_aux_tai_i,
        cycles_i  => wr_aux_cycles_i,
        time_o    => s_aux_time);

    time : process(clk_ref_aux_i) is
    begin
      if rising_edge(clk_ref_aux_i) then
        ra_time      <= s_aux_time;
        ra_time_gray <= f_eca_gray_encode(ra_time);
      end if;
    end process;

    bulk : process(clk_i) is
    begin
      if rising_edge(clk_i) then
        rc_time_gray0 <= ra_time_gray;
        rc_time_gray1 <= rc_time_gray0;
        rc_time       <= f_eca_gray_decode(rc_time_gray1, 1);
      end if;
    end process;
  end generate;

  aux_time_n : if not g_dual_port_wr generate
    rc_time <= (others => '0');
  end generate;

  slave_o.dat   <= r_dat;
  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.rty   <= '0';

  wb_handler : process(clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      r_ack         <= '0';
      r_dat         <= (others => '0');
      r_aux_time    <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');
      -- 0x20: any access (read or write) latches the time of the auxiliary WR port
      if slave_i.cyc = '1' and slave_i.stb = '1' and slave_i.adr(7 downto 2) = "001000" then
        r_aux_time <= rc_time;
      end if;
      if slave_i.cyc = '1' and slave_i.stb = '1' and slave_i.we = '0' then
        case slave_i.adr(7 downto 2) is
          when "000000" => -- 0x00: primary WR port
            r_dat(c_wr_info_time_valid_bit) <= status_sync(0);
            r_dat(c_wr_info_link_valid_bit) <= status_sync(1);
          when "000001" => -- 0x04: auxiliary WR port
            r_dat(c_wr_info_time_valid_bit) <= status_sync(2);
            r_dat(c_wr_info_link_valid_bit) <= status_sync(3);
            if g_dual_port_wr then
              r_dat(c_wr_info_present_bit)  <= '1';
            end if;
          when "001001" => -- 0x24: auxiliary WR port, latched time high [ns]
            r_dat <= r_aux_time(63 downto 32);
          when "001010" => -- 0x28: auxiliary WR port, latched time low [ns]
            r_dat <= r_aux_time(31 downto 0);
          when others => null;
        end case;
      end if;
    end if;
  end process;

end rtl;
