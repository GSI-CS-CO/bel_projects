----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IFA_SWoutSel.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices:
-- Tool versions:
-- Description:
-- Wählt Ausgabesignale in abhängigkeit des Modus aus
-- Sollwert-MUX
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:


LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;


ENTITY IFA_SWoutSel IS

  PORT
  (
      sys_clk        : in  std_logic;     -- System-Clock
      sys_reset      : in  std_logic;     -- syn. Clear

      FG_Mode        : in std_logic;      -- Funktonsgen.-SEL
      FG_DDS_Mode    : in STD_LOGIC;
      Sweep_Mode     : in std_logic;      -- Sweeper-SEL

      FG             : in STD_LOGIC_VECTOR(31 DOWNTO 8);
      FG_DDS_Out     : in STD_LOGIC_VECTOR(15 DOWNTO 0);
      Sweep_Out      : in STD_LOGIC_VECTOR(15 DOWNTO 0);
      IFA_Data       : in STD_LOGIC_VECTOR(15 DOWNTO 0);

      SW             : out STD_LOGIC_VECTOR(15 DOWNTO 0);
      SWF            : out STD_LOGIC_VECTOR(7 DOWNTO 0)

);

END IFA_SWoutSel;


architecture Behavioral of IFA_SWoutSel is

begin

process(sys_clk,sys_reset,FG_DDS_Mode,FG_Mode,Sweep_Mode,IFA_Data,FG,FG_DDS_Out,Sweep_Out)
begin

if rising_edge(sys_clk) then
  if FG_Mode='1' then
    SW(15 downto 0) <= FG(31 downto 16);
    SWF(7 downto 0) <= FG(15 downto 8);
  else
    if FG_DDS_Mode= '1' then
      SW(15 downto 0) <= FG_DDS_Out(15 downto 0);
      SWF(7 downto 0) <= (others => '0');
    else
        if Sweep_Mode= '1' then
          SW(15 downto 0) <= Sweep_Out(15 downto 0);
          SWF(7 downto 0) <= (others => '0');
        else
          SW(15 downto 0) <=IFA_Data(15 downto 0);   -- SW IFA
          SWF(7 downto 0) <= (others => '0');
        end if;
     end if;
  end if;
  end if;   --rising_edge(sys_clk)

end process;


end Behavioral;

