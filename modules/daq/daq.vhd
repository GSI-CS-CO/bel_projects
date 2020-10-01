library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

use work.genram_pkg.all;
use work.daq_pkg.all;


------------------------------------------------------------------------------------------------------------------------------------
--  Vers: 0 Revi: 0: 2015Sep09 K.Kaiser Initial Version
--        0       1: 2016Sep23 K.Kaiser Concept Change of DIOB requires flexible DAQ Channel count
--                2: 2016Okt07 K.Kaiser Add Descriptor to each DAQ Data Packet
--                3: 2017Nov20 K.Kaiser Added used Fifowords, Register readback and changed Interrupt Register handling
--                4: 2019Mar26 K.Kaiser HiRes_irq_pulse Bugfix (copypaste error)
--                                      DAQ Fifo Size changed back from 500 to 502 Samples
--                                      HiRes Timestamp latch now on HiRes Trigger Event, not on HiRes Switch off
--                                      For Signaltaps better change Samplerate to max than shorten FIFOs
--                                      crc_daq_out, HiRes_Runs and wait_for_trigger  bugfix
--
------------------------------------------------------------------------------------------------------------------------------------


entity daq is
generic (
    Base_addr:          unsigned(15 downto 0):= x"0000";
    CLK_sys_in_Hz:      integer := 125_000_000;               --needed to adjust sample rates
    ch_num:             integer := 1                          --allowed in range of 1 to 16
        );

port  (
      -- SCUB interface
      Adr_from_SCUB_LA:    in    std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:   in    std_logic_vector(15 downto 0);-- latched data from SCU_Bus
      Ext_Adr_Val:         in    std_logic;                    -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:       in    std_logic;                     -- '1' => Rd-Cycle is active
      Ext_Wr_active:       in    std_logic;                    -- '1' => Wr-Cycle is active
      clk_i:               in    std_logic;                    -- should be the same clk, used by SCU_Bus_Slave
      nReset:              in    std_logic;

      diob_extension_id:   in    std_logic_vector(15 downto 0);-- hard-coded ID Value for DAQ Diob implementation

      user_rd_active:      out   std_logic;
      Rd_Port:             out   std_logic_vector(15 downto 0);-- Data to SCU Bus Macro
      Dtack:               out   std_logic;                    -- Dtack to SCU Bus Macro
      daq_srq:             out   std_logic;                    -- consolidated irq lines from n daq channels for "channel fifo full"
      HiRes_srq:           out   std_logic;                    -- consolidated irq lines from n HiRes channels for "HiRes Daq finished"
      Timing_Pattern_LA:   in    std_logic_vector(31 downto 0);-- latched data from SCU_Bus
      Timing_Pattern_RCV:  in    std_logic;                    -- timing pattern received

      --daq input channels
      daq_dat_i:           in    t_daq_dat (1 to ch_num);      -- := (others => dummy_daq_dat_in);
      daq_ext_trig:        in    t_daq_ctl (1 to ch_num)       -- := (others => dummy_daq_ctl_in)
    );
end daq;


architecture daq_arch of daq is

  constant version_number:           std_logic_vector (15 downto 9):= (b"0000_000"); --Version 0 : Pre-Release (bis zur 1. SW Inbetriebnahme)
  signal   max_ch_cnt:               std_logic_vector (15 downto 10);
  --common registers for all channels
  constant daq_irq_reg_adr:          unsigned(15 downto 0):= (Base_addr + x"0060" );
  constant HiRes_irq_reg_adr:        unsigned(15 downto 0):= (Base_addr + x"0061" );
  constant timestamp_cntr_word1_adr: unsigned(15 downto 0):= (Base_addr + x"0062" );
  constant timestamp_cntr_word2_adr: unsigned(15 downto 0):= (Base_addr + x"0063" );
  constant timestamp_cntr_word3_adr: unsigned(15 downto 0):= (Base_addr + x"0064" );
  constant timestamp_cntr_word4_adr: unsigned(15 downto 0):= (Base_addr + x"0065" );
  constant timestamp_cntr_tag_lw_adr:unsigned(15 downto 0):= (Base_addr + x"0066" );
  constant timestamp_cntr_tag_hw_adr:unsigned(15 downto 0):= (Base_addr + x"0067" );
  constant timestamp_cntr_increment : integer := 1_000_000_000 / CLK_sys_in_Hz;

  --specific register (one per channel)
  type adr_ch_array is array (1 to 16) of unsigned(15 downto 0); --Channelcount is 1 to 16 !

  constant  CtrlReg_adr_ch:        adr_ch_array         :=  (
                                                            Base_addr + x"0000",
                                                            Base_addr + x"0001",
                                                            Base_addr + x"0002",
                                                            Base_addr + x"0003",
                                                            Base_addr + x"0004",
                                                            Base_addr + x"0005",
                                                            Base_addr + x"0006",
                                                            Base_addr + x"0007",
                                                            Base_addr + x"0008",
                                                            Base_addr + x"0009",
                                                            Base_addr + x"000a",
                                                            Base_addr + x"000b",
                                                            Base_addr + x"000c",
                                                            Base_addr + x"000d",
                                                            Base_addr + x"000e",
                                                            Base_addr + x"000f"
                                                           );
  constant  trig_lw_adr_ch:        adr_ch_array        :=  (
                                                            Base_addr + x"0010",
                                                            Base_addr + x"0011",
                                                            Base_addr + x"0012",
                                                            Base_addr + x"0013",
                                                            Base_addr + x"0014",
                                                            Base_addr + x"0015",
                                                            Base_addr + x"0016",
                                                            Base_addr + x"0017",
                                                            Base_addr + x"0018",
                                                            Base_addr + x"0019",
                                                            Base_addr + x"001a",
                                                            Base_addr + x"001b",
                                                            Base_addr + x"001c",
                                                            Base_addr + x"001d",
                                                            Base_addr + x"001e",
                                                            Base_addr + x"001f"
                                                           );
  constant  trig_hw_adr_ch:        adr_ch_array        :=  (
                                                            Base_addr + x"0020",
                                                            Base_addr + x"0021",
                                                            Base_addr + x"0022",
                                                            Base_addr + x"0023",
                                                            Base_addr + x"0024",
                                                            Base_addr + x"0025",
                                                            Base_addr + x"0026",
                                                            Base_addr + x"0027",
                                                            Base_addr + x"0028",
                                                            Base_addr + x"0029",
                                                            Base_addr + x"002a",
                                                            Base_addr + x"002b",
                                                            Base_addr + x"002c",
                                                            Base_addr + x"002d",
                                                            Base_addr + x"002e",
                                                            Base_addr + x"002f"
                                                           );
  constant  trig_dly_word_adr_ch:  adr_ch_array      :=(
                                                            Base_addr + x"0030",
                                                            Base_addr + x"0031",
                                                            Base_addr + x"0032",
                                                            Base_addr + x"0033",
                                                            Base_addr + x"0034",
                                                            Base_addr + x"0035",
                                                            Base_addr + x"0036",
                                                            Base_addr + x"0037",
                                                            Base_addr + x"0038",
                                                            Base_addr + x"0039",
                                                            Base_addr + x"003a",
                                                            Base_addr + x"003b",
                                                            Base_addr + x"003c",
                                                            Base_addr + x"003d",
                                                            Base_addr + x"003e",
                                                            Base_addr + x"003f"
                                                          );
  constant  PMDat_adr_ch:          adr_ch_array      :=(
                                                            Base_addr + x"0040",
                                                            Base_addr + x"0041",
                                                            Base_addr + x"0042",
                                                            Base_addr + x"0043",
                                                            Base_addr + x"0044",
                                                            Base_addr + x"0045",
                                                            Base_addr + x"0046",
                                                            Base_addr + x"0047",
                                                            Base_addr + x"0048",
                                                            Base_addr + x"0049",
                                                            Base_addr + x"004a",
                                                            Base_addr + x"004b",
                                                            Base_addr + x"004c",
                                                            Base_addr + x"004d",
                                                            Base_addr + x"004e",
                                                            Base_addr + x"004f"
                                                           );
  constant  DaqDat_adr_ch:        adr_ch_array      :=(
                                                            Base_addr + x"0050",
                                                            Base_addr + x"0051",
                                                            Base_addr + x"0052",
                                                            Base_addr + x"0053",
                                                            Base_addr + x"0054",
                                                            Base_addr + x"0055",
                                                            Base_addr + x"0056",
                                                            Base_addr + x"0057",
                                                            Base_addr + x"0058",
                                                            Base_addr + x"0059",
                                                            Base_addr + x"005a",
                                                            Base_addr + x"005b",
                                                            Base_addr + x"005c",
                                                            Base_addr + x"005d",
                                                            Base_addr + x"005e",
                                                            Base_addr + x"005f"
                                                           );



  constant  DAQ_FIFO_Words_adr_ch:        adr_ch_array      :=(
                                                            Base_addr + x"0070",
                                                            Base_addr + x"0071",
                                                            Base_addr + x"0072",
                                                            Base_addr + x"0073",
                                                            Base_addr + x"0074",
                                                            Base_addr + x"0075",
                                                            Base_addr + x"0076",
                                                            Base_addr + x"0077",
                                                            Base_addr + x"0078",
                                                            Base_addr + x"0079",
                                                            Base_addr + x"007a",
                                                            Base_addr + x"007b",
                                                            Base_addr + x"007c",
                                                            Base_addr + x"007d",
                                                            Base_addr + x"007e",
                                                            Base_addr + x"007f"
                                                           );


  constant  PM_FIFO_Words_adr_ch:        adr_ch_array      :=(
                                                            Base_addr + x"0080",
                                                            Base_addr + x"0081",
                                                            Base_addr + x"0082",
                                                            Base_addr + x"0083",
                                                            Base_addr + x"0084",
                                                            Base_addr + x"0085",
                                                            Base_addr + x"0086",
                                                            Base_addr + x"0087",
                                                            Base_addr + x"0088",
                                                            Base_addr + x"0089",
                                                            Base_addr + x"008a",
                                                            Base_addr + x"008b",
                                                            Base_addr + x"008c",
                                                            Base_addr + x"008d",
                                                            Base_addr + x"008e",
                                                            Base_addr + x"008f"
                                                           );


