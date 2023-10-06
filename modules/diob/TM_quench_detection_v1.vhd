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
           quench_in_out_sel : in STD_LOGIC_VECTOR(11 downto 0);
           out_reset: in std_logic;
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

Type delay_count_type is array (1 to 23) of std_logic_vector (11 downto 0);
--signal delay_count  :    std_logic_vector (11 downto 0):=X"032";
signal delay_count  : delay_count_type;

signal delay_ctr_tc :    std_logic_vector(23 downto 0);

signal delay_ctr_load :  std_logic_vector(23 downto 0);
signal combi : std_logic_vector(23 downto 0):=(others =>'0');
signal int_sel: integer range 0 to 63:=0;
signal out_sel: integer range 0 to 23:=0;
signal QuD_data: STD_LOGIC_VECTOR (23 downto 0);
signal QuD_in_data: STD_LOGIC_VECTOR (53 downto 0);
begin


    del_quench_det_test: del_quench_detection 
     Port map( clk => clk,
            nReset => nReset,
            time_pulse => time_pulse,
            delay=> delay(0),
             combi => combi(0),
            QuDOut => QuD_data(0));


    del_quench_det: for i in 1 to 23 generate 
     e_elem: del_quench_detection 
     Port map( clk => clk,
            nReset => nReset,
            time_pulse => time_pulse,
            delay=> delay(i),
             combi => combi(i),
            QuDOut => QuD_data(i));
end generate del_quench_det;



     --combi <= '1' when (  not (QuDIn or mute ) = 0) else '0';


data_out_process: process (clk, nReset)
begin
    if nReset = '0' then

        int_sel <= 0;
        out_sel <= 0;
        combi <= (others =>'0');
      

    elsif rising_edge (clk) then
        if out_reset ='1' then 
            QuD_in_data <= (others =>'0');
            combi  <= (others =>'0');
            QuDout <= (others =>'0');
        else
            QuD_in_data (0) <= QuDIn(0) and (not mute (0));
            for i in 1 to 53 loop
                QuD_in_data(i) <= QuDin(i) and ((not mute(i)) and quench_en(i));
            end loop;

            int_sel <= to_integer(unsigned(quench_in_out_sel(5 downto 0)));
            out_sel <= to_integer(unsigned(quench_in_out_sel(11 downto 6)));
            combi(out_sel)<=QuD_in_data(int_sel);

            QuDout <= QUD_data;
        end if;
    end if;
end process;
       


end Arch_quench_detection;


