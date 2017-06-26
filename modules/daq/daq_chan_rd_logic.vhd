library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;


library work;
use work.genram_pkg.all;
use work.daq_pkg.all;


------------------------------------------------------------------------------------------------------------------------------------
--  Vers: 0 Revi: 0: 2016Sep27 K.Kaiser Initial Version    
--  This entity is to be used in a for-generate loop
--  PMDat and DaqDat are read-only by conception 
--  Rd_Ports are or-ed on level above to concentrate RD Bus
------------------------------------------------------------------------------------------------------------------------------------


entity daq_chan_rd_logic is

generic(
      PmDat_adr:            unsigned(15 downto 0):= x"0000";
      DaqDat_adr:           unsigned(15 downto 0):= x"0000";
	    CtrlReg_adr:          unsigned(15 downto 0):= x"0000";
      trig_lw_adr:          unsigned(15 downto 0):= x"0000";
      trig_hw_adr:          unsigned(15 downto 0):= x"0000";
      trig_dly_word_adr:    unsigned(15 downto 0):= x"0000"
      );

port  (
      -- SCUB interface
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:		    in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:			   	    in		std_logic;							      -- should be the same clk, used by SCU_Bus_Slave
      nReset:			        in		std_logic;
      
      PmDat:              in    std_logic_vector(15 downto 0);
      DaqDat:             in    std_logic_vector(15 downto 0);
      CtrlReg:            in    std_logic_vector(15 downto 0);
      TrigWord_LW :       in    std_logic_vector(15 downto 0);
      TrigWord_HW:        in    std_logic_vector(15 downto 0);
      Trig_dly_word:      in    std_logic_vector(15 downto 0);
    
    
      Ena_PM_rd:          in    std_logic;
      
      Rd_Port:            out   std_logic_vector(15 downto 0);
      user_rd_active:     out   std_logic;
      rd_PMDat:           out   std_logic;      
      rd_DAQDat:          out   std_logic;
      dtack :             out   std_logic
           
    );
end daq_chan_rd_logic;


architecture daq_chan_rd_logic_arch of daq_chan_rd_logic is

signal  rd_PMDat_int          :std_logic;
signal  rd_DAQDat_int         :std_logic;
signal  rd_CtrlReg_int        :std_logic;
signal  rd_trig_lw_int        :std_logic;
signal  rd_trig_hw_int        :std_logic;
signal  rd_trig_dly_word_int  :std_logic;


begin

  
adr_decoder: process (clk_i, nReset)
  begin
    if nReset = '0' then
      rd_PMDat_int           <= '0';
      rd_DAQDat_int          <= '0';    
    elsif rising_edge(clk_i) then
      rd_PMDat_int           <= '0';
      rd_DAQDat_int          <= '0';

      if Ext_Adr_Val = '1' then
        case unsigned(Adr_from_SCUB_LA) is

          when PmDat_adr =>  
            if Ext_Rd_active =  '1' then
              rd_PMDat_int   <= '1';
            end if;
            
          when DaqDat_adr =>  
            if Ext_Rd_active =  '1' then
              rd_DAQDat_int  <= '1';
            end if;     
            
          when CtrlReg_adr =>  
            if Ext_Rd_active =  '1' then
              rd_CtrlReg_int  <= '1';
            end if;            
            
          when trig_lw_adr =>  
            if Ext_Rd_active =  '1' then
              rd_trig_lw_int  <= '1';
            end if;            
 
          when trig_hw_adr =>  
            if Ext_Rd_active =  '1' then
              rd_trig_hw_int  <= '1';
            end if;            
 
 
          when trig_dly_word_adr =>  
            if Ext_Rd_active =  '1' then
              rd_trig_dly_word_int  <= '1';
            end if;            
 
          when others =>
        
            rd_PMDat_int          <= '0';
            rd_DAQDat_int         <= '0';
            rd_CtrlReg_int        <= '0';
            rd_trig_lw_int        <= '0';           
            rd_trig_hw_int        <= '0';          
            rd_trig_dly_word_int  <= '0';          
 
          end case;
      end if;
    end if;
    
  end process adr_decoder;
  
rd_PMDat        <= rd_PMDat_int;   -- to trigger fifo read-outs on level above
rd_DAQDat       <= rd_DAQDat_int;

Rd_Port         <= pmdat          when rd_PMDat_int  ='1'  and Ena_PM_rd ='1'  else  -- PM Fifo data only valid on stopped PM/Hires DAQ
                   daqdat         when rd_DAQDat_int ='1'                      else       
                   CtrlReg        when rd_CtrlReg_int ='1'                     else      
                   TrigWord_LW    when rd_trig_lw_int ='1'                     else      
                   TrigWord_HW    when rd_trig_hw_int ='1'                     else      
                   Trig_dly_word  when rd_trig_dly_word_int ='1'               else                         
                   x"0000";


user_rd_active  <=    rd_PMDat_int or rd_DAQDat_int or rd_CtrlReg_int or rd_trig_lw_int or rd_trig_hw_int or rd_trig_dly_word_int;                            
dtack           <=    rd_PMDat_int or rd_DAQDat_int or rd_CtrlReg_int or rd_trig_lw_int or rd_trig_hw_int or rd_trig_dly_word_int;      


end architecture daq_chan_rd_logic_arch;