-- fifo stuff
  signal we_pg1:                  t_daq_ctl(1 to ch_num);
  signal rd_pg1:                  t_daq_ctl(1 to ch_num);
  signal we_pg2:                  t_daq_ctl(1 to ch_num);
  signal rd_pg2:                  t_daq_ctl(1 to ch_num);
  signal Pm_we_i:                 t_daq_ctl(1 to ch_num);
  signal PM_rd_i:                 t_daq_ctl(1 to ch_num);

  signal fifo_empty_pm:           t_daq_ctl(1 to ch_num);
  signal fifo_full_pm:            t_daq_ctl(1 to ch_num);


  signal fifo_full_pg1:           t_daq_ctl(1 to ch_num);
  signal fifo_empty_pg1:          t_daq_ctl(1 to ch_num);
  signal fifo_full_pg1_del:       t_daq_ctl(1 to ch_num);


  signal fifo_full_pg2:           t_daq_ctl(1 to ch_num);
  signal fifo_empty_pg2:          t_daq_ctl(1 to ch_num);
  signal fifo_full_pg2_del:       t_daq_ctl(1 to ch_num);

  signal fifo_full_pulse_pg1:     t_daq_ctl(1 to ch_num);
  signal fifo_full_pulse_pg2:     t_daq_ctl(1 to ch_num);
  signal DAQ_FIFO_Overflow:       t_daq_ctl(1 to ch_num);

  signal page1_active:            t_daq_ctl(1 to ch_num);

  type   t_pagefifowords  is array(natural range <> ) of std_logic_vector(8 downto 0);
  type   t_pmfifowords    is array(natural range <> ) of std_logic_vector(9 downto 0);

  signal page1_fifo_words:        t_pagefifowords(1 to ch_num);
  signal page2_fifo_words:        t_pagefifowords(1 to ch_num);
  signal daq_fifo_words:          t_pagefifowords(1 to ch_num);
  signal pm_fifo_words:           t_pmfifowords  (1 to ch_num);


--Register stuff

  signal CtrlReg_Ch:              t_daq_dat(1 to ch_num);
  signal TrigWord_LW_Ch:          t_daq_dat(1 to ch_num);
  signal TrigWord_HW_Ch:          t_daq_dat(1 to ch_num);
  signal Trig_Dly_word_Ch:        t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);

  signal PmDat_Ch:                t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);
  signal Ena_PM_rd:               t_daq_ctl(1 to ch_num);
  signal PMDat_rd_pulse:          t_daq_ctl(1 to ch_num);
  signal DAQDat_rd_pulse:         t_daq_ctl(1 to ch_num);
  signal rd_PMDat:                t_daq_ctl(1 to ch_num);
  signal rd_PMDat_del:            t_daq_ctl(1 to ch_num);

  signal DaqDat_Ch:               t_daq_dat(1 to ch_num):= (others => dummy_daq_dat_in);
  signal DaqDat_pg1:              t_daq_dat(1 to ch_num);
  signal DaqDat_pg2:              t_daq_dat(1 to ch_num);
  signal rd_DAQDat:               t_daq_ctl(1 to ch_num);
  signal rd_DAQDat_del:           t_daq_ctl(1 to ch_num);


  signal Dtack_ch:                t_daq_ctl(1 to 16);
  signal Rd_Port_ch:              t_daq_dat(1 to 16);
  signal user_rd_active_ch:       t_daq_ctl(1 to 16);


  signal dtack_cn_regs:           std_logic;
  signal user_rd_active_cn_regs:  std_logic;
  signal Rd_Port_common_regs:     std_logic_vector (15 downto 0);


--register bit definitions control register (per channel)

  signal Ena_PM:                  t_daq_ctl(1 to ch_num); --bit 0
  signal Start_PM_pulse:          t_daq_ctl(1 to ch_num);
  signal Ena_PM_del:              t_daq_ctl(1 to ch_num);
  signal Sample10us_Ch:           t_daq_ctl(1 to ch_num); --bit 1
  signal Sample100us_Ch:          t_daq_ctl(1 to ch_num); --bit 2
  signal Sample1ms_Ch:            t_daq_ctl(1 to ch_num); --bit 3
  signal Ena_TrigMod:             t_daq_ctl(1 to ch_num); --bit 4
  signal ExtTrig_nEvTrig:         t_daq_ctl(1 to ch_num); --bit 5
  signal Ena_HiRes:               t_daq_ctl(1 to ch_num); --bit 6
  signal Start_HiRes_pulse:       t_daq_ctl(1 to ch_num);
  signal Ena_HiRes_del:           t_daq_ctl(1 to ch_num);
  signal ExtTrig_nEvTrig_HiRes:   t_daq_ctl(1 to ch_num); --bit 7

--counter stuff
  signal DaqDat_DlyCnt:           t_daq_dat(1 to ch_num);
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

-- descriptor for DAQ
  signal almost_full_pg1:         t_daq_ctl(1 to ch_num);
  signal almost_full_pg2:         t_daq_ctl(1 to ch_num);

  signal almost_full_pg1_del:     t_daq_ctl(1 to ch_num);
  signal almost_full_pg2_del:     t_daq_ctl(1 to ch_num);

  signal almost_full_pulse_pg1:   t_daq_ctl(1 to ch_num);
  signal almost_full_pulse_pg2:   t_daq_ctl(1 to ch_num);

  signal DAQ_Descr_runs:          t_daq_ctl(1 to ch_num);
  signal DAQ_Descr_data:          t_daq_dat(1 to ch_num);

  signal DAQ_Timestamp_latched:   t_daq_ts(1 to ch_num);
  signal DAQ_Descr_cntr:          t_daq_dctr(1 to ch_num);
  signal DAQ_fifo_data_pg1:       t_daq_dat(1 to ch_num);
  signal DAQ_fifo_data_pg2:       t_daq_dat(1 to ch_num);

 -- descriptor for PM/HiRes Mode
  signal Stop_PM_pulse:           t_daq_ctl(1 to ch_num);
  signal Stop_HiRes_pulse:        t_daq_ctl(1 to ch_num);

  signal PM_Descr_runs:           t_daq_ctl(1 to ch_num);
  signal Last_mode_was_PM:        t_daq_ctl(1 to ch_num);
  signal Last_mode_was_HiRes:     t_daq_ctl(1 to ch_num);
  signal PM_Descr_data:           t_daq_dat(1 to ch_num);

  signal PM_Timestamp_latched:    t_daq_ts(1 to ch_num);
  signal PM_Descr_cntr:           t_daq_dctr(1 to ch_num);
  signal PM_Descr_reached_7:      t_daq_ctl(1 to ch_num);
  signal PM_fifo_data:            t_daq_dat(1 to ch_num);

-- input synchronizations
  signal daq_ext_trig_synched:    t_daq_ctl(1 to ch_num);
  signal daq_dat_i_synched:       t_daq_dat(1 to ch_num);

-- Interrupt Requests to SCU Slave Bus Macro
  signal daq_irq_pulse:           t_daq_ctl(1 to ch_num);
  signal HiRes_IRQ_pulse:         t_daq_ctl(1 to ch_num);
  signal or_daq_srq:              t_daq_ctl(1 to ch_num);
  signal or_HiRes_srq:            t_daq_ctl(1 to ch_num);

  signal HiRes_irq_reg_rd:        std_logic;
  signal HiRes_irq_reg_wr:        std_logic;
  signal daq_irq_reg_rd:          std_logic;
  signal daq_irq_reg_wr:          std_logic;
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
  signal Ena_DAQ_Stopp_pulse:     t_daq_ctl(1 to ch_num); -- stopps Sampling


