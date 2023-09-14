--extended version of the quench_detection.vhd file for QuD_Trigger Matrices with 6 IN/OUT Cards
-- author: A. Russo

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity TM_quench_detection is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC;
           delay: in STD_LOGIC_VECTOR(23 DOWNTO 0);
           quench_out_nr : in STD_LOGIC_VECTOR(5 downto 0);
           QuDIn: in STD_LOGIC_VECTOR (53 downto 0);
           mute: in STD_LOGIC_VECTOR (53 downto 0);
           QuDOut : out STD_LOGIC_VECTOR(23 downto 0));
end TM_quench_detection;

architecture Arch_quench_detection of TM_quench_detection is
Type delay_count_type is array (0 to 23) of std_logic_vector (11 downto 0);
--signal delay_count  :    std_logic_vector (11 downto 0):=X"032";
signal delay_count  : delay_count_type;

signal delay_ctr_tc :    std_logic_vector(23 downto 0);
signal QuD_data_in: STD_LOGIC_VECTOR (53 downto 0);
signal QuD_data_out: STD_LOGIC_VECTOR (23 downto 0):=(others =>'0');
signal delay_ctr_load :  std_logic_vector(23 downto 0);
signal combi : std_logic_vector(23 downto 0);
signal int_sel: integer range 0 to 63;

begin

QuD_data_in <=not( QuDIn or mute) ;

data_process: process (clk, nReset)
begin
    if nReset = '0' then

        QuD_data_out <=(others =>'0');
        int_sel <= 0;
        combi <= (others =>'0');

    elsif rising_edge (clk) then
        int_sel <= to_integer(unsigned(quench_out_nr));
        for i in 0 to 23 loop
            QuD_data_out(i) <= QUD_data_in(int_sel);
        end loop;

        combi <= QuD_data_out;
     
    end if;
end process;

          
---Delay-Count-------------------------------------------
delay_proc: process (clk, nReset)
begin
        if nReset = '0' then
            for i in 0 to 23 loop
          delay_count(i) <= X"032";
          end loop;
          QuDOut <= (others =>'0');

        elsif rising_edge (clk) then
            for i in 0 to 23 loop
            if delay_ctr_load(i)= '1' then
                delay_count(i) <= X"032";
            elsif (delay_ctr_tc(i) = '0') then
                if time_pulse = '1' then
                delay_count(i) <= delay_count(i) - 1;
                end if;
            end if;
            end loop;

for i in 0 to 23 loop

    if delay_count(i) =0 then
        delay_ctr_tc(i) <= '1' ;
    else
        delay_ctr_tc(i) <= '0';
    end if;
end loop;



for i in 0 to 23 loop
    if combi(i)='1' then 
        delay_ctr_load(i) <='1';
    else
        delay_ctr_load(i) <='0';
    end if;

end loop;

for i in 0 to 23 loop
    if delay(i) ='0' then
        QuDOut(i) <= combi(i);
    else
        QuDOut(i) <= not delay_ctr_tc(i);
    end if;
end loop;
end if;
end process;

end Arch_quench_detection;