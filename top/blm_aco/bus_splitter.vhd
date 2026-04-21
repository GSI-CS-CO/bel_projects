
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;


entity bus_splitter is
    port(
   -- from bus

        clock: in std_logic;

        A_A: in std_logic; -- SCU-Adress Bus
        A_nDS: in std_logic; -- Data strobe driven by master
     
        nSel_Ext_Data_Drv_out : out std_logic; -- '0' => select the external data driver on the SCU_Bus slave
        --A_D: inout std_logic_vector(15 downto 0); -- SCU-Data Bus
        A_nDtack: out std_logic; -- Data-Acknowlege zero active, '0' => enables external open drain driver

    -- from/to slave 1
        A_nDS_to_1: out std_logic; -- Data strobe driven by master to slave
        SCU_Dtack_from_1: in  std_logic;    -- for connect via ext. open collector driver - '1' => slave givedtack to SCU master
        nSel_Ext_Data_Drv_in_from_1:  in std_logic;    -- '0' => select the external data driver on the SCU_Bus slave
      --  A_D_to_1: inout std_logic_vector(15 downto 0); -- SCU-Data Bus
        -- from/to ram 2
        A_nDS_to_2: out std_logic; -- Data strobe driven by master to slave
        SCU_Dtack_from_2: in  std_logic;    -- for connect via ext. open collector driver - '1' => slave givedtack to SCU master
        nSel_Ext_Data_Drv_in_from_2:  in std_logic   -- '0' => select the external data driver on the SCU_Bus slave
       -- A_D_to_2: inout std_logic_vector(15 downto 0) -- SCU-Data Bus
    );
end bus_splitter;

architecture rtl of bus_splitter is

    signal sync_nds: std_logic;
   -- signal sync_A_A: std_logic;
    signal sync_dtack_from2: std_logic;
    signal sync_dtack_from1: std_logic;
    signal sync_ext_DD_from2: std_logic;
    signal sync_ext_DD_from1: std_logic;
    
begin

    sync_proc: process(clock)
    begin
     
        if (clock'EVENT AND clock= '1') then 
            sync_nds <= A_nDS;
           -- sync_A_A <= A_A;
            sync_dtack_from2 <= SCU_Dtack_from_2;
            sync_dtack_from1 <= SCU_Dtack_from_1;
            sync_ext_DD_from1 <= nSel_Ext_Data_Drv_in_from_1;
            sync_ext_DD_from2 <= nSel_Ext_Data_Drv_in_from_2;
        end if;
        
    end process;


signal_distribution_proc: process(A_A, A_nDS, SCU_DTack_from_1, SCU_DTack_from_2, nSel_Ext_Data_Drv_in_from_1, nSel_Ext_Data_Drv_in_from_2)
begin
 
    -- Controlling DS & DTACK
    if  A_A = '0' then -- signals to 1
        A_nDS_to_1 <= sync_nds; --A_nDS;
        A_nDS_to_2  <= '1';
        A_nDTack <= not (sync_dtack_from1); -- not(SCU_Dtack_from_1);
    else -- signals to RAM
        A_nDS_to_2 <= sync_nds; -- A_nDS;
        A_nDS_to_1  <= '1';
        A_nDTack <= not (sync_dtack_from2); --not(SCU_Dtack_from_2);
    end if;
   
    -- Controlling data bus
   -- if  nSel_Ext_Data_Drv_in_from_1 ='0' then
   if  sync_ext_DD_from1  ='0' then
        nSel_Ext_Data_Drv_out <= '0';
        --A_D_to_1 <= (others=>'Z');
       -- A_D_to_2 <= (others=>'Z');
       -- A_D <= A_D_to_1;
   -- elsif nSel_Ext_Data_Drv_in_from_2 ='0' then
    elsif sync_ext_DD_from2 ='0' then
        nSel_Ext_Data_Drv_out <= '0';
       -- A_D_to_1 <= (others=>'Z');
       -- A_D_to_2 <= (others=>'Z');
       -- A_D <= A_D_to_2;
    else
        nSel_Ext_Data_Drv_out <= '1';
      --  A_D_to_1 <= A_D;
     --   A_D_to_2 <= A_D;
     --   A_D <= (others=>'Z');
    end if;

end process;

end architecture rtl;