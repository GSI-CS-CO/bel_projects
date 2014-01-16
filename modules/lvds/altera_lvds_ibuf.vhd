library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria5_lvds_pkg.all;

entity altera_lvds_ibuf is
  generic(
    g_family : string);
  port(
    datain   : in  std_logic;
    datain_b : in  std_logic;
    dataout  : out std_logic);
end altera_lvds_ibuf;

architecture rtl of altera_lvds_ibuf is
begin

  arria5 : if g_family = "Arria V" generate
    ibuf : arria5_lvds_ibuf
      port map(
        datain(0)   => datain,
        datain_b(0) => datain_b,
        dataout(0)  => dataout);
  end generate;
  
end rtl;
