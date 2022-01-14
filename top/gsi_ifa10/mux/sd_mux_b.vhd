library ieee;
use ieee.std_logic_1164.all;

entity sd_mux_b is
  port (
    ifa_sd:          in std_logic_vector(15 downto 0);
    mb_sd:           in std_logic_vector(15 downto 0);
    fb_sd:           in std_logic_vector(15 downto 0);
    fg_sd:           in std_logic_vector(15 downto 0);
    sweep_stat:      in std_logic_vector(15 downto 0);
    FG1x2_sd:        in std_logic_vector(15 downto 0);
    FG122_sd:        in std_logic_vector(15 downto 0);

    mb_rd_active:    in std_logic;
    fb_rd_active:    in std_logic;
    fg_rd_active:    in std_logic;
    sweep_stat_rd:   in std_logic;
    FG112_rd_active: in STD_LOGIC;
    FG122_rd_active: in STD_LOGIC;

    sd:              out std_logic_vector(15 downto 0));
end entity;

architecture arch of sd_mux_b is


begin

--VK "SD" hier ganz übel Multi-Quellen!-> ISSUE!
--Position in VHDL bestimmt Select-Wert

--mux: process(mb_rd_active, fb_rd_active, fg_rd_active, sweep_stat_rd,ifa_sd,mb_sd,fb_sd,sweep_stat,fg_sd)
--  begin
--    if mb_rd_active = '0' then
--      sd <= ifa_sd;
--    else
--      sd <= mb_sd;
--    end if;
--
--    if fb_rd_active = '1' then
--      sd <= fb_sd;
--    end if;
--
--    if fg_rd_active = '1' then
--      sd <= fg_sd;
--    end if;
--
--    if sweep_stat_rd = '1' then
--      sd <= sweep_stat;
--    end if;
--  end process;

--vk: Umsetzung nach RTL-Generator
mux: process(mb_rd_active, fb_rd_active, fg_rd_active, sweep_stat_rd,ifa_sd,mb_sd,fb_sd,sweep_stat,fg_sd,FG112_rd_active,FG1x2_sd,FG122_sd,FG122_rd_active)

  begin
  if FG122_rd_active ='1' then --nur wenn im modul 122 werden die Daten von hier geholt --überschreibt die ifa intern
      sd<= FG122_sd;
  elsif FG112_rd_active ='1' then --nur wenn im modul 122 werden die Daten von hier geholt --überschreibt die ifa intern
      sd<= FG1x2_sd;
  elsif sweep_stat_rd = '1' then
      sd <= sweep_stat;
   elsif fg_rd_active = '1' then
      sd <= fg_sd;
   elsif fb_rd_active = '1' then
      sd <= fb_sd;
   elsif mb_rd_active = '1' then
      sd <= mb_sd;
   else
      sd <= ifa_sd;
    end if;

  end process;

end architecture;
