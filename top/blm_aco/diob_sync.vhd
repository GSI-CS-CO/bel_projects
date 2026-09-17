library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;
use ieee.math_real.all;


entity diob_sync is
port(
    Sync_In:    in  std_logic;
    Reset:      in  std_logic;
    Clk:        in  std_logic;
    Sync_Out:   out std_logic
    );
end diob_sync;

architecture Arch_diob_sync OF diob_sync is
    
SIGNAL      S_Sync_Meta:    std_logic;
SIGNAL      S_Sync_Out:     std_logic;

begin


P_Sync: process (Reset, Clk, Sync_In) 
  begin
    if Reset = '1' then
      s_Sync_Out <= '0';
      elsif rising_edge(clk) then
      S_Sync_Meta <= Sync_In;
      S_Sync_Out  <= S_Sync_Meta;
    end if;
  end process P_Sync;

Sync_Out <= S_Sync_Out;

end Arch_diob_sync;
