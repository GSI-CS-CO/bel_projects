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
--  These register are (due to gate count reasons) write-only
--  Register contents  is appended to related daq - packet to make packets self - describing
--   
------------------------------------------------------------------------------------------------------------------------------------


entity daq_chan_wrregs is
generic(
      CtrlReg_adr:          unsigned(15 downto 0):= x"0000";
      trig_lw_adr:          unsigned(15 downto 0):= x"0000";
      trig_hw_adr:          unsigned(15 downto 0):= x"0000";
      trig_dly_word_adr:    unsigned(15 downto 0):= x"0000"
      );

port  (
      -- SCUB interface
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Wr_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:						  in		std_logic;							      -- should be the same clk, used by SCU_Bus_Slave
      nReset:					    in		std_logic;
      
      CtrlReg:            out   std_logic_vector(15 downto 0);
      TrigWord_LW :       out   std_logic_vector(15 downto 0);
      TrigWord_HW:        out   std_logic_vector(15 downto 0);
      Trig_dly_word:      out   std_logic_vector(15 downto 0);
      dtack :             out   std_logic
           
    );
end daq_chan_wrregs;


architecture daq_chan_wrregs_arch of daq_chan_wrregs is



signal wr_CtrlReg           : std_logic;
signal wr_trig_lw           : std_logic;
signal wr_trig_hw           : std_logic;
signal wr_Trig_dly_word     : std_logic;


begin
  
adr_decoder: process (clk_i, nReset)
  begin
    if nReset = '0' then
      wr_CtrlReg          <= '0';
      wr_trig_lw          <= '0';
      wr_trig_hw          <= '0';      
      wr_Trig_dly_word    <= '0'; 
      
    elsif rising_edge(clk_i) then
      wr_CtrlReg          <= '0';
      wr_trig_lw          <= '0';
      wr_trig_hw          <= '0';
      wr_Trig_dly_word    <= '0'; 
    
      if Ext_Adr_Val = '1' then
        case unsigned(Adr_from_SCUB_LA) is
          when CtrlReg_adr =>  
            if Ext_Wr_active = '1' then
              wr_CtrlReg       <= '1';
              dtack            <= '1';
            end if;

          when trig_lw_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_lw       <= '1';
              dtack            <= '1';
            end if;
            
          when trig_hw_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_hw       <= '1';
              dtack            <= '1';
            end if;
 

          when trig_dly_word_adr =>  
            if Ext_Wr_active = '1' then
              wr_trig_dly_word    <= '1';
              dtack               <= '1';
            end if;


          when others =>
            wr_CtrlReg          <= '0';
            wr_trig_lw          <= '0';
            wr_trig_hw          <= '0';        
            wr_Trig_dly_word    <= '0'; 
            dtack               <= '0';
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



end architecture daq_chan_wrregs_arch;