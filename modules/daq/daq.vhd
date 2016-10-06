library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

use work.genram_pkg.all;
use work.daq_pkg.all;


------------------------------------------------------------------------------------------------------------------------------------
--  Vers: 0 Revi: 0: 2015Sep09 K.Kaiser Initial Version    
--        0       1: 2016Sep23 K.Kaiser Concept Change of DIOB requires flexible DAQ Channel count                                                                                                                             --
------------------------------------------------------------------------------------------------------------------------------------


entity daq is
generic (
    Base_addr:          unsigned(15 downto 0):= x"0000";      
    CLK_sys_in_Hz:      integer := 125000000;                 --needed to adjust sample rates
    ch_num:             integer := 4                          --allowed in range of 1 to 16
        );

port  (
      -- SCUB interface
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:			in		std_logic;							      -- '1' => Rd-Cycle is active
      Ext_Wr_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:						  in		std_logic;							      -- should be the same clk, used by SCU_Bus_Slave
      nReset:					    in		std_logic;
      
      --time_stamp        in    std_logic_vector (63 downto 0); --WR Timestamp
      
      user_rd_active:     out   std_logic;
      Rd_Port:            out   std_logic_vector(15 downto 0);-- Data to SCU Bus Macro
      Dtack:              out   std_logic;                    -- Dtack to SCU Bus Macro
      daq_srq:            out   std_logic;                    -- consolidated irq lines from n daq channels for "channel fifo full"
      HiRes_srq:          out   std_logic;                    -- consolidated irq lines from n HiRes channels for "HiRes Daq finished"
      Timing_Pattern_LA:  in    std_logic_vector(31 downto 0);-- latched data from SCU_Bus 
      Timing_Pattern_RCV: in    std_logic;                    -- timing pattern received
      
      --daq input channels
      daq_dat_i:          in    t_daq_dat (1 to ch_num);      -- := (others => dummy_daq_dat_in);
      daq_ext_trig:       in    t_daq_ctl (1 to ch_num)       -- := (others => dummy_daq_ctl_in)  
    );
end daq;


architecture daq_arch of daq is

-- fifo stuff  
  signal we_pg1:                  t_daq_ctl(1 to ch_num); 
  signal rd_pg1:                  t_daq_ctl(1 to ch_num); 
  signal we_pg2:                  t_daq_ctl(1 to ch_num); 
  signal rd_pg2:                  t_daq_ctl(1 to ch_num); 
  signal Pm_we_i:                 t_daq_ctl(1 to ch_num); 
  signal PM_rd_i:                 t_daq_ctl(1 to ch_num); 

  signal fifo_empty_pm:           t_daq_ctl(1 to ch_num); 
  signal fifo_full_pm:            t_daq_ctl(1 to ch_num);
  
  signal fifo_empty_pg1:          t_daq_ctl(1 to ch_num); 
  signal fifo_full_pg1:           t_daq_ctl(1 to ch_num); 
  signal fifo_full_pg1_del:       t_daq_ctl(1 to ch_num); 

  signal fifo_empty_pg2:          t_daq_ctl(1 to ch_num); 
  signal fifo_full_pg2:           t_daq_ctl(1 to ch_num); 
  signal fifo_full_pg2_del:       t_daq_ctl(1 to ch_num); 
  
  signal fifo_full_pulse_pg1:     t_daq_ctl(1 to ch_num); 
  signal fifo_full_pulse_pg2:     t_daq_ctl(1 to ch_num); 
  signal DAQ_FIFO_Overflow:       t_daq_ctl(1 to ch_num);

  signal page1_active:            t_daq_ctl(1 to ch_num);

