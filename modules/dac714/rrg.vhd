--Realtime Ramp Generator


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity rrg is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC; -- an enable every 1 us 
           ControlReg : in STD_LOGIC_VECTOR (15 downto 0);
           TargetReg  : in STD_LOGIC_VECTOR (15 downto 0);
           TimeStepReg : in STD_LOGIC_VECTOR (15 downto 0):= "0000000000000100"; -- in microsec
  CountStepReg : in STD_LOGIC_VECTOR (15 downto 0):= "0000000000000001"; 
           DAC_Out : out STD_LOGIC_VECTOR (15 downto 0);
           DAC_Strobe : out STD_LOGIC);
end rrg;

architecture Arch_rrg of rrg is


type T_status is(
                      
INITIALIZE, -- initialzing the required variables after achieving the target
                        CHECK, -- checking wherther we need to decrement or increment
                        INCREMENT, -- incrementing the output
                        DECREMENT, -- decrementing the output
OUTPUT
                        );
                        
signal ramp_status : T_status;
signal time_step: STD_LOGIC_VECTOR (15 downto 0); --to count the step time 
signal prev_TargetReg  : signed (15 downto 0); --to save the prev value of the output to continue working on it
signal Strobe_cntr: STD_LOGIC; 
signal step_counter  : unsigned (15 downto 0) := (others =>'0'); 
signal TargetSaver  : signed (15 downto 0);
signal prev_TargetSaver  : signed (15 downto 0);


begin

process_rrg : process (clk, nReset, time_pulse) --the process is senstive to clock and reset signals

begin

--a condition to enter once reset is done

    if (nReset = '1') then
        DAC_Out <= (others =>'0');
  DAC_Strobe <= '0';
  Strobe_cntr<= '0';
  prev_TargetReg <= (others =>'0');
  step_counter <= unsigned(CountStepReg);
ramp_status<= INITIALIZE;
        
-- a condition to enter once a time pulse detected
 
elsif( Strobe_cntr = '1' and clk = '1')then
DAC_Strobe <= '1';
Strobe_cntr <= '0';
 
        elsif (rising_edge(time_pulse) and nReset = '0') then
  
  --when case to switch between the status
            case ramp_status is
        
  
when INITIALIZE =>
time_step <= (others =>'0');
DAC_Strobe <= '0';
step_counter <= unsigned(CountStepReg);
ramp_status<= CHECK;
 
--checking case to check if the required steps counts achieved 
--to switch to inc or dec status and then change the output
--or to decrement or increment
                
                when CHECK =>
 
time_step <= time_step +1;
TargetSaver <= signed(TargetReg);
step_counter <= unsigned(CountStepReg);
DAC_Strobe <= '0';
 
                    if (time_step < TimeStepReg -3 ) then
 
ramp_status<= CHECK;

                        
                    elsif (prev_TargetReg < TargetSaver) then
  
prev_TargetSaver <= TargetSaver;
ramp_status<= INCREMENT;

                        
                    elsif (prev_TargetReg > TargetSaver) then
  
prev_TargetSaver <= TargetSaver;
ramp_status<= DECREMENT;
 
elsif (prev_TargetReg = TargetSaver)then
                        
ramp_status<= OUTPUT;
 
                        
                    end if;
 
                when INCREMENT =>
                        time_step <= (others =>'0');
                        
                            if  (prev_TargetReg + signed (step_counter ) > prev_TargetSaver) then
prev_TargetReg <=  prev_TargetSaver;
else
prev_TargetReg <= signed(prev_TargetReg) + signed (step_counter);
end if;
 
ramp_status <= OUTPUT;

 
 
 
                
when DECREMENT =>

time_step <= (others =>'0');
                            if  (prev_TargetReg - signed (step_counter ) < prev_TargetSaver) then
prev_TargetReg <=  prev_TargetSaver;
else
 
prev_TargetReg <= signed(prev_TargetReg) - signed (step_counter);
 
end if;
 
ramp_status <= OUTPUT;

 

 
 
                when OUTPUT =>
DAC_Out <= STD_LOGIC_VECTOR(prev_TargetReg);
Strobe_cntr <='1';
ramp_status<= CHECK;            
                    
                        
        end case;
    end if;
end process;


end Arch_rrg;