-- Timestamp Counter
  signal timestamp_cntr:             std_logic_vector (63 downto 0);
  signal timestamp_cntr_preset:      std_logic_vector (63 downto 0);
  signal timestamp_cntr_word1_wr:    std_logic;
  signal timestamp_cntr_word2_wr:    std_logic;
  signal timestamp_cntr_word3_wr:    std_logic;
  signal timestamp_cntr_word4_wr:    std_logic;
  signal timestamp_cntr_word1_rd:    std_logic;
  signal timestamp_cntr_word2_rd:    std_logic;
  signal timestamp_cntr_word3_rd:    std_logic;
  signal timestamp_cntr_word4_rd:    std_logic;
-- Timestamp Counter Tag
  signal timestamp_cntr_tag_matched: std_logic;
  signal timestamp_cntr_tag_pulse:   std_logic;
  signal timestamp_cntr_tag_lw_rd:   std_logic;
  signal timestamp_cntr_tag_hw_rd:   std_logic;
  signal timestamp_cntr_tag_lw_wr:   std_logic;
  signal timestamp_cntr_tag_hw_wr:   std_logic;
  signal timestamp_cntr_tag:         std_logic_vector (31 downto 0);

-- For clear and trigger crc calculation and append it to packet as last word
  signal crc_pm_en_pulse:            t_daq_ctl(1 to ch_num); --enables crc calculation (width of pulse=1 clk_i)
  signal crc_daq_en_pulse:           t_daq_ctl(1 to ch_num);
  signal crc_pm_start_pulse:         t_daq_ctl(1 to ch_num); --clears crc block on start of crc calculation
  signal crc_daq_start_pulse:        t_daq_ctl(1 to ch_num);

  signal crc_daq_was_read:           t_daq_ctl(1 to ch_num);
  signal crc_pm_was_read:            t_daq_ctl(1 to ch_num);
  signal crc_pm_data_in:             t_daq_dat(1 to ch_num);
  signal crc_daq_data_in:            t_daq_dat(1 to ch_num);
  --signal crc_daq_word:               t_daq_dat(1 to ch_num);
  signal crc_pm_out:                 t_daq_dat(1 to ch_num);
  signal crc_daq_out:                t_daq_dat(1 to ch_num);
  signal PMDat_to_SCUB:              t_daq_dat(1 to ch_num); --switches Fifo dat or crc data to SCUB
----------------------------------------------------------------------------------------------------------------

begin

Reset <= not nReset;

---------------------------- Various Logic for incoming Trigger ------------------------------------------------

Pulses_on_trigger_sources: for I in 1 to ch_num generate

  Timing_pattern_matched(i) <= '1' when Timing_Pattern_LA = (TrigWord_HW_Ch(i) & TrigWord_LW_Ch(i)) else '0';
  -- Timing Trigger fired on received pattern rising edge
  Timing_trigger_pulse(i)   <= Timing_pattern_matched(i) and     Timing_Pattern_RCV  and not Timing_Pattern_RCV_del;
  -- External Trigger fired on rising edge
  ExtTrig_Pulse(i)          <= daq_ext_trig_synched(i)   and not daq_ext_trig_del(i);
  -- Trigger Source selected due to Register bit
  Trig_Pulse(i)             <= ExtTrig_Pulse(i) when ExtTrig_nEvTrig(i) ='1'       else Timing_trigger_pulse(i);
  Trig_Pulse_HiRes(i)       <= ExtTrig_Pulse(i) when ExtTrig_nEvTrig_HiRes(i) ='1' else Timing_trigger_pulse(i);
end generate Pulses_on_trigger_sources;


Trigger_Registered_Logic : FOR i IN 1 TO ch_num GENERATE
  Ena_DAQ_Start_pulse(i) <=     Ena_DAQ(i) AND NOT Ena_DAQ_del(i);
  Ena_DAQ_Stopp_pulse(i) <= NOT Ena_DAQ(i) AND     Ena_DAQ_del(i);
  nSClr_DAQ_FIFO(i)      <= NOT (Ena_DAQ_Start_pulse(i) OR Reset OR DAQ_FIFO_Overflow(i));

  PreTrigger_DelayCntr : PROCESS (clk_i, nReset)
  BEGIN
    IF nReset = '0' THEN
      Trig_dly_cnt(i)     <= (OTHERS => '0');
      Wait_for_Trigger(i) <= '0';
      Ena_Sampling(i)     <= '0';
    ELSIF rising_edge (clk_i) THEN
      --Count down with Channel Rate after preload on trigger pulse
      IF (Ena_DAQ_Start_pulse(i) = '1' AND Ena_TrigMod(i) = '1') THEN
        Trig_dly_cnt(i) <= Trig_dly_word_Ch (i);      -- initial load
      ELSIF Trig_dly_cnt(i) /= "0000" AND ChRate(i) = '1' AND Wait_for_Trigger(i) = '0' THEN
        Trig_dly_cnt(i) <= Trig_dly_cnt(i) - 1;       -- count on each tick until zero
      ELSE
        NULL;                                         --stay
      END IF;

      -- Wait for Trigger after setup of channel
      IF Ena_DAQ_Start_pulse(i) = '1' AND Ena_TrigMod(i) = '1' THEN
        Wait_for_Trigger(i) <= '1';
      ELSIF Trig_Pulse(i) = '1' OR Ena_TrigMod(i) = '0' THEN
        Wait_for_Trigger(i) <= '0';
      ELSE
        NULL;
      END IF;

      -- Enable Sampling
      IF Ena_TrigMod(i) = '1' THEN           -- Trigger Modus

        IF Wait_for_Trigger(i) = '1' THEN
          Ena_Sampling(i) <= '0';            -- waiting for trigger event
        ELSE
          IF Trig_dly_cnt(i) /= "0000" THEN
            Ena_Sampling(i) <= '0';          -- waiting for triggercounter
          ELSE
            Ena_Sampling(i) <= '1';          -- Condition fulfilled
          END IF;
        END IF;

      ELSIF Ena_TrigMod(i) = '0' THEN        -- Non Triggered Modus

        IF Ena_DAQ_Start_pulse(i) = '1' THEN
          Ena_Sampling(i) <= '1';
        ELSE
          NULL;
        END IF ;

      ELSIF Ena_DAQ_Stopp_pulse(i) = '1' THEN  -- Stopp Condition
        Ena_Sampling(i) <= '0';

      ELSE
        NULL;
      END IF ;
    END IF;
  END PROCESS PreTrigger_DelayCntr;
END GENERATE Trigger_Registered_Logic;


Fifo_Data_Muxes: for I in 1 to ch_num generate

    -- Provide data of Page which is actually not active
    DaqDat_Ch(i)      <=  DaqDat_pg1(i)   when (page1_active(i) ='0' and fifo_empty_pg1(i) ='0'                            )else  --read from inactive fifo1  as long as not empty
                         -- crc_daq_word(i) when (page1_active(i) ='0' and fifo_empty_pg1(i) ='1' and crc_daq_was_read(i)='0')else  --read from crc_daq as long not read
                          crc_daq_out(i) when (page1_active(i) ='0' and fifo_empty_pg1(i) ='1' and crc_daq_was_read(i)='0')else  --read from crc_daq as long not read
                          DaqDat_pg2(i)   when (page1_active(i) ='1' and fifo_empty_pg2(i) ='0'                            )else  --read from inactive fifo2 as long as not empty
                          crc_daq_out(i)  when (page1_active(i) ='1' and fifo_empty_pg2(i) ='1' and crc_daq_was_read(i)='0')else  --read from crc_daq as long not read
                          x"0000";

    daq_fifo_words(i) <=  page1_fifo_words(i) when page1_active(i) ='0' else
                          page2_fifo_words(i) when page1_active(i) ='1' else
                          b"0_0000_0000";

end generate Fifo_Data_Muxes;


---------------------- Various Logic needed for pulse generation etc (these are "array of std_logic" signals!)-------------

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


-- Some Remarks on DAQ Modes (Continous and Trigger Mode)
-- All modes are working on DAQ FIFOs with 1 out of 3 sample rates

-- Continous Mode:   Sampling starts after enabling one of 3 sample rates whilst Ena_TrigMod=0
--                   Stopped by clearing the sample rate

-- Trigger Mode:     Sampling starts after Trigger delay on selected sample rate  and Ena_TrigMod set to 1
--                   Before set Ena_TrigMod to 1, set the trigger condition (Trigger Delay, ExtTrig_nEvTrig Bit )
--                   Stopped by clearing the sample rate and clearing Ena_TrigMod bit.