--Register stuff
  signal CtrlReg_Ch:              t_daq_dat(1 to ch_num);
  signal TrigWord_LW_Ch:          t_daq_dat(1 to ch_num);
  signal TrigWord_HW_Ch:          t_daq_dat(1 to ch_num);
  signal Trig_Dly_word_Ch:        t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);   

  signal PmDat_Ch:                t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);
  signal Ena_PM_rd:               t_daq_ctl(1 to ch_num);
  signal PMDat_rd_pulse:          t_daq_ctl(1 to ch_num);
  signal rd_PMDat:                t_daq_ctl(1 to ch_num);
  signal rd_PMDat_del:            t_daq_ctl(1 to ch_num);
  
  signal DaqDat_Ch:               t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);
  signal DaqDat_pg1:              t_daq_dat(1 to ch_num);
  signal DaqDat_pg2:              t_daq_dat(1 to ch_num);
  signal rd_DAQDat:               t_daq_ctl(1 to ch_num); 
  signal rd_DAQDat_del:           t_daq_ctl(1 to ch_num);  

  signal or_dtack_wr:             t_daq_ctl(1 to ch_num);
  signal or_dtack_rd:             t_daq_ctl(1 to ch_num);  
  signal or_Rd_Port:              t_daq_dat(1 to ch_num);
  signal or_user_rd_active:       t_daq_ctl(1 to ch_num); 

  signal Dtack_ch_wr:             t_daq_ctl(1 to ch_num);
  signal Dtack_ch_rd:             t_daq_ctl(1 to ch_num);
  signal Rd_Port_ch:              t_daq_dat(1 to ch_num);  
  signal user_rd_active_ch:       t_daq_ctl(1 to ch_num);

  
  signal dtack_irqs:              std_logic;
  signal user_rd_active_irqs:     std_logic;
  signal Rd_Port_irqs:            std_logic_vector (15 downto 0);           
  
  
--register bit definitions control register (per channel)
  
  signal Ena_PM:                  t_daq_ctl(1 to ch_num); --bit 0 
  signal Ena_PM_pulse:            t_daq_ctl(1 to ch_num);   
  signal Ena_PM_del:              t_daq_ctl(1 to ch_num);
  signal Sample10us_Ch:           t_daq_ctl(1 to ch_num); --bit 1
  signal Sample100us_Ch:          t_daq_ctl(1 to ch_num); --bit 2
  signal Sample1ms_Ch:            t_daq_ctl(1 to ch_num); --bit 3
  signal Ena_TrigMod:             t_daq_ctl(1 to ch_num); --bit 4
  signal ExtTrig_nEvTrig:         t_daq_ctl(1 to ch_num); --bit 5
  signal Ena_HiRes:               t_daq_ctl(1 to ch_num); --bit 6
  signal Ena_HiRes_pulse:         t_daq_ctl(1 to ch_num);
  signal Ena_HiRes_del:           t_daq_ctl(1 to ch_num);
  signal ExtTrig_nEvTrig_HiRes:   t_daq_ctl(1 to ch_num); --bit 7

--counter stuff
  signal DaqDat_DlyCnt:           t_daq_dat(1 to ch_num);
  signal Daq_dly_cnt_Pg1:         t_daq_dat(1 to ch_num) := (others => dummy_daq_dat_in);  
  signal Daq_dly_cnt_Pg2:         t_daq_dat(1 to ch_num) := (others => dummy_daq_dat_in);
  signal Trig_dly_cnt:            t_daq_dat(1 to ch_num) := (others => dummy_daq_dat_in);
  signal HiRes_trig_cntr:         t_daq_dat(1 to ch_num) := (others => dummy_daq_dat_in);

-- resets, clocks and clock enables  
  signal Reset:                   std_logic;
  signal nSClr_PM_FIFO:           t_daq_ctl(1 to ch_num);
  signal nSClr_DAQ_FIFO:          t_daq_ctl(1 to ch_num);
  
  signal Ena_every_10us:          std_logic;
  signal Ena_every_100us:         std_logic;
  signal Ena_every_1ms:           std_logic;
  signal Ena_every_100us_del:     std_logic;
  signal Ena_every_250ns:         std_logic;
  signal Ena_every_250ns_del:     std_logic;
  signal ChRate:                  t_daq_ctl(1 to ch_num);
  signal ChRate_gated:            t_daq_ctl(1 to ch_num);
  signal HiRes_runs:              t_daq_ctl(1 to ch_num);
  signal HiRes_runs_del:          t_daq_ctl(1 to ch_num);
  
-- input synchronizations 
  signal daq_ext_trig_synched:    t_daq_ctl(1 to ch_num);
  signal daq_dat_i_synched:       t_daq_dat(1 to ch_num);

-- Interrupt Requests to SCU Slave Bus Macro
  signal daq_irq_pulse:           t_daq_ctl(1 to ch_num);
  signal HiRes_IRQ_pulse:         t_daq_ctl(1 to ch_num);
  signal or_daq_srq:              t_daq_ctl(1 to ch_num);
  signal or_HiRes_srq:            t_daq_ctl(1 to ch_num);

  signal HiRes_irq_reg_rd:        std_logic;
  signal daq_irq_reg_rd:          std_logic;
  signal daq_irq_reg:             std_logic_vector (16 downto 1);
  signal HiRes_irq_reg:           std_logic_vector (16 downto 1);
  
  
