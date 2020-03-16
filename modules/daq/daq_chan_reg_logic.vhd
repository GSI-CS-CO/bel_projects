library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;


library work;
use work.genram_pkg.all;
use work.daq_pkg.all;


------------------------------------------------------------------------------------------------------------------------------------
--  Vers: 0 Revi: 0: 2016Sep27 K.Kaiser Initial Version  
--          Revi: 1: 2018Okt20 K.Kaiser Version Number added 
--  This entity is to be used in a for-generate loop
--  PMDat and DaqDat are read-only by conception 
--  Rd_Ports are or-ed on level above to concentrate RD Bus
------------------------------------------------------------------------------------------------------------------------------------


entity daq_chan_reg_logic is

generic(
      CtrlReg_adr:          unsigned(15 downto 0):= x"0000";
      trig_lw_adr:          unsigned(15 downto 0):= x"0000";
      trig_hw_adr:          unsigned(15 downto 0):= x"0000";
      trig_dly_word_adr:    unsigned(15 downto 0):= x"0000";
      PmDat_adr:            unsigned(15 downto 0):= x"0000";
      DaqDat_adr:           unsigned(15 downto 0):= x"0000";
      DAQ_FIFO_Words_adr:   unsigned(15 downto 0):= x"0000";
      PM_FIFO_Words_adr:    unsigned(15 downto 0):= x"0000"
      );

port  (
      -- SCUB interface
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:			in		std_logic;							      -- '1' => Rd-Cycle is active
      Ext_Wr_active:			in		std_logic;							      -- '1' => Rd-Cycle is active      
      clk_i:						  in		std_logic;							      -- should be the same clk, used by SCU_Bus_Slave
      nReset:					    in		std_logic;
      
      PmDat:              in    std_logic_vector(15 downto 0);
      DaqDat:             in    std_logic_vector(15 downto 0);
      Ena_PM_rd:          in    std_logic;
      daq_fifo_word:      in    std_logic_vector (8 downto 0);     
      pm_fifo_word:       in    std_logic_vector (9 downto 0);
      version_number:     in    std_logic_vector (15 downto 9);
      max_ch_cnt:         in    std_logic_vector (15 downto 10);
      
      Rd_Port:            out   std_logic_vector(15 downto 0);
      user_rd_active:     out   std_logic;
      
      CtrlReg_o:          out   std_logic_vector(15 downto 0);
      TrigWord_LW_o :     out   std_logic_vector(15 downto 0);
      TrigWord_HW_o:      out   std_logic_vector(15 downto 0);
      Trig_dly_word_o:    out   std_logic_vector(15 downto 0);
      rd_PMDat:           out   std_logic;      
      rd_DAQDat:          out   std_logic;
      dtack :             out   std_logic
           
    );
end daq_chan_reg_logic;

architecture daq_chan_reg_logic_arch of daq_chan_reg_logic is

signal  rd_PMDat_int       :std_logic;
signal  rd_DAQDat_int      :std_logic;
signal  rd_DAQ_FIFO_Word   :std_logic;
signal  rd_PM_FIFO_Word    :std_logic;

signal wr_CtrlReg          : std_logic;
signal wr_trig_lw          : std_logic;
signal wr_trig_hw          : std_logic;
signal wr_Trig_dly_word    : std_logic;
signal rd_CtrlReg          : std_logic;
signal rd_trig_lw          : std_logic;
signal rd_trig_hw          : std_logic;
signal rd_Trig_dly_word    : std_logic;

signal CtrlReg             : std_logic_vector(15 downto 0);
signal TrigWord_LW         : std_logic_vector(15 downto 0);
signal TrigWord_HW         : std_logic_vector(15 downto 0);
signal Trig_dly_word       : std_logic_vector(15 downto 0);

begin

