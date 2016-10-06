library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


package daq_pkg is
  
  type t_daq_dat is array(natural range <>) of std_logic_vector(15 downto 0);
  type t_daq_ctl is array(natural range <>) of std_logic;

  constant dummy_daq_dat_in :       std_logic_vector(15 downto 0) :=  (x"0000");
  constant dummy_daq_ctl_in :       std_logic :=  ('0');

  type adr_ch_array is array (1 to 16) of unsigned(15 downto 0); --Channelcount is 1 to 16 !
 
  constant Base_addr:       unsigned(15 downto 0):= x"0000"; 

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
  constant daq_irq_reg_adr:      unsigned(15 downto 0):= (Base_addr + x"0060" );
  constant HiRes_irq_reg_adr:    unsigned(15 downto 0):= (Base_addr + x"0061" ); 

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
      DaqDat_adr:           unsigned(15 downto 0):= x"0000"
      );

  port  (
      Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);-- latched address from SCU_Bus
      Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);-- latched data from SCU_Bus 
      Ext_Adr_Val:			  in		std_logic;							      -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active:			in		std_logic;							      -- '1' => Wr-Cycle is active
      clk_i:						  in		std_logic;							      -- same clk as SCU_Bus_Slave macro
      nReset:					    in		std_logic;
      PmDat:              in    std_logic_vector(15 downto 0);
      DaqDat:             in    std_logic_vector(15 downto 0);
      Ena_PM_rd:          in    std_logic;
      
      Rd_Port:            out   std_logic_vector(15 downto 0);
      user_rd_active:     out   std_logic;
      rd_PMDat:           out   std_logic;      
      rd_DAQDat:          out   std_logic;
      dtack :             out   std_logic
    );
  end component daq_chan_rd_logic;
  
end daq_pkg;