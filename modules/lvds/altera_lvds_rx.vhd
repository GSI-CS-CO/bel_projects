library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;

entity altera_lvds_rx is
  generic(
    g_family : string);
  port(
    rx_core    : in  std_logic;
    rx_inclock : in  std_logic;
    rx_enable  : in  std_logic;
    rx_in      : in  std_logic;
    rx_out     : out std_logic_vector(7 downto 0));
end altera_lvds_rx;

architecture rtl of altera_lvds_rx is
  signal raw : std_logic_vector(7 downto 0);
begin

  main : process(rx_core) is
  begin
    if rising_edge(rx_core) then
      rx_out <= raw;
    end if;
  end process;
  
  arria2 : if g_family = "Arria II" generate
    rx : arria2_lvds_rx
      port map(
        rx_inclock => rx_inclock,
        rx_enable  => rx_enable,
        rx_in(0)   => rx_in,
        rx_out     => raw);
  end generate;
  
  arria5 : if g_family = "Arria V" generate
    rx : arria5_lvds_rx
      port map(
        rx_inclock => rx_inclock,
        rx_enable  => rx_enable,
        rx_in(0)   => rx_in,
        rx_out     => raw);
  end generate;

  arria10_scu4 : if g_family = "Arria 10 GX SCU4" generate
    rx : arria10_scu4_lvds_rx
      port map(
        ext_fclk      => rx_inclock,
        ext_loaden    => rx_enable,
        ext_coreclock => rx_core,
        rx_in(0)      => rx_in,
        rx_out        => raw);
  end generate;

  arria10_pex10 : if g_family = "Arria 10 GX PEX10" generate
    rx : arria10_pex10_lvds_rx
      port map(
        ext_fclk      => rx_inclock,
        ext_loaden    => rx_enable,
        ext_coreclock => rx_core,
        rx_in(0)      => rx_in,
        rx_out        => raw);
  end generate;

  arria10_ftm10 : if g_family = "Arria 10 GX FTM10" generate
    rx : arria10_ftm10_lvds_rx
      port map(
        ext_fclk      => rx_inclock,
        ext_loaden    => rx_enable,
        ext_coreclock => rx_core,
        rx_in(0)      => rx_in,
        rx_out        => raw);
  end generate;

end rtl;
