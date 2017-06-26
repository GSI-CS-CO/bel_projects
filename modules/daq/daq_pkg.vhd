library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


package daq_pkg is
  
  type t_daq_dat  is array(natural range <>) of std_logic_vector(15 downto 0);
  type t_daq_ctl  is array(natural range <>) of std_logic;

  type t_pm_uw    is array(natural range <>) of std_logic_vector(4 downto 0);
  type t_daq_uw   is array(natural range <>) of std_logic_vector(3 downto 0);
  type t_daq_ts   is array(natural range <>) of std_logic_vector(63 downto 0); 
  type t_daq_cn   is array(natural range <>) of std_logic_vector(3 downto 0);
  type t_daq_dctr is array(natural range <>) of std_logic_vector(3 downto 0);

  
  constant dummy_daq_dat_in :       std_logic_vector(15 downto 0) :=  (x"0000");
  constant dummy_daq_ctl_in :       std_logic :=  ('0');

  type adr_ch_array is array (1 to 16) of unsigned(15 downto 0); --Channelcount is 1 to 16 !
 

  COMPONENT daq
  generic (
    Base_addr:          unsigned(15 downto 0):= x"0000";
    CLK_sys_in_Hz:      integer := 125000000;
    ch_num:             natural := 1
          );
	PORT (
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:			in		std_logic;							      -- '1' => Rd-Cycle is active
      Ext_Wr_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:						  in		std_logic;							      -- should be the same clk, used by SCU_Bus_Slave
      nReset:					    in		std_logic;
      
      timestamp:          in    std_logic_vector(63 downto 0);--WR Timestamp
      diob_extension_id:  in    std_logic_vector(15 downto 0);
      
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
  END COMPONENT;


  component zeitbasis_daq
  generic (
      CLK_in_Hz:       integer;
      diag_on:         integer
      );
  port  (
      Res:              in  std_logic;
      Clk:              in  std_logic;
      
      Ena_every_10us:   out std_logic;
      Ena_every_100us:  out std_logic;
      Ena_every_1ms:    out std_logic;
      Ena_every_250ns:  out std_logic
      );
  end component;
  
  component daq_chan_wrregs is
  generic(
      CtrlReg_adr:          unsigned(15 downto 0):= x"0000";
      trig_lw_adr:          unsigned(15 downto 0):= x"0000";
      trig_hw_adr:          unsigned(15 downto 0):= x"0000";
      trig_dly_word_adr:    unsigned(15 downto 0):= x"0000"
      );
  port  (
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Wr_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:						  in		std_logic;							      --  same clk as SCU_Bus_Slave macro
      nReset:					    in		std_logic;
      
      CtrlReg:            out   std_logic_vector(15 downto 0);
      TrigWord_LW :       out   std_logic_vector(15 downto 0);
      TrigWord_HW:        out   std_logic_vector(15 downto 0);
      Trig_dly_word:      out   std_logic_vector(15 downto 0);
      Dtack:              out   std_logic                     -- Dtack to SCU Bus Macro
    );
  end component daq_chan_wrregs;

  component daq_chan_rd_logic is
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
  end component daq_chan_rd_logic;
  
end daq_pkg;