-- DAQ and HiRes uses the same trigger inputs. This has some restrictions:
--  * When both use the SCU Bus Timing Event as a trigger,  the same timing event for both (DAQ and HiRes) is used.
--  * When both use the external input daq_ext_trig as a trigger, it is the same input signal
--  * It is possible that one use the timing event and the other use the external input daq_ext_trig

-- The DAQ Channel trigger can be delayed by 2exp16 samples by Trig_dly_cnt.
-- Rising Edge of trigger event generates a pulse, which starts the trig_dly_cnt.

-- DAQ Fifo Full generates  daq_srq pulse
-- SCU is requested to empty DAQ Fifo before next page is filled.
-- In case of DAQ_FIFO_Overflow(both pages full) and fifo was not emptied , both fifo pages are reset by logic

----------------------------------- Generate Sample Rates for DAQ Channels------------------------------------
Sample_Rates : for I in 1 to ch_num generate
  Ena_DAQ(i)  <=  Sample10us_Ch(i) or Sample100us_Ch(i) or Sample1ms_Ch(i);
  ChRate(i)   <=  Ena_every_1ms     when Sample1ms_Ch(i)   ='1' else
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

-------------------------------------DAQ FIFO Page Control (active and shadow page)----------------------------
fifo_logic: for i in 1 to ch_num generate

-- Rd allowed when page passive and DAQ_Descr_runs finished
-- Wr allowed on active page for data aquisition and on passive page for DAQ_Descr Writes
-- rd_DAQDat generated from SCU Bus fifo read access
DAQDat_rd_pulse(i) <= rd_DAQDat_del(i) and not rd_DAQDat(i);

page_access_cntrl:process (page1_active,rd_DAQDat,ChRate_gated,rd_DAQDat_del,DAQ_Descr_runs)
  --todo: Improve case when DAQ_Descr_runs and page changes (should never occur)
  --todo: Maybe set some error bit in Descriptor Header for "false read operation"

  begin
    if page1_active(i)='1' then                            --page 1 active

      rd_pg1(i)<= '0';                                     --active page
      we_pg1(i)<= ChRate_gated(i);

      if DAQ_Descr_runs(i) ='1' then                       --passive page
        rd_pg2(i)<= '0';
        we_pg2(i)<= DAQ_Descr_runs(i);
      else
        rd_pg2(i)<= DAQDat_rd_pulse(i);                    --readout next fifo word after SCUB access
        we_pg2(i)<= '0';
      end if;
    else                                                   --page 2 active

      rd_pg2(i)<= '0';                                     --active page
      we_pg2(i)<= ChRate_gated(i);

      if DAQ_Descr_runs(i) ='1' then                       --passive page
        rd_pg1(i)<= '0';
        we_pg1(i)<= DAQ_Descr_runs(i);
      else
        rd_pg1(i)<= DAQDat_rd_pulse(i);                    --readout next fifo word after SCUB access
        we_pg1(i)<= '0';
      end if;

    end if;
  end process page_access_cntrl;


-- Determine Page Active Condition on Fifo Full signals
fifo_cntrl: process (clk_i, nReset)
  begin
    if nReset = '0' then
      fifo_full_pg1_del(i)    <='0';
      fifo_full_pg2_del(i)    <='0';
      almost_full_pg1_del(i)  <='0';
      almost_full_pg2_del(i)  <='0';
      Ena_PM_del(i)           <='0';

      page1_active(i)         <='1';
      DAQ_FIFO_Overflow(i)    <='0';

    elsif rising_edge(clk_i) then
      fifo_full_pg1_del(i)    <=fifo_full_pg1(i);   --for interrupt pulse on Page Full
      fifo_full_pg2_del(i)    <=fifo_full_pg2(i);   --for interrupt pulse on Page Full

      almost_full_pg1_del(i)  <=almost_full_pg1(i); --for daq page swap
      almost_full_pg2_del(i)  <=almost_full_pg2(i); --for daq page swap


      Ena_PM_del(i)           <= Ena_PM(i);         --to clear the Fifo on PM DAQ start


      if    almost_full_pulse_pg2(i) ='1' or Ena_DAQ_Start_pulse(i)='1' then
        page1_active(i)       <='1';
      elsif almost_full_pulse_pg1(i) ='1' then
        page1_active(i)       <='0';
      else
        null ;
      end if;

      if    fifo_full_pg1(i) = '1' and fifo_full_pg2(i) ='1' then
        DAQ_FIFO_Overflow(i)  <='1';                --reset both fifo pages when not served
      else
        DAQ_FIFO_Overflow(i)  <='0';
      end if;


  end if;
end process fifo_cntrl;




end generate fifo_logic;

-------------------------------------------Descriptor append logic for DAQ Mode ------------------------------------------------
--  Descriptor clocks and Descriptor data for shadow fifo is generated here
--  DAQ Descriptor is loaded on 9 following sysclk cycles after page daq swapped inactive.
--  Then Interrupt is generated with Fifo Full of shadow fifo
--  Page swaps from active to shadow fifo on (FifoLength minus 9) due to almost_full_pulse


DAQ_Descr: for i in 1 to ch_num generate


  DAQ_Descr_Logic: process (clk_i,nreset)
  begin
    if nreset = '0' then
      DAQ_Descr_cntr(i)            <= (others => '0');
      DAQ_Descr_runs(i)            <=  '0';
      DAQ_Timestamp_latched(i)     <= (others => '0');
    elsif rising_edge (clk_i) then
      if almost_full_pulse_pg1(i) ='1' or almost_full_pulse_pg2(i)='1' then
        DAQ_Timestamp_latched(i)   <= Timestamp_cntr;
      else
        null;
      end if;

      if almost_full_pulse_pg1(i) ='1' or almost_full_pulse_pg2(i)='1' then
        DAQ_Descr_cntr(i)      <= (others => '0');
        DAQ_Descr_runs(i)      <='1';
      elsif DAQ_Descr_cntr(i) /= x"8"  and DAQ_Descr_runs(i)='1' then
        DAQ_Descr_cntr(i)      <= DAQ_Descr_cntr(i) + 1;
        DAQ_Descr_runs(i)      <='1';
      else
        DAQ_Descr_runs(i)      <='0';
      end if;
    end if;
  end process DAQ_Descr_Logic;

  DAQ_Descr_data(i) <=  diob_extension_id(3 downto 0)  &  "00000000" & CtrlReg_Ch(i) (15 downto 12)  when DAQ_Descr_cntr(i)=x"0" else
                        DAQ_Timestamp_latched(i)(15 downto 0)                                        when DAQ_Descr_cntr(i)=x"1" else
                        DAQ_Timestamp_latched(i)(31 downto 16)                                       when DAQ_Descr_cntr(i)=x"2" else
                        DAQ_Timestamp_latched(i)(47 downto 32)                                       when DAQ_Descr_cntr(i)=x"3" else
                        DAQ_Timestamp_latched(i)(63 downto 48)                                       when DAQ_Descr_cntr(i)=x"4" else
                        TrigWord_LW_Ch(i)                                                            when DAQ_Descr_cntr(i)=x"5" else
                        TrigWord_HW_Ch(i)                                                            when DAQ_Descr_cntr(i)=x"6" else
                        Trig_Dly_word_Ch(i)                                                          when DAQ_Descr_cntr(i)=x"7" else
                        (
                         CtrlReg_Ch(i)(7 downto 0) & '1' & '0' & '0' & std_logic_vector (to_unsigned(I,5 ))
                        )                                                                            when DAQ_Descr_cntr(i)=x"8" else
                        x"0000";


DAQ_fifo_data_pg1(i) <= daq_dat_i_synched(i) when page1_active(i)='1' else
                        DAQ_Descr_data(i)    when page1_active(i)='0' and DAQ_Descr_runs(i)='1' else
                        x"0000";

DAQ_fifo_data_pg2(i) <= daq_dat_i_synched(i) when page1_active(i)='0' else
                        DAQ_Descr_data(i)    when page1_active(i)='1' and DAQ_Descr_runs(i)='1' else
                        x"0000";



end generate DAQ_Descr;



--------------------------------------------PM/HiRes Mode starting here -----------------------------------------------------


-- Some Remarks on PostMortem Mode ( which captures always last 1014 Data Samples of a channel)
-- Postmortem starts with Ena_Pm = 1  and clocked with fixed Ena_every_100us clock rate
-- Readout of PM Fifo is only allowed on stopped PM Capture.
-- todo : maybe send "deadbeef"  instead of "0000" on false readout
-- Ring Buffer Feature: To prevent PM FifoFull on PM Capture: last word on full PM Fifo is read before  fresh data is written.
-- Post Mortem is stopped with Ena_PM = 0
-- For full Fifo : Even when ENA_PM is set to zero at the same time when Ena_every_100us occurs, following is done:
-- PM_we is stopped one clock later with Ena_PM_del to keep exactly 1024 sample in PMFifo

