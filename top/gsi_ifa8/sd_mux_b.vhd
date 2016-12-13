library ieee;
use ieee.std_logic_1164.all;

entity sd_mux_b is
  port (
    ifa_sd:       in std_logic_vector(15 downto 0);
    mb_sd:        in std_logic_vector(15 downto 0);
    fb_sd:        in std_logic_vector(15 downto 0);
    fg_sd:        in std_logic_vector(15 downto 0);
    sweep_stat:   in std_logic_vector(15 downto 0);
    
    mb_rd_active:   in std_logic;
    fb_rd_active:   in std_logic;
    fg_rd_active:   in std_logic;
    sweep_stat_rd:  in std_logic;
    
    sd:           out std_logic_vector(15 downto 0));
end entity;

architecture arch of sd_mux_b is
begin
  mux: process(mb_rd_active, fb_rd_active, fg_rd_active, sweep_stat_rd)
  begin
    if mb_rd_active = '0' then
      sd <= ifa_sd;
    else
      sd <= mb_sd;
    end if;
  
    if fb_rd_active = '1' then
      sd <= fb_sd;
    end if;
  
    if fg_rd_active = '1' then
      sd <= fg_sd;
    end if;
  
    if sweep_stat_rd = '1' then
      sd <= sweep_stat;
    end if;
  end process;
end architecture;