--Realtime Ramp Generator


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity rrg is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC;
           ControlReg : in STD_LOGIC_VECTOR (15 downto 0);
           TargetReg  : in STD_LOGIC_VECTOR (15 downto 0);
           TimeStepReg : in STD_LOGIC_VECTOR (15 downto 0);
           DAC_Out : out STD_LOGIC_VECTOR (15 downto 0);
           DAC_Strobe : out STD_LOGIC);
end rrg;

architecture Arch_rrg of rrg is


begin

    --This just fot Testing
    --Put your code here
    DAC_Out <= TimeStepReg;
    DAC_Strobe <= time_pulse;

end Arch_rrg;
