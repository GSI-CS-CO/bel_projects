library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria5_lvds_pkg.all;

entity altera_lvds_obuf is
  generic(
    g_family : string);
  port(
    datain    : in  std_logic;
    dataout   : out std_logic;
    dataout_b : out std_logic);
end altera_lvds_obuf;

architecture rtl of altera_lvds_obuf is
begin

  arria5 : if g_family = "Arria V" generate
    obuf : arria5_lvds_obuf
      port map(
        datain(0)        => datain,
        dataout(0)       => dataout,
        dataout_b(0)     => dataout_b);
  end generate;
  
end rtl;
