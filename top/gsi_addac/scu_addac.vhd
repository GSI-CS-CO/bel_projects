library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_altera_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;

entity scu_addac is
  port (
          -------------------------------------------------------------------------------------------------------------------
          CLK_FPGA:               in    std_logic;
          
          --------- Parallel SCU-Bus-Signale --------------------------------------------------------------------------------
          A_A:		                in    std_logic_vector(15 downto 0);  -- SCU-Adressbus
          A_nADR_EN:              out   std_logic := '0';		            -- '0' => externe Adresstreiber des Slaves aktiv
          A_nADR_SCUB:	          out   std_logic := '0';  		          -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave  
          A_D:		                inout std_logic_vector(15 downto 0);  -- SCU-Datenbus
          A_nDS:	                in    std_logic;		                  -- Data-Strobe vom Master gertieben
          A_RnW:	                in    std_logic;		                  -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
          A_nSel_Ext_Data_Drv:    out   std_logic;		                  -- '0' => externe Datentreiber des Slaves aktiv
          A_Ext_Data_RD:	        out   std_logic;		                  -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                        --  Slave (besser default 0, oder Treiber A/B tauschen)
                                                                        -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
          A_nDtack:		            out   std_logic;                      -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                        -- Opendrain-Treiber
          A_nSRQ:		              out   std_logic;		                  -- Service-Request null aktiv, '0' => aktiviert externen
                                                                        -- Opendrain-Treiber
          A_nBoardSel:		        in    std_logic;		                  -- '0' => Master aktiviert diesen Slave
          A_nEvent_Str:		        in	  std_logic;		                  -- '0' => Master sigalisiert Timing-Zyklus
          A_SysClock:		          in	  std_logic;		                  -- Clock vom Master getrieben.
          A_Spare0:		            in	  std_logic;		                  -- vom Master getrieben
          A_Spare1:		            in	  std_logic;		                  -- vom Master getrieben
          A_nReset:               in    std_logic;
          
          -------------------------------------------------------------------------------------------------------------------
          
          A_nState_LED:           out   std_logic_vector(2 downto 0)   -- SEL, R/W, ...
  );
end entity;


architecture scu_addac_arch of scu_addac is

  signal clk_sys, clk_cal, rstn, locked : std_logic;
  signal clk_sys_rstn : std_logic;
  
  signal SCUB_SRQ:    std_logic;
  signal SCUB_Dtack:  std_logic;
  
begin

  

-- Obtain core clocking
sys_pll_inst : sys_pll      -- Altera megafunction
  port map (
    inclk0 => CLK_FPGA,     -- 125Mhz oscillator from board
    c0     => clk_sys,      -- 62.5MHz system clk (cannot use external pin as clock for RAM blocks)
    c1     => clk_cal,      -- 50Mhz calibration clock for Altera reconfig cores
    locked => locked);       -- '1' when the PLL has locked

SCU_Slave : SCU_Bus_Slave
generic map (
	      CLK_in_Hz		=>  62500000,
	      Firmware_Release	=>  0,
	      Firmware_Version	=>  1,
	      Hardware_Release	=>  2,
	      Hardware_Version	=>  3,
	      Intr_Edge_Trig	=>  "111111111111110",
	      Intr_Enable	=>  "000000000000001",
	      Intr_Level_Neg	=>  "000000000000000",
	      Slave_ID		=>  16
	    )
port map  (
	    SCUB_Addr           =>  A_A,
	    SCUB_Data           =>  A_D,
	    nSCUB_DS            =>  A_nDS,
	    SCUB_RDnWR	        =>  A_RnW,
	    clk                 =>  clk_sys,
	    nSCUB_Slave_Sel     =>  A_nBoardSel,
	    nSCUB_Reset_in      =>  A_nReset,
	    nSCUB_Timing_Cyc    =>  A_nEvent_Str,
	    Dtack_to_SCUB       =>  '0',
	    User_Ready	        =>  '1',
	    Data_to_SCUB        =>  x"0000",
	    Intr_In             =>  "000000000000000",
	    nSCUB_Dtack_Opdrn   =>  open,
	    SCUB_Dtack	        =>  SCUB_Dtack,
	    nSCUB_SRQ_Opdrn     =>  open,
	    SCUB_SRQ            =>  SCUB_SRQ,
	    nSel_Ext_Data_Drv   =>  A_nSel_Ext_Data_Drv,
	    Ext_Data_Drv_Rd     =>  A_Ext_Data_RD,
	    ADR_from_SCUB_LA    =>  open,
	    Data_from_SCUB_LA   =>  open,
	    Ext_Adr_Val	        =>  open,
	    Ext_Rd_active       =>  open,
	    Ext_Rd_fin	        =>  open,
	    Ext_Rd_Fin_ovl      =>  open,
	    Ext_Wr_active       =>  open,
	    Ext_Wr_fin	        =>  open,
	    Ext_Wr_Fin_ovl      =>  open,
	    Timing_Pattern_LA   =>  open,
	    Timing_Pattern_RCV	=>  open
	);
  
  
  sel_led: led_n
  generic map (
    stretch_cnt => 6250000
    )
  port map (
    ena => '1',
    clk => clk_sys,
    Sig_in => A_nBoardSel,
    nLED => A_nState_LED(0),
    nLED_opdrn => open);
    
  rw_led: led_n
  generic map (
    stretch_cnt => 6250000
    )
  port map (
    ena => '1',
    clk => clk_sys,
    Sig_in => A_RnW,
    nLED => A_nState_LED(2),
    nLED_opdrn => open);
    

  A_nState_LED(1) <= '1';
  A_nDtack <= not SCUB_Dtack;
  A_nSRQ <= not SCUB_SRQ;


end architecture;