library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;

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

  arria2 : if g_family = "Arria II" generate
    ibuf : arria2_lvds_ibuf
      port map(
        datain(0)   => datain,
        datain_b(0) => datain_b,
        dataout(0)  => dataout);
  end generate;

  arria5 : if g_family = "Arria V" generate
    ibuf : arria5_lvds_ibuf
      port map(
        datain(0)   => datain,
        datain_b(0) => datain_b,
        dataout(0)  => dataout);
  end generate;

  arria10_scu4 : if g_family = "Arria 10 GX SCU4" generate
    ibuf : arria10_scu4_lvds_ibuf
      port map(
        pad_in(0)   => datain,
        pad_in_b(0) => datain_b,
        dout(0)     => dataout);
  end generate;

  arria10_pex10 : if g_family = "Arria 10 GX PEX10" generate
    ibuf : arria10_pex10_lvds_ibuf
      port map(
        pad_in(0)   => datain,
        pad_in_b(0) => datain_b,
        dout(0)     => dataout);
  end generate;

  arria10_ftm10 : if g_family = "Arria 10 GX FTM10" generate
    ibuf : arria10_ftm10_lvds_ibuf
      port map(
        pad_in(0)   => datain,
        pad_in_b(0) => datain_b,
        dout(0)     => dataout);
  end generate;

end rtl;
