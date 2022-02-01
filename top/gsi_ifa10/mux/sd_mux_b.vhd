----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:    30.01.2022
-- Design Name:
-- Module Name:    sd_mux_b - Behavioral
-- Project Name:   ifa10
-- Target Devices:  MAX10
-- Tool versions:  Quartus 16 +

-- Description:
-- Auswahl der zu sendenden MIL-Wörter über Select-Leitungen
-- Dependencies:

----------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

entity sd_mux_b is
  port (
    sys_clk:         in std_logic;
    sys_reset:       in std_logic;
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
	 WR_Mil_in:       in std_logic;

	 WR_Mil_out:      out std_logic;
	 sd:              out std_logic_vector(15 downto 0)
);

end entity;

architecture arch of sd_mux_b is

signal WR_Mil_delay:   std_logic:='0';


begin


mux: process(sys_reset,sys_clk,WR_Mil_in,mb_rd_active,fg_rd_active,fb_rd_active, sweep_stat_rd,FG112_rd_active,FG122_rd_active)

	begin
		if sys_reset = '1'then

			WR_Mil_out 		<= '0';
			WR_Mil_delay	<= '0';
			sd         		<= ( others =>'0');

		elsif rising_edge(sys_clk) then

		     --delay one sysclk to settle and fix sd outs
			WR_Mil_delay <= WR_Mil_in;
			WR_Mil_out   <= WR_Mil_delay; 
			   
			if WR_Mil_in='1' then
			   sd <= ifa_sd;
				
				if FG122_rd_active ='1' then --nur wenn im modul 122 werden die Daten von hier geholt --überschreibt die ifa intern
					sd<= FG122_sd;
				elsif FG112_rd_active = '1' then --nur wenn im modul 122 werden die Daten von hier geholt --überschreibt die ifa intern
					sd<= FG1x2_sd;
				elsif sweep_stat_rd = '1' then
					sd <= sweep_stat;
				elsif fg_rd_active = '1' then
					sd <= fg_sd;
				elsif fb_rd_active = '1' then
					sd <= fb_sd;
				elsif mb_rd_active = '1' then
					sd <= mb_sd;
				end if;
			end if;

	end if; --rising_edge(sys_clk) then

	end process;

end architecture;