-- Some Remarks on the Single Shot High-Res Mode (which is  dual-use of the 1014 Datasamples PMFIFO)
-- HiRes needs to be enabled by Ena_HiRes=1 (Enabling HiRes and PM at the same time is not allowed)
-- HiRes makes only sense in combination with the HiRes Trigger therefore do set-up of ExtTrig_nEvTrig_HiRes)

-- After entering HiRes Mode, continous sampling is done with 4MHz (250ns) sample rate
-- When Channel Trigger Condition is fired , another 914 words are sampled to fill the Fifo
-- So we get a 228µs post-trigger and a 25 µs pre-trigger history.
-- HiRes Capture stops by itself at sample position 914 after trigger event and then it releases an interrupt.
-- For next HiRes capture Ena_HiRes needs to be switched off and on again


-------------------------------------------PostMortem and HiRes FIFO Control -------------------------------------------------
--crc is calculated on each transmitted packet
--crc start pulse clears crc calculation, its cleared when descriptor is shifted to fifo
--crc_en_pulse is a one-clock pulse, same pulse as fifo readout pulse
--with each fifo readout crc is calculated with the data just read out
--(the first data visible on fifo q - without any readout - is the first data shiftet in)
--crc_data_in are the outputs of the daq fifo or pm fifo
--crc is transmitted as last word of the transmitted packet
--just after crc is readout, another crc calculation, which is a dont-care, takes place

crc_daq_gen_logic : for I in 1 to ch_num generate

  crc_daq_start_pulse(i) <= Daq_Descr_runs(i);      -- clear crc when Descriptor is loaded into DAQ Fifo
  crc_daq_en_pulse(i)    <= DAQDat_rd_pulse(i);     -- DAQDat_rd_pulse (marking end of SCUB Access) reads the next fifo word (or the crc word)

  crc_daq_data_in(i)     <=  DaqDat_Ch(i);                -- dont worry on crc_daq_data_in when crc is read out
                                                          -- this  false CRC calculation is suppressed by crc_daq_was_read

  crc_daq_was_read_pr : PROCESS (clk_i, nreset)
  BEGIN
    IF nreset = '0' THEN
      crc_daq_was_read(i) <= '0';
    ELSIF rising_edge (clk_i) THEN
     -- marking the end of readout of daq fifo samples including descripto
      IF (DAQ_Descr_cntr(i) = x"8" AND fifo_empty_pg1(i) = '1' and page1_active(i)='0') or  --inactive pg1 fifo now empty
       (DAQ_Descr_cntr(i) = x"8" AND fifo_empty_pg2(i) = '1' and page1_active(i)='1')     --inactive pg2 fifo now empty
    THEN
        IF DAQDat_rd_pulse(i) = '1' THEN                           -- marking falling_edge on crc SCUB readout
          crc_daq_was_read(i) <= '1';
        END IF;
      END IF;

     IF (DAQ_Descr_cntr(i) = x"8" AND fifo_empty_pg1(i) = '0' and page1_active(i)='0') or  --inactive pg1 fifo now filled
       (DAQ_Descr_cntr(i) = x"8" AND fifo_empty_pg2(i) = '0' and page1_active(i)='1')     --inactive pg2 fifo now filled
    THEN
        crc_daq_was_read(i) <= '0';
      END IF;
    END IF;
  END PROCESS crc_daq_was_read_pr;

end generate crc_daq_gen_logic;


crc_pm_gen_logic : for I in 1 to ch_num generate

  crc_pm_start_pulse(i) <= PM_Descr_runs(i);            -- clear crc calculation when Descriptor is loaded
  crc_pm_en_pulse(i)    <= PMDat_rd_pulse(i);           -- PMDat_rd_pulse reads the next fifo word.Here we get the actual, not next word for crc

  crc_pm_data_in(i)     <=  PMDat_Ch(i);
  PMDat_to_SCUB(i)      <=  PMDat_Ch(i)      when fifo_empty_pm(i) ='0'                            else  -- when not empty SCUB Data comes from FIFO
                            crc_pm_out(i)    when fifo_empty_pm(i) ='1' and crc_pm_was_read(i)='0' else  -- when Fifo empty and CRC was not read
                            x"0000";                                                                     -- suppresses wrong SCUB readouts

  crc_pm_was_read_pr : PROCESS (clk_i, nreset)
  BEGIN
    IF nreset = '0' THEN
      crc_pm_was_read(i) <= '0';
    ELSIF rising_edge (clk_i) THEN
      IF PM_Descr_cntr(i) = x"8" AND fifo_empty_pm(i) = '1' THEN -- marking the end of readout of pm fifo samples including descriptor
        IF PMDat_rd_pulse(i) = '1' THEN                          -- next falling_edge on SCU Slave Bus Read Accesses generates crc_pm_was_read flag
          crc_pm_was_read(i) <= '1';
        END IF;
      END IF;
      IF PM_Descr_cntr(i) = x"8" AND fifo_empty_pm(i) = '0' THEN  -- marking the begin of readout of pm fifo
        crc_pm_was_read(i) <= '0';                                -- on begining of readout crc_pm_was_read flag is cleared
      END IF;
    END IF;
  END PROCESS crc_pm_was_read_pr;

end generate crc_pm_gen_logic;

PM_Logic : for I in 1 to ch_num generate

  Start_PM_pulse(i)     <= Ena_PM(i)       and not Ena_PM_del(i);                          --rising_edge
  Start_HiRes_pulse(i)  <= Ena_HiRes(i)    and not Ena_HiRes_del(i);

  Stop_PM_pulse(i)      <= not Ena_PM(i)   and Ena_PM_del(i);                              --falling_edge
  Stop_HiRes_pulse(i)   <= not Ena_HiRes(i)and Ena_HiRes_del(i);

  PMDat_rd_pulse(i)     <= rd_PMDat_del(i) and not rd_PMDat(i);                            --falling_edge on SCU Slave Bus Read Accesses

  nSClr_PM_FIFO(i)      <= not (Start_PM_pulse(i) or Start_HiRes_pulse(i) or  Reset);      --Clear on SampleStart


  Pm_we_i(i)       <=  PM_Descr_runs(i)
                        or
                       ( Ena_every_100us_del and  (Ena_PM(i)     or  Ena_PM_del(i)    ) )
                        or
                       ( Ena_every_250ns_del and  (HiRes_runs(i) or  HiRes_runs_del(i)) )
                       ;
  -- Ena_PM_rd is used to block SCUB if when '0'
  Ena_PM_rd(i)     <=  not crc_pm_was_read(i) and not Ena_PM(i) and not Ena_HiRes(i) ;    -- PM Reads only allowed on stopped PM/Hires until CRC was read

  -- pm_rd_i is the read signal for the PM fifo, used to shuffle, to get descriptor into fifo and to get captured samples out to SCUB
  Pm_rd_i(i)       <=  (Ena_every_100us     and   Ena_PM(i)               and Fifo_full_pm(i)) or -- PM rd in PM Mode to shuffle PM Data
                       (Ena_every_250ns     and   HiRes_runs(i)           and Fifo_full_pm(i)) or -- PM rd in HiRes Mode to shuffle HiRes Data ?!
                       (PM_Descr_reached_7(i)                                                ) or -- to get PM Descriptor data in Fifo whilst FIFO Full
                       (Stop_PM_pulse(i)    and   fifo_full_pm(i)                            ) or -- to get PM Descriptor data in Fifo whilst  PM FIFO Full
                                                                                                  -- PMStop may occur on full fifo or on not full fifo
                                                                                                  -- only on full fifo we need to shuffle 1 word at PM Stop
                       (Stop_HiRes_pulse(i)                                                  ) or -- to get HiRes Descriptor data in Fifo whilst PM FIFO Full
                       (PMDat_rd_pulse(i)   and   Ena_PM_rd(i)                               )    -- to get captured  samples out to SCUB
                        ;

  -------------------------------------------------------------HiRes Trigger Control--------------------------------------------
  HiRes_Counter: process (clk_i,nreset)
  begin
    if nreset = '0' then
      HiRes_trig_cntr(i)      <=  (others => '0');
      HiRes_runs(i)           <=  '0';
    elsif rising_edge (clk_i) then

      if    Stop_HiRes_pulse(i)  ='1' then
        HiRes_trig_cntr(i)    <=  (others => '0');
        HiRes_runs(i)         <=  '0';
      elsif Start_HiRes_pulse(i) ='1' then
        HiRes_trig_cntr(i)    <= (others => '0');                                                 --set counter
        HiRes_runs(i)         <='1';                                                              --enable HiRes_Counter and wait for Trig_Pulse_HiRes
      elsif Trig_Pulse_HiRes(i) ='1' or  HiRes_trig_cntr(i) /= x"0000" then
        if HiRes_trig_cntr(i) /=    x"0392" then
          HiRes_trig_cntr(i)  <= HiRes_trig_cntr(i) + 1;
          HiRes_runs(i)       <='1';
        else
          HiRes_runs(i)       <='0';
          HiRes_trig_cntr(i)  <=  (others => '0');
        end if;
      end if;
    end if;
  end process HiRes_Counter;




