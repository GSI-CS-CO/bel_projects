--extended version of the quench_detection.vhd file for QuD_Trigger Matrices with 6 IN/OUT Cards
-- author: A. Russo

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
library work;
use work.qud_pkg.all;

entity TM_quench_detection is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC;
           delay: in STD_LOGIC_VECTOR(23 DOWNTO 0);
           quench_out_sel : in STD_LOGIC_VECTOR(5 downto 0);
           qud_reset: in std_logic;
           write : in std_logic;
           QuDIn: in STD_LOGIC_VECTOR (53 downto 0);
           quench_en: in STD_LOGIC_VECTOR (53 downto 0);
           mute: in STD_LOGIC_VECTOR (53 downto 0);  
           QuDOut : out STD_LOGIC_VECTOR(23 downto 0));
end TM_quench_detection;

architecture Arch_quench_detection of TM_quench_detection is

    component del_quench_detection 
        Port ( clk : in STD_LOGIC;
               nReset : in STD_LOGIC;
               time_pulse : in STD_LOGIC;
               delay: in STD_LOGIC;
            combi: in std_logic;
               QuDOut : out STD_LOGIC);

    end component; 
    
    component QuD_masken_reg 
        Port ( clk : in STD_LOGIC;
               nReset : in STD_LOGIC;      
               reg_addr : in STD_LOGIC_VECTOR(5 downto 0);
               reg_reset: in std_logic;
               write : in std_logic;
               quench_en: in STD_LOGIC_VECTOR (53 downto 0);
               QuD_mask : out t_qud_mask);
    end component;
    

Type delay_count_type is array (1 to 23) of std_logic_vector (11 downto 0);
signal delay_count  : delay_count_type;

signal delay_ctr_tc :    std_logic_vector(23 downto 0);

signal delay_ctr_load :  std_logic_vector(23 downto 0);
signal combi : std_logic_vector(23 downto 0):=(others =>'0');

signal QuD_data: STD_LOGIC_VECTOR (23 downto 0);

signal in_count:t_qud_cnt;
signal QuD_mask_reg :t_qud_mask;
signal flag:t_qud_cnt;

begin


    del_quench_det: for i in 0 to 23 generate 
     e_elem: del_quench_detection 
     Port map( clk => clk,
            nReset => nReset,
            time_pulse => time_pulse,
            delay=> delay(i),
             combi => combi(i),
            QuDOut => QuD_data(i));
end generate del_quench_det;

    QuD_masken : QuD_masken_reg 
    Port map( clk => clk,
           nReset => nReset,  
           reg_addr =>quench_out_sel,
           reg_reset => qud_reset,
           write => write,
           quench_en => quench_en and (not mute),
           QuD_mask => QuD_mask_reg);

     --combi <= '1' when (  not (QuDIn or mute ) = 0) else '0';


data_quench_process: process (clk, nReset)
begin
    if (nReset = '0' ) then
         
        for i in 0 to 23 loop
        flag(i) <= 0;
        in_count (i) <=0;
        end loop;
        
        combi <= (others =>'0');

    elsif rising_edge (clk) then
        if qud_reset = '1' then 
        for i in 0 to 23 loop
            flag(i) <= 0;
            in_count (i) <=0;
            end loop;
            combi <=(others =>'0');
        else

  
  
        for i in 0 to 23 loop
            flag(i) <= 0;
            in_count (i) <=0;
            end loop;
            
    for j in 0 to 23 loop
    for i in 0 to 53 loop
      if QuD_mask_reg(j)(i) = '1' then
        if QUDin(i) ='0'then
            flag(j) <= flag(j) + 1;
        else
            if QUDin(i) ='1'then
                in_count(j) <= in_count(j) +1;  
            end if;
        end if; 
   
      end if;

      end loop;
      if (in_count(j) > 0) and (flag (j)= 0) then 
      combi(j) <='1';
    else 
        combi(j) <= '0';
    end if;  
      
 
  end loop;

  end if;
  end if;

end process;

QuDout <= QuD_data;

end Arch_quench_detection;