-- Trigger from Inputs or Timing Events  
  signal Timing_pattern_matched:  t_daq_ctl(1 to ch_num);
  signal Timing_trigger_pulse:    t_daq_ctl(1 to ch_num);
  signal Timing_Pattern_RCV_del:  std_logic; 

  signal daq_ext_trig_del:        t_daq_ctl(1 to ch_num);  
  signal ExtTrig_Pulse:           t_daq_ctl(1 to ch_num);
  
  signal Trig_Pulse:              t_daq_ctl(1 to ch_num);  
  signal Trig_Pulse_HiRes:        t_daq_ctl(1 to ch_num); 
  
  signal Wait_for_Trigger:        t_daq_ctl(1 to ch_num); 
  signal Ena_Sampling:            t_daq_ctl(1 to ch_num);
  
  signal Ena_DAQ:                 t_daq_ctl(1 to ch_num); -- one of Sample enables (10ns/100ns/1ms) is set
  signal Ena_DAQ_del:             t_daq_ctl(1 to ch_num);
  signal Ena_DAQ_Start_pulse:     t_daq_ctl(1 to ch_num); -- resets DAQ Fifos and starts Wait_for_Trigger
  
begin
  
Reset <= not nReset;

Pulses_on_trigger_sources: for I in 1 to ch_num generate

  Timing_pattern_matched(i) <= '1' when Timing_Pattern_LA = (TrigWord_HW_Ch(i) & TrigWord_LW_Ch(i)) else '0';
  -- Timing Trigger fired on received pattern rising edge
  Timing_trigger_pulse(i)   <= Timing_pattern_matched(i) and Timing_Pattern_RCV and not Timing_Pattern_RCV_del;
  -- External Trigger fired on rising edge
  ExtTrig_Pulse(i)          <= daq_ext_trig_synched(i) and not daq_ext_trig_del(i);
  -- Trigger Source selected due to Register bit
  Trig_Pulse(i)             <= ExtTrig_Pulse(i) when ExtTrig_nEvTrig(i) ='1'       else Timing_trigger_pulse(i);
  Trig_Pulse_HiRes(i)       <= ExtTrig_Pulse(i) when ExtTrig_nEvTrig_HiRes(i) ='1' else Timing_trigger_pulse(i);
end generate Pulses_on_trigger_sources;


Trigger_Registered_Logic: for i in 1 to ch_num generate


Ena_DAQ_Start_pulse(i) <= Ena_DAQ(i) and not Ena_DAQ_del(i);

nSClr_DAQ_FIFO(i) <= not (Ena_DAQ_Start_pulse(i) or Reset or DAQ_FIFO_Overflow(i));

PreTrigger_DelayCntr:process (clk_i,nReset)
begin
  if nReset='0' then
    Trig_dly_cnt(i)        <= (others =>'0');
    Wait_for_Trigger(i)    <= '0';
    Ena_Sampling(i)        <= '0';
  elsif rising_edge (clk_i) then
    --Count down with Channel Rate after preload on trigger pulse
    if  (Ena_DAQ_Start_pulse(i)='1' and Ena_TrigMod(i)='1') then
      Trig_dly_cnt(i)<=  Trig_dly_word_Ch (i); -- initial load
    elsif Trig_dly_cnt(i) /= "0000" and ChRate(i)='1' and Wait_for_Trigger(i)  ='0' then
      Trig_dly_cnt(i)<= Trig_dly_cnt(i)- 1; -- count on each tick until zero
    else
      null;                                 --stay 
    end if;

    -- Wait for Trigger after setup of channel
    if Ena_DAQ_Start_pulse(i)='1' and Ena_TrigMod(i)='1' then 
      Wait_for_Trigger(i) <= '1'; 
    elsif Trig_Pulse(i) ='1' or Ena_TrigMod(i)='0' then
      Wait_for_Trigger(i)  <='0'; 
    else
      null;
    end if;
    
    -- Enable Sampling after Pre Trigger Delay is done
    if  Trig_dly_cnt(i) = x"0000" and Ena_TrigMod(i)='1' and Ena_DAQ_Start_pulse(i) ='0'then 
      Ena_Sampling(i) <= '1'; -- start DAQ Sampling until Ena_TrigMod  disables it
    elsif Ena_TrigMod(i) ='0' then
      Ena_Sampling(i) <='0'; 
    else
      null;
    end if;
    
  end if;  
end process PreTrigger_DelayCntr;