--    HiRes_Counter: process (clk_i,nreset)
--  begin
--    if nreset = '0' then
--      HiRes_trig_cntr(i) <=  (others => '0');
--      HiRes_runs(i)<='0';
--    elsif rising_edge (clk_i) then
--      if Start_HiRes_pulse(i) ='1' then
--        HiRes_trig_cntr(i)    <= (others => '0');
--        HiRes_runs(i)  <='1';
--      elsif Trig_Pulse_HiRes(i) ='1' or  HiRes_trig_cntr(i) /= x"0000" then
--        if HiRes_trig_cntr(i) /=    x"0392" then
--          HiRes_trig_cntr(i) <= HiRes_trig_cntr(i) + 1;
--          HiRes_runs(i)  <='1';
--        else
--          HiRes_runs(i)  <='0';
--        end if;
--      end if;
--    end if;
--  end process HiRes_Counter;


----------------------------------- Descriptor append logic for PM and HiRes -------------------------------------------------------
-- Descriptor loaded on 9 following clocks after PM/HiRes DAQ finished.
-- PM Timestamp latched on first sysclk after PM finished
-- HiRes Timestamp latched on first sysclk after HiRes Trigger Event occurs


  PM_Descr_Logic: process (clk_i,nreset)
  begin
    if nreset = '0' then
      PM_Descr_cntr(i)            <= (others => '0');
      PM_Descr_runs(i)            <=  '0';
      PM_Timestamp_latched(i)     <= (others => '0');
      Last_mode_was_PM(i)         <= '0';
     Last_mode_was_HiRes(i)      <= '0';
     PM_Descr_reached_7(i)       <= '0';
    elsif rising_edge (clk_i) then
      if Stop_PM_pulse(i) ='1' or Trig_Pulse_HiRes(i)='1' then     --new  Tight coupling of Timestamp to HiRes Trigger Event
      --if Stop_PM_pulse(i) ='1' or (i)='1' then   -- old
        PM_Timestamp_latched(i)   <= Timestamp_cntr;
      end if;

      if Stop_PM_pulse(i) ='1' or Stop_HiRes_pulse(i)='1' then

        if Stop_PM_pulse(i) ='1' then
          Last_mode_was_PM(i)     <='1';
        elsif PM_Descr_runs(i) ='1' then
          null ;                               --keep value
        else
          Last_mode_was_PM(i)     <='0';
        end if;

        if Stop_HiRes_pulse(i) ='1' then
          Last_mode_was_HiRes(i)    <='1';
        elsif PM_Descr_runs(i) ='1' then
          null ;                               --keep value
        else
          Last_mode_was_HiRes(i)     <='0';
        end if;



      else
        null;
      end if;


      if Stop_PM_pulse(i) ='1' or Stop_HiRes_pulse(i)='1' then
        PM_Descr_cntr(i)      <= (others => '0');
        PM_Descr_runs(i)      <='1';
        PM_Descr_reached_7(i) <='1';
      elsif PM_Descr_cntr(i) /= x"8"  and PM_Descr_runs(i)='1' then
       if PM_Descr_cntr(i) = x"7" then
            PM_Descr_reached_7(i)   <='0';
       end if;
        PM_Descr_cntr(i)      <= PM_Descr_cntr(i) + 1;
        PM_Descr_runs(i)      <='1';
      else
        PM_Descr_runs(i)      <='0';
        PM_Descr_reached_7(i) <='0';
      end if;
    end if;
  end process PM_Descr_Logic;

  PM_Descr_data(i) <=   diob_extension_id(3 downto 0)  &  "00000000" & CtrlReg_Ch(i) (15 downto 12)  when PM_Descr_cntr(i)=x"0" else
                        PM_Timestamp_latched(i)(15 downto 0)                                         when PM_Descr_cntr(i)=x"1" else
                        PM_Timestamp_latched(i)(31 downto 16)                                        when PM_Descr_cntr(i)=x"2" else
                        PM_Timestamp_latched(i)(47 downto 32)                                        when PM_Descr_cntr(i)=x"3" else
                        PM_Timestamp_latched(i)(63 downto 48)                                        when PM_Descr_cntr(i)=x"4" else
                        TrigWord_LW_Ch(i)                                                            when PM_Descr_cntr(i)=x"5" else
                        TrigWord_HW_Ch(i)                                                            when PM_Descr_cntr(i)=x"6" else
                        Trig_Dly_word_Ch(i)                                                          when PM_Descr_cntr(i)=x"7" else
                        (
                         CtrlReg_Ch(i)(7 downto 0) & '0' &  Last_mode_was_HiRes (i) & Last_mode_was_PM(i) & std_logic_vector (to_unsigned(I,5 ))
                        )                                                                            when PM_Descr_cntr(i)=x"8" else
                        x"0000";

PM_fifo_data(i) <= daq_dat_i_synched(i) when PM_Descr_runs(i) ='0' else  PM_Descr_data(i);

end generate PM_Logic;


-------------------------------------------Component Instantiation Section------------------------------------------------
crc_instances:for i in 1 to ch_num generate


  crc_daq: crc5x16
    PORT MAP (
      nReset           => nReset,
      clk_i            => clk_i,
      data_in          => crc_daq_data_in(i),         -- 15 downto 0
      crc_start_pulse  => crc_daq_start_pulse(i),     -- Set CRC to its Start value on output of a new packet
      crc_en_pulse     => crc_daq_en_pulse(i),        -- Enables CRC calculation on stored previous CRC and data_in
      crc_out          => crc_daq_out(i)              -- 15 downto 0
    );
  crc_pm: crc5x16
    PORT MAP (
      nReset           => nReset,
      clk_i            => clk_i,
      data_in          => crc_pm_data_in(i),         -- 15 downto 0
      crc_start_pulse  => crc_pm_start_pulse(i),     -- Set CRC to its Start value on output of a new packet
      crc_en_pulse     => crc_pm_en_pulse(i),        -- Enables CRC calculation on stored previous CRC and data_in
      crc_out          => crc_pm_out(i)              -- 15 downto 0
    );

end generate crc_instances;



fifo_instances: for i in 1 to ch_num generate


  page1_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 16,
      g_size                   => 512, --g_size only available in 2exp(n) steps
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => true,
      g_with_count             => true,
      g_almost_full_threshold  => 502


      )
    port map(
      rst_n_i        => nSClr_DAQ_FIFO(i),
      clk_i          => clk_i,
      d_i            => DAQ_fifo_data_pg1(i),
      we_i           => we_pg1(i),
      q_o            => DaqDat_pg1(i),
      rd_i           => rd_pg1 (i),
      empty_o        => fifo_empty_pg1(i),
      full_o         => fifo_full_pg1(i),
      almost_full_o  => almost_full_pg1(i),
      count_o        => page1_fifo_words(i)
      );


  page2_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 16,
      g_size                   => 512, --g_size only available in 2exp(n) step
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => true,
      g_with_count             => true,
      g_almost_full_threshold  => 502

      )   --for simulation
    port map(
      rst_n_i        => nSClr_DAQ_FIFO(i),
      clk_i          => clk_i,
      d_i            => DAQ_fifo_data_pg2(i),
      we_i           => we_pg2(i),
      q_o            => DaqDat_pg2(i),
      rd_i           => rd_pg2 (i),
      empty_o        => fifo_empty_pg2(i),
      full_o         => fifo_full_pg2(i),
      almost_full_o  => almost_full_pg2(i),
      count_o        => page2_fifo_words(i)
      );


  postmortem_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 16,
      g_size                   => 1024,  --g_size only available in 2exp(n) step
      g_show_ahead             => true,  --shows the 1st word w/o rd request
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_with_count             => true,
      g_almost_full_threshold  => open)
    port map(
      rst_n_i        => nSClr_PM_FIFO(i),
      clk_i          => clk_i,
      d_i            => PM_fifo_data(i),
      we_i           => PM_we_i(i),
      q_o            => PmDat_Ch(i),
      rd_i           => PM_rd_i(i),
      empty_o        => fifo_empty_pm(i),
      count_o        => pm_fifo_words(i),
      full_o         => fifo_full_pm(i)

      );
end generate fifo_instances;

  -- Time Base
