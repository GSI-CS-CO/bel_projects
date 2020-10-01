-- ------------------------------------------------------------------------------------------------
-- Simple Reconf interface to wishbone adapter
-- Attention: Clock crossing is ignored, for debugging purpose only
-- ------------------------------------------------------------------------------------------------
-- Wishbone mapping:
-- 0x0: Write data (OUT)
-- 0x4: Address (read and write)
-- 0x8: Read or write access
-- -- [0]: Read
-- -- [1]: Write
-- 0xC: Read date (IN)
-- ------------------------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

entity cpri_phy_reconf is
  port(
    clk_i                  : in  std_logic := '0';
    rst_n_i                : in  std_logic := '1';
    slave_i                : in  t_wishbone_slave_in;
    slave_o                : out t_wishbone_slave_out;
    reconfig_write_o       : out std_logic;
    reconfig_read_o        : out std_logic;
    reconfig_address_o     : out std_logic_vector(31 downto 0);
    reconfig_writedata_o   : out std_logic_vector(31 downto 0);
    reconfig_readdata_i    : in  std_logic_vector(31 downto 0);
    reconfig_waitrequest_i : in  std_logic_vector(0 downto 0));
end cpri_phy_reconf;

architecture rtl of cpri_phy_reconf is
  signal r_ack : std_logic := '0';
  signal r_dat : std_logic_vector(31 downto 0) := (others => '0');
begin

  slave_o.dat        <= r_dat;
  slave_o.ack        <= r_ack;
  slave_o.err        <= '0';
  slave_o.stall      <= '0';
  slave_o.rty        <= '0';

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_ack                <= '0';
      r_dat                <= (others => '0');
      reconfig_read_o      <= '0';
      reconfig_write_o     <= '0';
      reconfig_writedata_o <= (others => '0');
      reconfig_address_o   <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack            <= slave_i.cyc and slave_i.stb;
      r_dat            <= (others => '0');
      if (slave_i.cyc and slave_i.stb and slave_i.we) = '1' then
        case slave_i.adr(3 downto 2) is
            when "00" => reconfig_writedata_o <= slave_i.dat;
            when "01" => reconfig_address_o   <= slave_i.dat;
            when "10" =>
                         reconfig_read_o      <= slave_i.dat(0);
                         reconfig_write_o     <= slave_i.dat(1);
          when others => null;
        end case;
      end if;
      case slave_i.adr(3 downto 2) is
        when "11" => r_dat <= reconfig_readdata_i;
        when others => null;
      end case;
    end if;
  end process;

end rtl;