end generate Trigger_Registered_Logic;




 Fifo_Data_Muxes: for I in 1 to ch_num generate
  
    -- Provide data of Page which is actually not active
    DaqDat_Ch(i)    <=  DaqDat_pg1(i) when page1_active(i) ='0' else
                        DaqDat_pg2(i) when page1_active(i) ='1' else
                        x"0000";
                                 
    DaqDat_DlyCnt(i)<=  Daq_dly_cnt_Pg1(i) when page1_active(i) ='0' else
                        Daq_dly_cnt_Pg2(i) when page1_active(i) ='1' else
                        x"0000";
end generate Fifo_Data_Muxes;


-- Various Logic needed for pulse generation etc (these are "array of std_logic" signals!)

rd_pulses: process (clk_i, nReset)
begin
  if nReset = '0' then
    rd_PMDat_del            <= (others => '0');
    rd_DAQDat_del           <= (others => '0');
    daq_ext_trig_del        <= (others => '0');
    Ena_every_100us_del     <= '0'; 
    Ena_every_250ns_del     <= '0';
    Timing_Pattern_RCV_del  <= '0';
    Ena_DAQ_del             <= (others => '0'); 
    daq_ext_trig_synched    <= (others => '0');
    daq_dat_i_synched       <= (others => x"0000");
    Ena_HiRes_del           <= (others => '0');
    HiRes_runs_del          <= (others => '0');

  elsif rising_edge(clk_i) then
    rd_PMDat_del            <= rd_PMDat;            --for Fifo read pulse after SCU rd access
    rd_DAQDat_del           <= rd_DAQDat;           --for Fifo read pulse after SCU rd access
    daq_ext_trig_del        <= daq_ext_trig_synched;--for Trigger Pulse on external Trigger input
    Ena_every_100us_del     <= Ena_every_100us;     --for dummy read on PM Fifo full condition
    Ena_every_250ns_del     <= Ena_every_250ns;     --for dummy read on PM Fifo full condition 
    Timing_Pattern_RCV_del  <= Timing_Pattern_RCV;  --for Trigger Pulse on Timing Word
    Ena_DAQ_del             <= Ena_DAQ;             --for pulse on starting DAQ
    daq_ext_trig_synched    <= daq_ext_trig;        --for ext Trigger input synchronization
    daq_dat_i_synched       <= daq_dat_i; 
    Ena_HiRes_del           <= Ena_HiRes;
    HiRes_runs_del          <= HiRes_runs;
  end if;
end process; 


-- Continous Mode:   Sample Rate Selection according to Control Register Setting
-- Single Shot Mode: Before applied to Fifo Sample Rate is gated due to Pre Trigger Delay

-- Trigger Section of DAQ Channels (each channel has separate circuit)
-- Ena_TrigMod Registerbit enables Mode. If not set, Channel works in Continous Mode.
-- Trigger Source can be chosen by ExtTrig_nEvTrig from ExtTrig input or from 32bit Timing Event. 
-- Rising Edge generates Trigger Pulse which can be delayed by trig_dly_cnt.
-- Then Fifo_Full generates interrupt which causes Read Process by SCU.
-- DAQ after Triggering is done until stopped by ENA_DAQ bit.


-- Interrupt are signalled to SCU Slave I/F
-- Fifo Full generates  daq_srq pulse per Channel, which is stored in SCU Slave I/F
-- SCU is requested to empty channel Fifo before next page is filled.
-- With next page filled, next Readout Sequence takes place.
-- In case of DAQ_FIFO_Overflow (both pages full), Fifo logic (both pages) are reset.

Sample_Rates : for I in 1 to ch_num generate

Ena_DAQ(i) <= Sample10us_Ch(i) or Sample100us_Ch(i) or Sample1ms_Ch(i);

ChRate(i) <= Ena_every_1ms     when Sample1ms_Ch(i)   ='1' else      
             Ena_every_100us   when Sample100us_Ch(i) ='1' else 
             Ena_every_10us    when Sample10us_Ch(i)  ='1' else
             '0';

gate_for_ChRate: process (ChRate, Ena_Sampling, Ena_TrigMod)
begin
  if Ena_TrigMod(i)='1' then
    ChRate_gated(i) <= ChRate(i) and Ena_Sampling(i);
  else
    ChRate_gated(i) <= ChRate(i);
  end if;
end process gate_for_ChRate;

end generate Sample_Rates ;
  
 
fifo_logic: for i in 1 to ch_num generate

-- Rd and Wr according to Page Active Conditions  
  