time_base:zeitbasis_daq
  generic map(
      CLK_in_Hz       => 125_000_000,
      diag_on         => 0
      )
  port map (
      Res             => Reset,
      Clk             => clk_i,
      Ena_every_1us   => open,
      Ena_every_10us  => Ena_every_10us,
      Ena_every_100us => Ena_every_100us,
      Ena_every_1ms   => Ena_every_1ms,
      Ena_every_250ns => Ena_every_250ns
      );

------------------------------------------------Interrupt Generation Section ----------------------------------------------------
-- Generate two consolidated Interrupt Service Requests to the SCU_Bus_Slave Macro

IRQ_PULSES: for I in 1 to ch_num generate
  -- IRQ Pulses when HiRes is finished or DAQ Fifo is full
  HiRes_IRQ_pulse(i)       <= '1' when HiRes_runs(i) ='0' and HiRes_runs_del(i)='1' else '0';

  -- ToDo : for Timing optimization HiRes_IRQ_pulse can be delayed by 9 sysclks (the header length)

  daq_irq_pulse(i)         <= fifo_full_pulse_pg1(i) or      fifo_full_pulse_pg2(i);
  fifo_full_pulse_pg1(i)   <= fifo_full_pg1(i)       and not fifo_full_pg1_del(i);
  fifo_full_pulse_pg2(i)   <= fifo_full_pg2(i)       and not fifo_full_pg2_del(i);

  -- DAQ Page swaps when fifo almost full
  almost_full_pulse_pg1(i) <= almost_full_pg1(i) and not almost_full_pg1_del(i);
  almost_full_pulse_pg2(i) <= almost_full_pg2(i) and not almost_full_pg2_del(i);

  -- Two Register to store pending Interrupts (daq_irq_reg and  HiRes_irq_reg are common for all channels)

  PENDING_IRQ_REG_bits_1_to_16: process (clk_i, nReset)
  begin
    if nReset = '0' then
      daq_irq_reg (i)   <=  '0';
      HiRes_irq_reg (i) <=  '0';
    elsif rising_edge(clk_i) then

      -- Keep in mind: ch_num i is from 1..16, bit position is from 0..15
      -- Writing 1 to a certain bit position clear the stored bit in daq_irq_reg or hires_irq_reg
      -- If writing a 1 to a reg bit whilst daq_irq_pulse occurs at the same time, pulse wins !
      if daq_irq_reg_wr = '1' and Data_from_SCUB_LA(i-1)='1'  and daq_irq_pulse(i)='0' then      -- no irq should be lost
         daq_irq_reg(i) <= '0';
      elsif daq_irq_pulse(i) ='1' then
         daq_irq_reg(i) <= '1';
      else
         null;
      end if;

      if HiRes_irq_reg_wr = '1'and Data_from_SCUB_LA(i-1)='1' and HiRes_irq_pulse(i)='0' then  -- no irq should be lost
         HiRes_irq_reg(i) <= '0';
      elsif HiRes_irq_pulse(i) ='1' then
         HiRes_irq_reg(i) <= '1';
      else
         null;
      end if;
    end if;
  end process PENDING_IRQ_REG_bits_1_to_16;
end generate IRQ_PULSES;

fill_unused_regbits: for I in (ch_num + 1) to 16 generate
      daq_irq_reg (i)   <=  '0';
      HiRes_irq_reg (i) <=  '0';
end generate fill_unused_regbits;

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


-----------------------------------------Register Section-------------------------------------------------------------------

-- For better readability CtrlReg_Ch bits are assigned to clear names
-- Todo Add bit 15..12 for SCU Slave Slots

wr_reg_bits:For i in 1 to ch_num generate
  Ena_PM(i)               <= CtrlReg_Ch (i)(0);  -- to stop or run PostMortem Sampling with fixed Sample rate 100us
  Sample10us_Ch(i)        <= CtrlReg_Ch (i)(1);  -- DAQ SampleRate (set only one of this 3 bits)
  Sample100us_Ch(i)       <= CtrlReg_Ch (i)(2);  --  * IF one of DAQ SampleRate bits is set and Trigger Mode is not enabled
  Sample1ms_Ch(i)         <= CtrlReg_Ch (i)(3);  --  * then DAQ Sampling is run in continous mode
  Ena_TrigMod(i)          <= CtrlReg_Ch (i)(4);  -- DAQ Trigger Mode: No DAQ sampling until Trig Event occurs
  ExtTrig_nEvTrig(i)      <= CtrlReg_Ch (i)(5);  -- DAQ Trigger Mode fills Fifo with samples until Fifo Full
  Ena_HiRes(i)            <= CtrlReg_Ch (i)(6);  -- Triggered HiRes SingleShot instead of PM Sampling on PM FIFO
  ExtTrig_nEvTrig_HiRes(i)<= CtrlReg_Ch (i)(7);  -- Trigger Select for HiRes Mode (uses same Trigger sources as DAQ Channel)
end generate;

max_ch_cnt <= std_logic_vector (to_unsigned(ch_num,6 ));

 rd_logic:For i in 1 to ch_num generate
  rd_logic_inst:daq_chan_reg_logic
  generic map
    (
      CtrlReg_adr        => CtrlReg_adr_ch(i),
      trig_lw_adr        => trig_lw_adr_ch(i),
      trig_hw_adr        => trig_hw_adr_ch(i),
      trig_dly_word_adr  => trig_dly_word_adr_ch(i),
      PmDat_adr          => PMDat_adr_ch(i),
      DaqDat_adr         => DAQDat_adr_ch(i),
      DAQ_FIFO_Words_adr => DAQ_FIFO_Words_adr_ch(i),
      PM_FIFO_Words_adr  => PM_FIFO_Words_adr_ch(i)
    )

  port map
    (
      Adr_from_SCUB_LA   => ADR_from_SCUB_LA,
      Data_from_SCUB_LA  => Data_from_SCUB_LA,
      Ext_Adr_Val        => Ext_Adr_Val,
      Ext_Rd_active      => Ext_Rd_active,
      Ext_Wr_active      => Ext_Wr_active,
      clk_i              => clk_i,
      nReset             => nreset,

      PmDat              => PmDat_to_SCUB(i),
      DaqDat             => DaqDat_Ch(i),
      Ena_PM_rd          => Ena_PM_rd(i),
      daq_fifo_word      => daq_fifo_words(i),
      pm_fifo_word       => pm_fifo_words(i),
      version_number     => version_number,
      max_ch_cnt         => max_ch_cnt,
      Rd_Port            => Rd_Port_Ch(i),
      user_rd_active     => user_rd_active_ch(i),

      CtrlReg_o          => CtrlReg_Ch(i),
      TrigWord_LW_o      => TrigWord_LW_Ch(i),
      TrigWord_HW_o      => TrigWord_HW_Ch(i),
      Trig_dly_word_o    => Trig_Dly_word_Ch(i),

      rd_PMDat           => rd_PMDat(i),      --active as long SCUB accesses PMFIFO
      rd_DAQDat          => rd_DAQDat(i),     --active as long SCUB accesses DAQFIFO
      dtack              => dtack_ch(i)
    );
end generate;


-- -- Generate consolidated DAQ macro Rd_Port by an large "or" of all channel Rd_Port signals
-- -- (not addressed channel Rd_Ports deliver 0x0 when not accessed)

fill_unused_signals:for i in (ch_num + 1) to (16) generate
                      dtack_ch(i)          <= '0';
                      user_rd_active_ch(i) <= '0';
                      Rd_Port_ch(i)        <= x"0000";
                    end generate;

dtack          <=  dtack_ch(1) or dtack_ch(2)  or dtack_ch(3)  or dtack_ch(4)  or dtack_ch(5) or dtack_ch(6)  or dtack_ch(7)  or dtack_ch(8)  or
                   dtack_ch(9) or dtack_ch(10) or dtack_ch(11) or dtack_ch(12) or dtack_ch(13)or dtack_ch(14) or dtack_ch(15) or dtack_ch(16) or dtack_cn_regs ;

Rd_Port        <=  Rd_Port_Ch(1) or Rd_Port_Ch(2)  or Rd_Port_Ch(3)  or Rd_Port_Ch(4)  or Rd_Port_Ch(5)  or Rd_Port_Ch(6)  or Rd_Port_Ch(7)  or Rd_Port_Ch(8)  or
                   Rd_Port_Ch(9) or Rd_Port_Ch(10) or Rd_Port_Ch(11) or Rd_Port_Ch(12) or Rd_Port_Ch(13) or Rd_Port_Ch(14) or Rd_Port_Ch(15) or Rd_Port_Ch(16) or Rd_Port_common_regs;