CtrlReg_o       <= CtrlReg;
TrigWord_LW_o   <= TrigWord_LW;
TrigWord_HW_o   <= TrigWord_HW;
Trig_dly_word_o <= Trig_dly_word;

  
adr_decoder: process (clk_i, nReset)
  begin
    if nReset = '0' then
      rd_PMDat_int          <= '0';
      rd_DAQDat_int         <= '0';  
      rd_DAQ_FIFO_Word      <= '0';
      rd_PM_FIFO_Word       <= '0';
      wr_CtrlReg            <= '0';
      wr_trig_lw            <= '0';
      wr_trig_hw            <= '0';      
      wr_Trig_dly_word      <= '0'; 
      rd_CtrlReg            <= '0';
      rd_trig_lw            <= '0';
      rd_trig_hw            <= '0';      
      rd_Trig_dly_word      <= '0';       
      
    elsif rising_edge(clk_i) then
      rd_PMDat_int          <= '0';
      rd_DAQDat_int         <= '0';
      rd_DAQ_FIFO_Word      <= '0';
      rd_PM_FIFO_Word       <= '0';
      
      wr_CtrlReg            <= '0';
      wr_trig_lw            <= '0';
      wr_trig_hw            <= '0';      
      wr_Trig_dly_word      <= '0'; 
      rd_CtrlReg            <= '0';
      rd_trig_lw            <= '0';
      rd_trig_hw            <= '0';      
      rd_Trig_dly_word      <= '0'; 
      
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
            
          when DAQ_FIFO_Words_adr =>  
            if Ext_Rd_active =  '1' then
              rd_DAQ_FIFO_Word  <= '1';
            end if;      
            
          when PM_FIFO_Words_adr =>  
            if Ext_Rd_active =  '1' then
              rd_PM_FIFO_Word  <= '1';
            end if;            

           when CtrlReg_adr =>  
            if   Ext_Wr_active = '1' then
              wr_CtrlReg       <= '1';
            elsif Ext_rd_active ='1' then
              rd_CtrlReg       <= '1';
           end if;

          when trig_lw_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_lw       <= '1';
            elsif Ext_rd_active ='1' then
              rd_trig_lw       <= '1';            
            end if;
            
          when trig_hw_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_hw       <= '1';
            elsif Ext_rd_active ='1' then
              rd_trig_hw       <= '1';
            end if;
 
          when trig_dly_word_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_dly_word    <= '1';
            elsif Ext_rd_active ='1' then
              rd_trig_dly_word    <= '1';            
            end if;           
            
            
            
          when others =>
        
            rd_PMDat_int     <= '0';
            rd_DAQDat_int    <= '0';
            rd_DAQ_FIFO_Word <= '0';
            rd_PM_FIFO_Word  <= '0';
            
            wr_CtrlReg          <= '0';
            wr_trig_lw          <= '0';
            wr_trig_hw          <= '0';        
            wr_Trig_dly_word    <= '0'; 
            rd_CtrlReg          <= '0';
            rd_trig_lw          <= '0';
            rd_trig_hw          <= '0';        
            rd_Trig_dly_word    <= '0'; 
                 
        end case;
      end if;
    end if;
    
  end process adr_decoder;
  
  write2regs: process (clk_i, nReset)
begin
  if nReset = '0' then
    CtrlReg            <= (others => '0');
    TrigWord_LW        <= (others => '0');
    TrigWord_HW        <= (others => '0');
    trig_dly_word      <= (others => '0'); 
  elsif rising_edge(clk_i) then
  
    if wr_CtrlReg = '1' then
      CtrlReg          <= Data_from_SCUB_LA;
    end if;
    
    if wr_trig_lw = '1' then
      TrigWord_LW      <= Data_from_SCUB_LA;
    end if;

    if wr_trig_hw = '1' then
      TrigWord_HW      <= Data_from_SCUB_LA;
    end if;
 
    if wr_trig_dly_word = '1' then
      Trig_dly_word    <= Data_from_SCUB_LA;
    end if;

  end if;
end process write2regs;
  
  
rd_PMDat        <= rd_PMDat_int;   -- to trigger fifo read-outs on level above
rd_DAQDat       <= rd_DAQDat_int;

Rd_Port         <= pmdat                          when rd_PMDat_int     ='1'  and Ena_PM_rd ='1'  else  -- PM Fifo data only valid on stopped PM/Hires DAQ
                   daqdat                         when rd_DAQDat_int    ='1'                      else  -- all channel Rd Port or-ed on level above      
                   max_ch_cnt  & pm_fifo_word     when rd_PM_FIFO_Word  ='1'                      else
                   version_number & daq_fifo_word when rd_DAQ_FIFO_Word ='1'                      else
                   CtrlReg                        when rd_CtrlReg       ='1'                      else  
                   TrigWord_LW                    when rd_trig_lw       ='1'                      else       
                   TrigWord_HW                    when rd_trig_hw       ='1'                      else
                   Trig_dly_word                  when rd_trig_dly_word ='1'                      else
                   x"0000";

user_rd_active  <=    rd_PMDat_int  or rd_DAQDat_int  or rd_PM_FIFO_Word or rd_DAQ_FIFO_Word  or
                      rd_CtrlReg    or rd_trig_lw     or rd_trig_hw      or rd_trig_dly_word     ;      -- all channel user_rd_actives or-ed on level above 
                      
dtack           <=    rd_PMDat_int  or rd_DAQDat_int  or rd_PM_FIFO_Word or rd_DAQ_FIFO_Word  or
                      wr_CtrlReg    or wr_trig_lw     or wr_trig_hw      or wr_trig_dly_word  or 
                      rd_CtrlReg    or rd_trig_lw     or rd_trig_hw      or rd_trig_dly_word     ;  

end architecture daq_chan_reg_logic_arch;