page_cntrl:process (page1_active,rd_DAQDat,ChRate_gated,rd_DAQDat_del)
begin
  if page1_active(i)='1' then
    rd_pg1(i)<= '0'; 
    rd_pg2(i)<= rd_DAQDat_del(i) and not rd_DAQDat(i); --rd on Ch2 allowed
    we_pg1(i)<= ChRate_gated(i);                       --wr on Ch1 allowed
    we_pg2(i)<= '0';
  else  
    rd_pg1(i)<= rd_DAQDat_del(i) and not rd_DAQDat(i); --rd on Ch1 allowed
    rd_pg2(i)<= '0';
    we_pg1(i)<= '0';
    we_pg2(i)<= ChRate_gated(i);                        --wr on Ch2 allowed
  end if;
end process page_cntrl;

-- Determine Page Active Condition on Fifo Full signals
fifo_cntrl: process (clk_i, nReset)
  begin
    if nReset = '0' then
      fifo_full_pg1_del(i) <='0';
      fifo_full_pg2_del(i) <='0'; 
      Ena_PM_del(i)        <='0'; 
      
      page1_active(i)      <='1';  
      DAQ_FIFO_Overflow(i) <='0';      
      Daq_dly_cnt_Pg1(i)   <= (others =>'0');  
      Daq_dly_cnt_Pg2(i)   <= (others =>'0');     
  
    elsif rising_edge(clk_i) then   
      fifo_full_pg1_del(i) <=fifo_full_pg1(i); --for interrupt pulse on Page Full
      fifo_full_pg2_del(i) <=fifo_full_pg2(i); --for interrupt pulse on Page Full
      Ena_PM_del(i)        <= Ena_PM(i);       --to clear the Fifo on PM DAQ start
       

      if    fifo_full_pulse_pg2(i) ='1' or Ena_DAQ_Start_pulse(i)='1' then
        page1_active(i)    <='1';
      elsif fifo_full_pulse_pg1(i) ='1' then     
        page1_active(i)    <='0';
      else
        null ; 
      end if;
      
      if    fifo_full_pg1(i) = '1' and fifo_full_pg2(i) ='1' then
        DAQ_FIFO_Overflow(i)    <='1'; --for reset of both fifo pages
      else
        DAQ_FIFO_Overflow(i)    <='0'; 
      end if;
      
      
      if fifo_full_pg1(i) ='1' and ChRate_gated(i) ='1' then
        Daq_dly_cnt_Pg1(i) <= Daq_dly_cnt_Pg1(i) + 1 ;  -- to count delay between Fifo Full and 1st read
      elsif fifo_empty_pg1(i) ='1' then
        Daq_dly_cnt_Pg1(i) <= (others =>'0');
      end if;
      
      if fifo_full_pg2(i) ='1' and ChRate_gated(i) ='1' then
        Daq_dly_cnt_Pg2(i) <= Daq_dly_cnt_Pg2(i) + 1 ; -- to count delay between Fifo Full and 1st read
      elsif fifo_empty_pg2(i) ='1' then
        Daq_dly_cnt_Pg2(i) <= (others =>'0');
      end if;    
      
  end if;
end process fifo_cntrl;

end generate fifo_logic;


-- Postmortem is enabled with Ena_Pm (4...1) and clocked in with fixed Ena_every_100us until Ena_PM stops
-- Readout only allowed on stopped PM Capture
-- To prevent FifoFull on PM Capture reads one data word before write next data word on a  full fifo.
-- Read is stopped on Ena_PM = 0
-- To allow a write for rare cases where Ena_every_100us coincides with falling edge of Ena_PM
-- PM_we is stopped one clock later with Ena_PM_del to keep a 1024 sample fifo capture

-- Generate two consolidated Interrupt Service Requests to the SCU_Bus_Slave Macro

IRQ_PULSES: for I in 1 to ch_num generate   
  -- IRQ Pulses when HiRes is finished or DAQ Fifo is full
  HiRes_IRQ_pulse(i)     <= '1' when HiRes_runs(i) ='0' and HiRes_runs_del(i)='1' else '0';
  daq_irq_pulse(i)       <=  fifo_full_pulse_pg1(i) or fifo_full_pulse_pg2(i);
  fifo_full_pulse_pg1(i) <= (fifo_full_pg1(i)and not fifo_full_pg1_del(i));
  fifo_full_pulse_pg2(i) <= (fifo_full_pg2(i)and not fifo_full_pg2_del(i));

  -- Register to store pending Interrupts (these two registers are common for all channels)
  
 
  PENDING_IRQ_REGs: process (clk_i, nReset)
  begin
    if nReset = '0' then
      daq_irq_reg (i)   <=  '0';
      HiRes_irq_reg (i) <=  '0';
    elsif rising_edge(clk_i) then  
      if daq_irq_reg_rd = '1' and daq_irq_pulse(i)='0' then      -- no irq should be lost
         daq_irq_reg(i) <= '0';
      elsif daq_irq_pulse(i) ='1' then 
         daq_irq_reg(i) <= '1';
      else
         null;
      end if;
      if HiRes_irq_reg_rd = '1' and HiRes_irq_pulse(i)='0' then  -- no irq should be lost
         HiRes_irq_reg(i) <= '0';
      elsif daq_irq_pulse(i) ='1' then 
         HiRes_irq_reg(i) <= '1';
      else
         null;
      end if;
    end if;
  end process PENDING_IRQ_REGs;