user_rd_active <=  user_rd_active_ch(1)  or user_rd_active_ch(2)  or user_rd_active_ch(3)  or user_rd_active_ch(4)  or user_rd_active_ch(5)  or user_rd_active_ch(6)  or
                   user_rd_active_ch(7)  or user_rd_active_ch(8)  or user_rd_active_ch(9)  or user_rd_active_ch(10) or user_rd_active_ch(11) or user_rd_active_ch(12) or
                   user_rd_active_ch(13) or user_rd_active_ch(14) or user_rd_active_ch(15) or user_rd_active_ch(16) or user_rd_active_cn_regs ;
 ----------------------------------------------------------------------------------------------

-- All Registers which are not channel specific, are placed here
adr_decoder: process (clk_i, nReset)
  begin
    if nReset = '0' then
      daq_irq_reg_rd           <= '0';
      HiRes_irq_reg_rd         <= '0';
      daq_irq_reg_wr           <= '0';
      HiRes_irq_reg_wr         <= '0';
      timestamp_cntr_word1_wr  <= '0';
      timestamp_cntr_word2_wr  <= '0';
      timestamp_cntr_word3_wr  <= '0';
      timestamp_cntr_word4_wr  <= '0';
      timestamp_cntr_word1_rd  <= '0';
      timestamp_cntr_word2_rd  <= '0';
      timestamp_cntr_word3_rd  <= '0';
      timestamp_cntr_word4_rd  <= '0';
      timestamp_cntr_tag_lw_wr <= '0';
      timestamp_cntr_tag_hw_wr <= '0';
      timestamp_cntr_tag_lw_rd <= '0';
      timestamp_cntr_tag_hw_rd <= '0';
    elsif rising_edge(clk_i) then
      daq_irq_reg_rd           <= '0';
      HiRes_irq_reg_rd         <= '0';
      daq_irq_reg_wr           <= '0';
      HiRes_irq_reg_wr         <= '0';
      timestamp_cntr_word1_wr  <= '0';
      timestamp_cntr_word2_wr  <= '0';
      timestamp_cntr_word3_wr  <= '0';
      timestamp_cntr_word4_wr  <= '0';
      timestamp_cntr_word1_rd  <= '0';
      timestamp_cntr_word2_rd  <= '0';
      timestamp_cntr_word3_rd  <= '0';
      timestamp_cntr_word4_rd  <= '0';
      timestamp_cntr_tag_lw_wr <= '0';
      timestamp_cntr_tag_hw_wr <= '0';
      timestamp_cntr_tag_lw_rd <= '0';
      timestamp_cntr_tag_hw_rd <= '0';

      if Ext_Adr_Val = '1' then
        case unsigned(Adr_from_SCUB_LA) is

          when HiRes_irq_reg_adr =>
            if Ext_Rd_active =  '1' then
              HiRes_irq_reg_rd <= '1';
            elsif Ext_Wr_active ='1' then
              HiRes_irq_reg_wr <= '1';
            end if;

          when daq_irq_reg_adr =>
            if Ext_Rd_active =  '1' then
              daq_irq_reg_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              daq_irq_reg_wr   <= '1';
            end if;

          when timestamp_cntr_word1_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_word1_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_word1_wr   <= '1';
            end if;

          when timestamp_cntr_word2_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_word2_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_word2_wr   <= '1';
            end if;

          when timestamp_cntr_word3_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_word3_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_word3_wr   <= '1';
            end if;

          when timestamp_cntr_word4_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_word4_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_word4_wr   <= '1';
            end if;

           when timestamp_cntr_tag_lw_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_tag_lw_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_tag_lw_wr   <= '1';
            end if;

          when timestamp_cntr_tag_hw_adr =>
            if Ext_Rd_active =  '1' then
              timestamp_cntr_tag_hw_rd   <= '1';
            elsif Ext_Wr_active ='1' then
              timestamp_cntr_tag_hw_wr   <= '1';
            end if;
          when others =>
            daq_irq_reg_rd           <= '0';
            HiRes_irq_reg_rd         <= '0';
            daq_irq_reg_wr           <= '0';
            HiRes_irq_reg_wr         <= '0';
            timestamp_cntr_word1_wr  <= '0';
            timestamp_cntr_word2_wr  <= '0';
            timestamp_cntr_word3_wr  <= '0';
            timestamp_cntr_word4_wr  <= '0';
            timestamp_cntr_word1_rd  <= '0';
            timestamp_cntr_word2_rd  <= '0';
            timestamp_cntr_word3_rd  <= '0';
            timestamp_cntr_word4_rd  <= '0';
          timestamp_cntr_tag_lw_wr <= '0';
          timestamp_cntr_tag_hw_wr <= '0';
          timestamp_cntr_tag_lw_rd <= '0';
          timestamp_cntr_tag_hw_rd <= '0';
          end case;
      end if;
    end if;

  end process adr_decoder;

-- wiring of common registers

Rd_Port_common_regs<= daq_irq_reg                         when daq_irq_reg_rd          ='1'  else
                      HiRes_irq_reg                       when HiRes_irq_reg_rd        ='1'  else
                      timestamp_cntr_preset(15 downto  0) when timestamp_cntr_word1_rd ='1'  else
                      timestamp_cntr_preset(31 downto 16) when timestamp_cntr_word2_rd ='1'  else
                      timestamp_cntr_preset(47 downto 32) when timestamp_cntr_word3_rd ='1'  else
                      timestamp_cntr_preset(63 downto 48) when timestamp_cntr_word4_rd ='1'  else
                      timestamp_cntr_tag   (15 downto  0) when timestamp_cntr_tag_lw_rd='1'  else
                      timestamp_cntr_tag   (31 downto 16) when timestamp_cntr_tag_hw_rd='1'  else
                      x"0000";

user_rd_active_cn_regs  <=    HiRes_irq_reg_rd        or daq_irq_reg_rd          or timestamp_cntr_tag_lw_rd    or timestamp_cntr_tag_hw_rd    or
                              timestamp_cntr_word1_rd or timestamp_cntr_word2_rd or timestamp_cntr_word3_rd     or timestamp_cntr_word4_rd;

dtack_cn_regs           <=    HiRes_irq_reg_rd        or daq_irq_reg_rd          or timestamp_cntr_tag_lw_rd    or timestamp_cntr_tag_hw_rd    or
                              timestamp_cntr_word1_rd or timestamp_cntr_word2_rd or timestamp_cntr_word3_rd     or timestamp_cntr_word4_rd     or
                              HiRes_irq_reg_wr        or daq_irq_reg_wr          or timestamp_cntr_tag_lw_wr    or timestamp_cntr_tag_hw_wr    or
                    timestamp_cntr_word1_wr or timestamp_cntr_word2_wr or timestamp_cntr_word3_wr     or timestamp_cntr_word4_wr;


---------------------------------timestamp logic-----------------------------------------------------

  timestamp_cntr_tag_matched <= '1' when Timing_Pattern_LA = timestamp_cntr_tag else '0';
  -- Timing Trigger fired on received pattern rising edge
  timestamp_cntr_tag_pulse   <= timestamp_cntr_tag_matched and Timing_Pattern_RCV and not Timing_Pattern_RCV_del;


timestamp : PROCESS (nReset, clk_i)
BEGIN
  IF nReset = '0' THEN
      timestamp_cntr        <= (OTHERS => '0');
      timestamp_cntr_preset <= (OTHERS => '0');

  ELSIF rising_edge(clk_i) THEN

      --register stuff
      if     timestamp_cntr_word1_wr  ='1' then
        timestamp_cntr_preset(15 downto  0)  <= Data_from_SCUB_LA;
      end if;
      if     timestamp_cntr_word2_wr  ='1' then
        timestamp_cntr_preset(31 downto  16) <= Data_from_SCUB_LA;
      end if;
      if     timestamp_cntr_word3_wr  ='1' then
        timestamp_cntr_preset(47 downto  32) <= Data_from_SCUB_LA;
      end if;
      if     timestamp_cntr_word4_wr  ='1' then
        timestamp_cntr_preset(63 downto  48) <= Data_from_SCUB_LA;
      end if;
      if     timestamp_cntr_tag_lw_wr ='1' then
        timestamp_cntr_tag(15 downto  0)     <= Data_from_SCUB_LA;
      end if;
      if     timestamp_cntr_tag_hw_wr ='1' then
        timestamp_cntr_tag(31 downto  16)    <= Data_from_SCUB_LA;
      end if;

    -- timestamp counter

      IF timestamp_cntr_tag_pulse = '1' THEN
        timestamp_cntr    <= timestamp_cntr_preset;
      ELSIF timestamp_cntr = x"1111_1111_1111_1111" then
      timestamp_cntr    <= x"0000_0000_0000_0000";
    ELSE
        timestamp_cntr    <= timestamp_cntr + x"0000_0000_0000_1000";
      END IF;

  END IF;
END PROCESS timestamp;


end architecture daq_arch;
