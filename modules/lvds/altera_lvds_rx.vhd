library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;

entity altera_lvds_rx is
  generic(
    g_family : string);
  port(
    rx_inclock : in  std_logic;
    rx_enable  : in  std_logic;
    rx_in      : in  std_logic;
    rx_out     : out std_logic_vector(7 downto 0));
end altera_lvds_rx;

architecture rtl of altera_lvds_rx is
begin

  arria2 : if g_family = "Arria II" generate
    rx : arria2_lvds_rx
      port map(
        rx_inclock => rx_inclock,
        rx_enable  => rx_enable,
        rx_in(0)   => rx_in,
        rx_out     => rx_out);
  end generate;
  
  arria5 : if g_family = "Arria V" generate
    rx : arria5_lvds_rx
      port map(
        rx_inclock => rx_inclock,
        rx_enable  => rx_enable,
        rx_in(0)   => rx_in,
        rx_out     => rx_out);
  end generate;
  
end rtl;