end generate IRQ_PULSES;  

-- Collecting irq pulses of channel 1 to n by large "or" to get 2 consolidated service request lines
                      or_daq_srq(1)   <= daq_irq_pulse(1);
or_daq_srq_gen:       for i in 2 to (ch_num) generate
                        or_daq_srq(i) <= daq_irq_pulse (i) or or_daq_srq(i-1);
                      end generate;
                      daq_srq  <= or_daq_srq (ch_num); 
                      
                      or_HiRes_srq(1)   <= HiRes_IRQ_pulse(1);
or_HiRes_srq_gen:     for i in 2 to (ch_num) generate
                        or_HiRes_srq(i) <= HiRes_IRQ_pulse (i) or or_HiRes_srq(i-1);
                      end generate;
                      HiRes_srq  <= or_HiRes_srq (ch_num); 
                      
                      
                      
------------------------------------------------------------------------------------------------------
                      
PM_Logic : for I in 1 to ch_num generate
  
  Ena_PM_pulse(i)    <=  Ena_PM(i)       and not Ena_PM_del(i);                         --rising_edge
  Ena_HiRes_pulse(i) <=  Ena_HiRes(i)    and not Ena_HiRes_del(i);                      --rising_edge
  PMDat_rd_pulse(i)  <=  rd_PMDat_del(i) and not rd_PMDat(i);                           --falling_edge on SCU Slave Bus Read Accesses
  
  nSClr_PM_FIFO(i) <= not (Ena_PM_pulse(i) or Ena_HiRes_pulse(i) or  Reset);            --Clear on SampleStart
  

  Pm_we_i(i)       <=  ( Ena_every_100us_del and  (Ena_PM(i)     or  Ena_PM_del(i)    ) )       
                        or
                       ( Ena_every_250ns_del and  (HiRes_runs(i) or  HiRes_runs_del(i)) )
                       ;
                       
  Ena_PM_rd(i)     <=  not fifo_empty_pm(i) and not Ena_PM(i) and not Ena_HiRes(i) ;    -- PM Reads only allowed on stopped PM / HiRes whilst PM Fifo not empty

  Pm_rd_i(i)       <=  (Ena_every_100us     and   Ena_PM(i)     and Fifo_full_pm(i)) or -- PM rd in PM Mode to shuffle PM Data  
                       (Ena_every_250ns     and   HiRes_runs(i) and Fifo_full_pm(i))    -- PM rd in HiRes Mode to shuffle HiRes Data ?!
                       or
                       (PMDat_rd_pulse(i) and Ena_PM_rd(i)) 
                       ;
                       

  HiRes_Counter: process (clk_i,nreset)
  begin
    if nreset = '0' then
      HiRes_trig_cntr(i) <=  (others => '0');  
      HiRes_runs(i)<='0';
    elsif rising_edge (clk_i) then
      if Ena_HiRes_pulse(i) ='1' then
        HiRes_trig_cntr(i)    <= (others => '0');
        HiRes_runs(i)  <='1';
      elsif Trig_Pulse_HiRes(i) ='1' or  HiRes_trig_cntr(i) /= x"0000" then
        if HiRes_trig_cntr(i) /=    x"0004" then -- x"0384" then
          HiRes_trig_cntr(i) <= HiRes_trig_cntr(i) + 1;
          HiRes_runs(i)  <='1';
        else
          HiRes_runs(i)  <='0';
        end if;
      end if;
    end if;
  end process HiRes_Counter;

