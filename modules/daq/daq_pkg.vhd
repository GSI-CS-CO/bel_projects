LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;



PACKAGE daq_pkg IS
 
  TYPE t_daq_dat  IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(15 DOWNTO 0);
  TYPE t_daq_ctl  IS ARRAY(NATURAL RANGE <>) OF std_logic;

  TYPE t_pm_uw    IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(4 DOWNTO 0);
  TYPE t_daq_uw   IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(3 DOWNTO 0);
  TYPE t_daq_ts   IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(63 DOWNTO 0);
  TYPE t_daq_cn   IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(3 DOWNTO 0);
  TYPE t_daq_dctr IS ARRAY(NATURAL RANGE <>) OF std_logic_vector(3 DOWNTO 0);

 
  CONSTANT dummy_daq_dat_in : std_logic_vector(15 DOWNTO 0) := (x"0000");
  CONSTANT dummy_daq_ctl_in : std_logic := ('0');

  COMPONENT daq
    GENERIC (
      Base_addr     : unsigned(15 DOWNTO 0) := x"0000";
      CLK_sys_in_Hz : INTEGER := 125_000_000;
      ch_num        : NATURAL := 1
    );
    PORT (
      Adr_from_SCUB_LA   : IN std_logic_vector(15 DOWNTO 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA  : IN std_logic_vector(15 DOWNTO 0);-- latched data from SCU_Bus
      Ext_Adr_Val        : IN std_logic;                    -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active      : IN std_logic;                    -- '1' => Rd-Cycle is active
      Ext_Wr_active      : IN std_logic;                    -- '1' => Wr-Cycle is active
      clk_i              : IN std_logic;                    -- should be the same clk as used by SCU_Bus_Slave
      nReset             : IN std_logic;
 
      diob_extension_id  : IN std_logic_vector(15 DOWNTO 0);
 
      user_rd_active     : OUT std_logic;
      Rd_Port            : OUT std_logic_vector(15 DOWNTO 0);-- Data to SCU Bus Macro
      Dtack              : OUT std_logic;                    -- Dtack to SCU Bus Macro
      daq_srq            : OUT std_logic;                    -- consolidated irq lines from n daq channels for "channel fifo full"
      HiRes_srq          : OUT std_logic;                    -- consolidated irq lines from n HiRes channels for "HiRes Daq finished"
      Timing_Pattern_LA  : IN std_logic_vector(31 DOWNTO 0); -- latched data from SCU_Bus
      Timing_Pattern_RCV : IN std_logic;                     -- timing pattern received
 
      --daq input channels
      daq_dat_i          : IN t_daq_dat (1 TO ch_num);             
      daq_ext_trig       : IN t_daq_ctl (1 TO ch_num)              

    );
  END COMPONENT;
  COMPONENT zeitbasis_daq
    GENERIC (
      CLK_in_Hz : INTEGER;
      diag_on   : INTEGER
    );
    PORT (
      Res                : IN std_logic;
      Clk                : IN std_logic;
      Ena_every_1us      : OUT std_logic;                        
      Ena_every_10us     : OUT std_logic;
      Ena_every_100us    : OUT std_logic;
      Ena_every_1ms      : OUT std_logic;
      Ena_every_250ns    : OUT std_logic
    );
  END COMPONENT;
  COMPONENT daq_chan_reg_logic IS
    GENERIC (
      CtrlReg_adr        : unsigned(15 DOWNTO 0) := x"0000";
      trig_lw_adr        : unsigned(15 DOWNTO 0) := x"0000";
      trig_hw_adr        : unsigned(15 DOWNTO 0) := x"0000";
      trig_dly_word_adr  : unsigned(15 DOWNTO 0) := x"0000";
      PmDat_adr          : unsigned(15 DOWNTO 0) := x"0000";
      DaqDat_adr         : unsigned(15 DOWNTO 0) := x"0000";
      DAQ_FIFO_Words_adr : unsigned(15 DOWNTO 0) := x"0000";
      PM_FIFO_Words_adr  : unsigned(15 DOWNTO 0) := x"0000"
    );
    PORT (
      -- SCUB interface
      Adr_from_SCUB_LA   : IN std_logic_vector(15 DOWNTO 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA  : IN std_logic_vector(15 DOWNTO 0);-- latched data from SCU_Bus
      Ext_Adr_Val        : IN std_logic;                    -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active      : IN std_logic;                    -- '1' => Rd-Cycle is active
      Ext_Wr_active      : IN std_logic;                    -- '1' => Rd-Cycle is active 
      clk_i              : IN std_logic;                    -- should be the same clk as used by SCU_Bus_Slave
      nReset             : IN std_logic;
                         
      PmDat              : IN std_logic_vector(15 DOWNTO 0);
      DaqDat             : IN std_logic_vector(15 DOWNTO 0);
      Ena_PM_rd          : IN std_logic;
      daq_fifo_word      : IN std_logic_vector (8 DOWNTO 0); 
      pm_fifo_word       : IN std_logic_vector (9 DOWNTO 0);
      version_number     : IN std_logic_vector (15 downto 9);
      max_ch_cnt         : IN std_logic_vector (15 downto 10);
                         
      Rd_Port            : OUT std_logic_vector(15 DOWNTO 0);
      user_rd_active     : OUT std_logic;
                         
      CtrlReg_o          : OUT std_logic_vector(15 DOWNTO 0);
      TrigWord_LW_o      : OUT std_logic_vector(15 DOWNTO 0);
      TrigWord_HW_o      : OUT std_logic_vector(15 DOWNTO 0);
      Trig_dly_word_o    : OUT std_logic_vector(15 DOWNTO 0);
      rd_PMDat           : OUT std_logic; 
      rd_DAQDat          : OUT std_logic;
      dtack              : OUT std_logic
    );
  END COMPONENT daq_chan_reg_logic;
  
  COMPONENT crc5x16 IS
    PORT (
      nReset           : IN std_logic;
      clk_i            : IN std_logic;
      data_in          : IN std_logic_vector (15 DOWNTO 0);
      crc_start_pulse  : IN std_logic;                       -- Set CRC to its Start value on transmission of a new packet
      crc_en_pulse     : IN std_logic;                       -- Enables CRC calculation on stored previous CRC and data_in 
      crc_out          : OUT std_logic_vector (15 DOWNTO 0)
    );
  END COMPONENT crc5x16;

END daq_pkg;
