library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity del_quench_detection is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC;
           delay: in STD_LOGIC;
           combi : in std_logic;
          
           QuDOut : out STD_LOGIC);
end del_quench_detection;


architecture Arch_quench_detection of del_quench_detection is


        signal delay_count  :    std_logic_vector (11 downto 0):=X"032";
        signal delay_ctr_tc :    std_logic;
        signal delay_ctr_en :    std_logic;
        signal delay_ctr_load :  std_logic;
       

        begin
       
        --combi <= '1' when (  not (QuDIn or mute ) = 0) else '0';
       

        ---Delay-Count-------------------------------------------
        process (clk, nReset)
        begin
                if nReset = '0' then
                  delay_count <= X"032";
                
                  
                elsif rising_edge (clk) then
                     
        
                    if delay_ctr_load= '1' then
                        delay_count <= X"032";
                    elsif (delay_ctr_tc = '0') then
                        if time_pulse = '1' then
                        delay_count <= delay_count - 1;
                        end if;
                    end if;
                end if;
        end process;
        delay_ctr_tc <= '1' when (delay_count = 0) else '0';
        delay_ctr_load <=  '1' when (combi = '1')  else '0';
        
        
        QuDOut <= combi when (delay = '0') else not delay_ctr_tc;
        
        
end Arch_quench_detection;