end generate PM_Logic;  
    
 
-- PM FIFO Special High-Res Mode
-- When enabled by HiRes Register bit, the PM Memory (1000 x 16) is used for a single shot hi-res capture.
-- After entering hi-res Mode continous sampling is done.
-- When Trigger Condition is fired , another 900 Samples are done to fill the PM Fifo
-- So there is a post-trigger and a pre-trigger HiRes history of the interesting time range 250µs(samplerate 250ns) available.
-- HiRes Capture stops by itself at sample position 900 after trigger 
-- PM FIFO Special High-Res Mode needs to disable/enable HiRes Register bit for next capture.
 
 
fifo_instances: for i in 1 to ch_num generate
    page1_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 16, 
      g_size                   => 509,
      --g_size                   => 4, --for simulation    
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 2
      )
    port map(
      rst_n_i        => nSClr_DAQ_FIFO(i),
      clk_i          => clk_i,
      d_i            => daq_dat_i_synched(i),
      we_i           => we_pg1(i),
      q_o            => DaqDat_pg1(i),
      rd_i           => rd_pg1 (i),
      empty_o        => fifo_empty_pg1(i),
      full_o         => fifo_full_pg1(i),
      almost_full_o  => open,
      count_o        => open
      );
     
     page2_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 16, 
      g_size                   => 509,
      --g_size                   => 4,  --for simulation           
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 2)
    port map(
      rst_n_i        => nSClr_DAQ_FIFO(i),
      clk_i          => clk_i,
      d_i            => daq_dat_i_synched(i),
      we_i           => we_pg2(i),
      q_o            => DaqDat_pg2(i),
      rd_i           => rd_pg2 (i),
      empty_o        => fifo_empty_pg2(i),
      full_o         => fifo_full_pg2(i),
      almost_full_o  => open,
      count_o        => open
      );     

    postmortem_fifo : generic_sync_fifo

    generic map(
      g_data_width             => 16, 
      g_size                   => 1009,
      --g_size                   => 8,   --for simulation      
      g_show_ahead             => true, --shows the 1st word w/o rd request
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 7)
    port map(
      rst_n_i        => nSClr_PM_FIFO(i),
      clk_i          => clk_i,
      d_i            => daq_dat_i_synched(i),
      we_i           => PM_we_i(i),
      q_o            => PmDat_Ch(i),
      rd_i           => PM_rd_i(i),
      empty_o        => fifo_empty_pm(i),
      full_o         => fifo_full_pm(i),
      almost_full_o  => open,
      count_o        => open
      );
end generate fifo_instances;
  
  -- Time Base
time_base:zeitbasis_daq
  generic map(
      CLK_in_Hz => 125000000,
      diag_on   => 0
      )
  port map (
      Res             => Reset,
      Clk             => clk_i,
      Ena_every_10us  => Ena_every_10us,
      Ena_every_100us => Ena_every_100us,
      Ena_every_1ms   => Ena_every_1ms,
      Ena_every_250ns => Ena_every_250ns
      );
     
-- From here to end Register and Register bit handling is done (exclusively write-only and read-only regs !)
  
-- For better readability CtrlReg_Ch bits are assigned to clear names  
wr_reg_bits:For i in 1 to ch_num generate
  Ena_PM(i)               <= CtrlReg_Ch (i)(0);  -- to stop or run PostMortem Sampling with fixed Sample rate 100us
  Sample10us_Ch(i)        <= CtrlReg_Ch (i)(1);  -- DAQ SampleRate (set only one of this 3 bits) 
  Sample100us_Ch(i)       <= CtrlReg_Ch (i)(2);  -- IF one of DAQ SampleRate bits is set and Trigger Mode is not enabled
  Sample1ms_Ch(i)         <= CtrlReg_Ch (i)(3);  -- then DAQ Sampling is run in continous mode
  Ena_TrigMod(i)          <= CtrlReg_Ch (i)(4);  -- DAQ Trigger Mode: No DAQ sampling until Trig Event occurs
  ExtTrig_nEvTrig(i)      <= CtrlReg_Ch (i)(5);  -- DAQ Trigger Mode fills Fifo with samples until Fifo Full  
  Ena_HiRes(i)            <= CtrlReg_Ch (i)(6);  -- Triggered HiRes SingleShot instead of PM Sampling on PM FIFO
  ExtTrig_nEvTrig_HiRes(i)<= CtrlReg_Ch (i)(7);  -- Trigger Select for HiRes Mode (uses same Trigger sources as DAQ Channel)
