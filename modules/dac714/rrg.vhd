----Realtime Ramp Generator
--
--

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
						DAC_OUTPUT,
						STROBE
                        );
                        
signal ramp_status : T_status;
signal next_ramp_status : T_status := INITIALIZE;
signal time_step: STD_LOGIC_VECTOR (15 downto 0); --to count the step time 
signal time_step_tc : STD_LOGIC; 
signal prev_TargetReg  : signed (15 downto 0) := (others =>'0'); --to save the prev value of the output to continue working on it
signal step_counter  : unsigned (15 downto 0) := (others =>'0');
signal TargetSaver  : signed (15 downto 0);
signal prev_TargetSaver  : signed (15 downto 0) := (others =>'0');


begin

--Clocking
process (clk, nReset)
    begin
        if rising_edge (clk)
        then
            if nReset = '0' then
            ramp_status <= INITIALIZE;
            else
            ramp_status  <= next_ramp_status;
            end if;
        end if;

end process;


process_rrg : process (ramp_status,time_step_tc,prev_TargetReg,step_counter) --the process is senstive to clock and reset signals

begin
           next_ramp_status <= ramp_status;    
 
           --when case to switch between the status
           case ramp_status is
       
 
               when INITIALIZE =>
                   DAC_Out <= (others =>'0');
                   DAC_Strobe <= '0';
                   step_counter <= unsigned(CountStepReg);
                   next_ramp_status<= CHECK;

               --checking case to check if the required steps counts achieved 
               --to switch to inc or dec status and then change the output
               --or to decrement or increment
               
               when CHECK =>

                   TargetSaver <= signed(TargetReg);
                   step_counter <= unsigned(CountStepReg);
                   DAC_Strobe <= '0';

                   if (time_step_tc = '0' ) then

                       next_ramp_status<= CHECK;
                       
                   elsif (prev_TargetReg < TargetSaver) then
 
                       prev_TargetSaver <= TargetSaver;                                          
                       next_ramp_status<= INCREMENT;
                       
                   elsif (prev_TargetReg > TargetSaver) then
 
                       prev_TargetSaver <= TargetSaver;
                       next_ramp_status<= DECREMENT;
                   --elsif (prev_TargetReg = TargetSaver)then
                   --    next_ramp_status<= DAC_OUTPUT;

                       
                   end if;

               when INCREMENT =>	
                       
                       if  (prev_TargetReg + signed (step_counter ) > prev_TargetSaver) then
                           prev_TargetReg <=  prev_TargetSaver;
                       else
                           prev_TargetReg <= signed(prev_TargetReg) + signed (step_counter);
                       end if;

                   next_ramp_status <= DAC_OUTPUT;
               
               when DECREMENT =>
                   if  (prev_TargetReg - signed (step_counter ) < prev_TargetSaver) then
                       prev_TargetReg <=  prev_TargetSaver;
                   else

                       prev_TargetReg <= signed(prev_TargetReg) - signed (step_counter);
                   end if;

                   next_ramp_status <= DAC_OUTPUT;


               when DAC_OUTPUT =>
                   DAC_Out <= STD_LOGIC_VECTOR(prev_TargetReg);
                   next_ramp_status<= STROBE;
               when STROBE =>
                   DAC_Strobe <= '1';
                   next_ramp_status<= CHECK;            
                   
                       
               end case;
end process;

--Step Counter
process (clk,nReset) 
begin
   if nReset = '0' then
    time_step <= X"FFFF";
   else
        if rising_edge (clk) then
            if time_step_tc = '1' then
                time_step <= TimeStepReg;    
            elsif time_pulse = '1' then
                time_step <= time_step-1;
            end if;
         end if;
   end if;
end process; 

time_step_tc <= '1' when (time_step = 0) else '0';


end Arch_rrg;