end generate;

 
wr_regs:For i in 1 to ch_num generate
  reg_inst:daq_chan_wrregs 
  generic map
    (
    CtrlReg_adr        => CtrlReg_adr_ch(i),
    trig_lw_adr        => trig_lw_adr_ch(i),
    trig_hw_adr        => trig_hw_adr_ch(i),
    trig_dly_word_adr  => trig_dly_word_adr_ch(i)
    )

  port map
    (
    -- SCUB interface
    Adr_from_SCUB_LA   => ADR_from_SCUB_LA,
    Data_from_SCUB_LA  => Data_from_SCUB_LA,  
    Ext_Adr_Val        => Ext_Adr_Val,
    Ext_Wr_active 		 => Ext_Wr_active, 	
    clk_i 						 => clk_i, 
    nReset  				   => nreset, 
    CtrlReg            => CtrlReg_Ch(i),   
    TrigWord_LW        => TrigWord_LW_Ch(i),
    TrigWord_HW        => TrigWord_HW_Ch(i),
    Trig_dly_word      => Trig_Dly_word_Ch(i),
    Dtack              => dtack_ch_wr(i)   
    );
end generate;
 

 rd_logic:For i in 1 to ch_num generate
  rd_logic_inst:daq_chan_rd_logic 
  generic map
    (
      PmDat_adr          => PMDat_adr_ch(i),
      DaqDat_adr         => DAQDat_adr_ch(i)
    )

  port map
    (
      Adr_from_SCUB_LA   => ADR_from_SCUB_LA,
      Data_from_SCUB_LA  => Data_from_SCUB_LA,  
      Ext_Adr_Val        => Ext_Adr_Val,
      Ext_Rd_active 		 => Ext_Rd_active, 	
      clk_i 						 => clk_i, 
      nReset  				   => nreset,
      
      PmDat              => PmDat_Ch(i),
      DaqDat             => DaqDat_Ch(i),
      Ena_PM_rd          => Ena_PM_rd(i),
      
      Rd_Port            => Rd_Port_Ch(i),
      user_rd_active     => user_rd_active_ch(i),
      rd_PMDat           => rd_PMDat(i),
      rd_DAQDat          => rd_DAQDat(i),
      dtack              => dtack_ch_rd(i)
    );
end generate;

-- Generate consolidated DAQ macro Rd_Port by an large "or" of all channel Rd_Port signals
-- (not addressed channel Rd_Ports deliver 0x0 when not accessed)

                      or_dtack_wr(1)         <= dtack_ch_wr(1);
                      or_dtack_rd(1)         <= dtack_ch_rd(1);
                      or_user_rd_active(1)   <= user_rd_active_ch(1);
                      or_Rd_Port(1)          <= Rd_Port_Ch(1);
big_or_gate_gen:      for i in 2 to (ch_num) generate
                        or_dtack_wr(i)       <= dtack_ch_wr(2) or or_dtack_wr (i-1);
                        or_dtack_rd(i)       <= dtack_ch_rd(2) or or_dtack_rd (i-1);
                        or_user_rd_active(i) <= user_rd_active_ch(2) or or_user_rd_active (i-1);
                        or_Rd_Port(i)        <= Rd_Port_Ch (i) or or_Rd_Port(i-1);
                      end generate;
                      dtack          <= or_dtack_rd (ch_num) or or_dtack_wr (ch_num) or dtack_irqs ;
                      user_rd_active <= or_user_rd_active (ch_num) or user_rd_active_irqs ;
                      Rd_Port        <= or_Rd_Port (ch_num) or Rd_Port_irqs ;

-- Both Service request regs are not channel specific therefore placed separately here
adr_decoder: process (clk_i, nReset)
  begin
    if nReset = '0' then
      daq_irq_reg_rd           <= '0';
      HiRes_irq_reg_rd         <= '0';   
    elsif rising_edge(clk_i) then
      daq_irq_reg_rd           <= '0';
      HiRes_irq_reg_rd         <= '0';

      if Ext_Adr_Val = '1' then
        case unsigned(Adr_from_SCUB_LA) is
 
          when HiRes_irq_reg_adr =>  
            if Ext_Rd_active =  '1' then
              HiRes_irq_reg_rd <= '1';
            end if;
            
          when daq_irq_reg_adr =>  
            if Ext_Rd_active =  '1' then
              daq_irq_reg_rd   <= '1';
            end if;            
 
          when others =>
            daq_irq_reg_rd     <= '0';
            HiRes_irq_reg_rd   <= '0';
            
        end case;
      end if;
    end if;
    
  end process adr_decoder;
  
Rd_Port_irqs     <= daq_irq_reg         when daq_irq_reg_rd    ='1'  else  
                    HiRes_irq_reg       when HiRes_irq_reg_rd  ='1'  else          
                    x"0000";

user_rd_active_irqs  <=    HiRes_irq_reg_rd or daq_irq_reg_rd  ;                          
dtack_irqs           <=    HiRes_irq_reg_rd or daq_irq_reg_rd   ; 
                 

end architecture daq_arch;