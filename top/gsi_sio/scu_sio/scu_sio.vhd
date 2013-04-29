LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;


LIBRARY work;
use work.scu_bus_slave_pkg.all;
use work.scu_slave_fg_pkg.all;
use work.wr_altera_pkg.all;

ENTITY scu_sio IS 
generic (
          CLK_in_Hz:          integer := 120000000;
          Dec_Enc_CLK_in_Hz:  integer := 24000000;
          SCU_SIO2_ID:        integer range 16#0200# to 16#020F# := 16#0200#
        );
port	( 
        nCB_RESET:      in std_logic;
        clk_fpga:       in std_logic;
        F_PLL_5:        in std_logic;
        GP_Ext_Out_B:   in std_logic;
        A_nReset:       in std_logic;
        A_ME_DSC:       in std_logic;
        A_ME_VW:        in std_logic;
        A_ME_CDS:       in std_logic;
        A_ME_TD:        in std_logic;
        A_ME_ESC:       in std_logic;
        A_ME_SD:        in std_logic;
        A_ME_nBOO:      in std_logic;
        A_ME_nBZO:      in std_logic;
        A_MIL1_BOI:     in std_logic;
        A_MIL1_BZI:     in std_logic;
        A_UMIL15V:      in std_logic;
        A_UMIL5V:       in std_logic;
        A_nOPK_INL:     in std_logic;
        A_nOPK_DRDY:    in std_logic;
        A_nOPK_DRQ:     in std_logic;
        A_ME_SDO:       in std_logic;
        clk_20:         in std_logic;

--------Galvanisch entkoppeltes LEMO-IO ---------------------------------------------------------------------------
        A_LEMO_1_IO:	      inout std_logic;
        A_LEMO_1_EN_IN:     out   std_logic;  -- '1' => A_LEMO_1_IO ist Eingang; '0' A_LEMO_1_IO ist Ausgang
        A_nLEMO_1_LED:      out   std_logic;	-- für Aktivitätsanzeige von A_LEMO_1_IO vorgesehen   
        A_LEMO_2_IO:	      inout std_logic;
        A_LEMO_2_EN_IN:     out   std_logic;  -- '1' => A_LEMO_2_IO ist Eingang; '0' A_LEMO_2_IO ist Ausgang
        A_nLEMO_2_LED:      out   std_logic;	-- für Aktivitätsanzeige von A_LEMO_2_IO vorgesehen
        A_LEMO_3_IO: 	      inout std_logic;
        A_LEMO_3_EN_IN:     out   std_logic;  -- '1' => A_LEMO_3_IO ist Eingang; '0' A_LEMO_3_IO ist Ausgang
        A_nLEMO_3_LED:      out   std_logic;  -- für Aktivitätsanzeige von A_LEMO_3_IO vorgesehen
        A_LEMO_4_IO:	      inout std_logic;
        A_LEMO_4_EN_IN:     out   std_logic;  -- '1' => A_LEMO_4_IO ist Eingang; '0' A_LEMO_4_IO ist Ausgang
        A_nLEMO_4_LED:      out   std_logic;	-- für Aktivitätsanzeige von A_LEMO_4_IO vorgesehen
  
-------- OneWire --------------------------------------------------------------------------------------------------
	  A_OneWire_EEPROM:   inout std_logic;  -- OneWire-EE-Prom auf Slave bestückt
	  A_OneWire:          inout std_logic;  -- OneWire-ADC auf Slave bestückt

--------- Parlalle SCU-Bus-Signale --------------------------------------------------------------------------------
	  A_A:		          in    std_logic_vector(15 downto 0);  -- SCU-Adressbus
	  A_nADR_EN:	          out   std_logic;		      -- '0' => externe Adresstreiber des Slaves aktiv
	  A_nADR_SCUB:	          out   std_logic;  		      -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave  
	  A_D:		          inout std_logic_vector(15 downto 0);  -- SCU-Datenbus
	  A_nDS:	          in    std_logic;		      -- Data-Strobe vom Master gertieben
	  A_RnW:	          in    std_logic;		      -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
	  A_nSel_Ext_Data_Drv:    out   std_logic;		      -- '0' => externe Datentreiber des Slaves aktiv
	  A_Ext_Data_RD:	  out   std_logic;		      -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                      --  Slave (besser default 0, oder Treiber A/B tauschen)
	  A_nSEL_Ext_Signal_RDV:  out   std_logic;		      -- '0' => Treiber für SCU-Bus-Steuer-Signale aktiv
	  A_nExt_Signal_In:	  out   std_logic;		      -- '0' => Treiber für SCU-Bus-Steuer-Signale-Richtung:
                                                                      -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
	  A_nDtack:		  out   std_logic;                    -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                      -- Opendrain-Treiber
	  A_nSRQ:		  out   std_logic;		      -- Service-Request null aktiv, '0' => aktiviert externen
                                                                      -- Opendrain-Treiber
	  A_nBoardSel:		  in    std_logic;		      -- '0' => Master aktiviert diesen Slave
          A_nEvent_Str:		  in	std_logic;		      -- '0' => Master sigalisiert Timing-Zyklus
	  A_SysClock:		  in	std_logic;		      -- Clock vom Master getrieben.
	  A_Spare0:		  in	std_logic;		      -- vom Master getrieben
	  A_Spare1:		  in	std_logic;		      -- vom Master getrieben

-------- Logikanalysator Ports ------------------------------------------------------------------------------------
	  A_TA:			  inout	std_logic_vector(15 downto 0);	-- Logikanalysator Port A
	  A_TB:			  inout	std_logic_vector(15 downto 0);	-- Logikanalysator Port B
	  A_SEL:		  in	std_logic_vector(3 downto 0);	-- Hex-Schalter, z.B. zur Auswahl bestimmter Signalgruppen
                                                                        -- auf die Logikanalysator Ports

-------- Frontplatten-LEDs (2,5 Volt-IO) --------------------------------------------------------------------------
	  A_nLED:		  out	std_logic_vector(15 downto 0);
    
-------- 3,3 Volt-IO zum Carrierboard des SCU-Bus-Masters ---------------------------------------------------------
	  EIO:			  inout	std_logic_vector(17 downto 0);
		
-------- 2,5 Volt-IO zum Carrierboard des SCU-Bus-Masters (LVDS möglch) -------------------------------------------
	  IO_2_5V:		  inout	std_logic_vector(15 downto 0);

-------- Altes GSI Timing (MIL based)------------------------------------------------------------------------------
    A_Timing:             in std_logic;  -- über 2pol. LEMO-Buchse und Optokoppler
    A_nLED_Timing:        out std_logic;  -- 2,5 Volt-IO zur LED-Signalisierung, dass Timing empfangen wird
    
    GP_Ext_In:            out std_logic;
    nExtension_Res_Out:   out std_logic;
	  A_ME_BOI:             out std_logic;
	  A_ME_BZI:             out	std_logic;
	  A_ME_UDI:             out	std_logic;
	  A_ME_12MHZ:           out std_logic;
	  A_ME_SDI:             out	std_logic;
	  A_ME_EE:              out	std_logic;
	  A_ME_SS:              out	std_logic;
	  A_MIL1_nBZO:          out	std_logic;
	  A_MIL1_Out_Ena:       out std_logic;
	  A_MIL1_nBOO:          out	std_logic;
	  A_MIL1_nIN_Ena:       out	std_logic
  );
end scu_sio;

ARCHITECTURE arch_scu_sio OF scu_sio IS 

component test_user_reg
generic (
          Base_addr : integer
	);
port	(
        Ext_Adr_Val:		in std_logic;
        Ext_Rd_active:	in std_logic;
        Ext_Rd_fin:		in std_logic;
        Ext_Wr_active:	in std_logic;
        Ext_Wr_fin:		in std_logic;
        clk:			in std_logic;
        nReset:		in std_logic;
        Adr_from_SCUB_LA:	in std_logic_vector(15 downto 0);
        Data_from_SCUB_LA:	in std_logic_vector(15 downto 0);
        Dtack_to_SCUB:	out std_logic;
        Data_to_SCUB:		out std_logic_vector(15 downto 0);
        User1_Reg:		out std_logic_vector(15 downto 0);
        User2_Reg:		out std_logic_vector(15 downto 0)
      );
end component;


component pll_sio
port	(
        inclk0:   in  std_logic;
        c0:       out std_logic;
        c1:       out std_logic;
        c2:       out std_logic;
        c3:       out std_logic;
        locked:   out std_logic
      );
end component;


component SysClock
port	(
        inclk0:   in  std_logic := '0';
        c0:       out std_logic;
        c1:       out std_logic;
        locked:   out std_logic 
      );
end component;


component pu_reset
generic	(
          PU_Reset_in_clks : integer
        );
port	(
        Clk:    in std_logic;
        PU_Res: out std_logic
      );
end component;

component lemo_io
GENERIC	(
	  Opendrain_is_1:   integer := 1;
	  stretch_cnt:      integer := 3
	);
port	(
	  reset:              in STD_LOGIC;
	  clk:                in STD_LOGIC;
	  led_clk:            in STD_LOGIC;
	  lemo_io_is_output:  in STD_LOGIC;
	  stretch_led_off:    in STD_LOGIC;
	  to_lemo:            in STD_LOGIC;
	  lemo_io:            inout STD_LOGIC;
	  lemo_en_in:         out STD_LOGIC;
	  activity_led_n:     out STD_LOGIC
	);
end component;

component zeitbasis
generic	(
	  CLK_in_Hz:  integer;
	  diag_on:    integer
	);
port	(
	  Res:		      in  std_logic;
	  Clk:		      in  std_logic;
	  Ena_every_100ns:    out std_logic;
	  Ena_every_166ns:    out std_logic;
	  Ena_every_250ns:    out std_logic;
	  Ena_every_500ns:    out std_logic;
	  Ena_every_1us:      out std_logic;
	  Ena_Every_20ms:     out std_logic
	);
end component;

signal  ADR_from_SCUB_LA:     std_logic_vector(15 downto 0);
signal	clk:                  std_logic;
signal	Data_from_SCUB_LA:    std_logic_vector(15 downto 0);
signal	Data_to_SCUB:         std_logic_vector(15 downto 0);
signal	Dtack_to_SCUB:	      std_logic;
signal	Ena_Every_100ns:      std_logic;
signal	Ena_Every_166ns:      std_logic;
signal	Ena_Every_20ms:	      std_logic;
signal	Ena_Every_250ns:      std_logic;
signal	Ena_Every_500ns:      std_logic;
signal	Ext_Adr_Val:          std_logic;
signal	Ext_Rd_active:	      std_logic;
signal	Ext_Rd_fin:           std_logic;
signal	Ext_Wr_active:	      std_logic;
signal	Ext_Wr_fin:           std_logic;
signal	Intr_In:              std_logic_vector(15 downto 1);
signal	la_clk:               std_logic;
signal	pll_1_locked:         std_logic;
signal	pll_2_locked:         std_logic;
SIGnal	SysClock_pll_out:     std_logic;
signal	Manchester_clk:	      std_logic;
signal	nPowerup_Res:         std_logic;
signal	SCLR:                 std_logic;
signal	SCU_Dtack:            std_logic;
signal	SCUB_SRQ:             std_logic;
signal	Standard_Reg_Acc:     std_logic;
signal	User1_Reg:            std_logic_vector(15 downto 0);
signal	User2_Reg:            std_logic_vector(15 downto 0);
signal	SYNTHESIZED_WIRE_0:   std_logic;
signal	F_8_MHz:              std_logic;

signal	user_reg1_Dtack:        std_logic;
signal	user_reg1_data_to_SCUB:	std_logic_vector(15 downto 0);

signal	fg1_read_cycle:     std_logic;
signal	fg1_sw:             std_logic_vector(31 downto 0);
signal	fg1_data_str:       std_logic;	
signal	fg1_data_point:     std_logic;
signal	fg1_data_to_SCUB:   std_logic_vector(15 downto 0);
signal	fg1_Dtack_to_SCUB:  std_logic;
signal	fg1_rd_cycle:       std_logic;

signal	Timing_Pattern_RCV:	std_logic;
signal	Timing_Pattern_LA:	std_logic_vector(31 downto 0);

signal	test_port_in_0:		std_logic_vector(31 downto 0);

signal	DFF_inst10:		std_logic;


begin 
nExtension_Res_Out <= '1';

A_nADR_EN <= '0';
A_nADR_SCUB <= '0';
A_nExt_Signal_In <= '0';
A_nSEL_Ext_Signal_RDV <= '0';

A_LEMO_3_EN_IN <= '1';
A_LEMO_4_EN_IN <= '1';
A_nLED_Timing <= '1';

A_nLEMO_3_LED <= '1';
A_nLEMO_4_LED <= '1';

A_ME_UDI <= '0';
SYNTHESIZED_WIRE_0 <= '1';


lemo_1: lemo_io
generic map (
	      Opendrain_is_1  =>  1,
	      stretch_cnt     =>  3
	    )
port map    (
	      reset             =>  SCLR,
	      clk               =>  clk,
	      led_clk           =>  Ena_Every_20ms,
	      lemo_io_is_output	=>  user2_reg(0),
        stretch_led_off   =>  user2_reg(1),
	      to_lemo           =>  A_Spare0,
	      lemo_io           =>  A_LEMO_1_IO,
	      lemo_en_in        =>  A_LEMO_1_EN_IN,
	      activity_led_n    =>  A_nLEMO_1_LED
	    );

lemo_2: lemo_io
generic map (
	      Opendrain_is_1  =>  1,
	      stretch_cnt     =>  3
	    )
port map  (
	    reset               =>  SCLR,
	    clk                 =>  clk,
	    led_clk             =>  Ena_Every_20ms,
	    lemo_io_is_output   =>  user2_reg(2),
	    stretch_led_off     =>  user2_reg(3),
	    to_lemo             =>  A_Spare1,
	    lemo_io             =>  A_LEMO_2_IO,
	    lemo_en_in          =>  A_LEMO_2_EN_IN,
	    activity_led_n      =>  A_nLEMO_2_LED
	  );


user_reg1: test_user_reg
generic map (
	      Base_addr	=>  16#200#
	    )
port map  (
	    Ext_Adr_Val	        =>  Ext_Adr_Val,
	    Ext_Rd_active       =>  Ext_Rd_active,
	    Ext_Rd_fin	        =>  Ext_Rd_fin,
	    Ext_Wr_active       =>  Ext_Wr_active,
	    Ext_Wr_fin	        =>  Ext_Wr_fin,
	    clk                 =>  clk,
	    nReset              =>  nCB_RESET,
	    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,
	    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,
	    Dtack_to_SCUB       =>  user_reg1_Dtack,
	    Data_to_SCUB        =>  user_reg1_data_to_SCUB,
	    User1_Reg           =>  User1_Reg,
	    User2_Reg           =>  User2_Reg
	  );


A_nLED <= NOT(User1_Reg);

fg1: SCU_Slave_FG
generic	map (
	      sys_clk_in_hz => 125_000_000,
	      base_addr	    => X"0300"
	    )
port map  (
	    fg_clk            =>  F_8_MHz,              -- attention, fg_clk is 8.192 Mhz
	    ext_clk           =>  '0',
	    sys_clk           =>  SysClock_pll_out,     -- should be the same clk, used by SCU_Bus_Slave
	    ADR_from_SCUB_LA  =>  ADR_from_SCUB_LA,     -- latched address from SCU_Bus
	    Ext_Adr_Val	      =>  Ext_Adr_Val,          -- '1' => "ADR_from_SCUB_LA" is valid
	    Ext_Rd_active     =>  Ext_Rd_active,        -- '1' => Rd-Cycle is active
	    Ext_Rd_fin	      =>  Ext_Rd_fin,           -- marks end of read cycle, active one for one clock period of sys_clk
	    Ext_Wr_active     =>  Ext_Wr_active,        -- '1' => Wr-Cycle is active
	    Ext_Wr_fin	      =>  Ext_Wr_fin,           -- marks end of write cycle, active one for one clock period of sys_clk
	    Data_from_SCUB_LA =>  Data_from_SCUB_LA,    -- latched data from SCU_Bus 
	    nPowerup_Res      =>  nCB_RESET,            -- '0' => the FPGA make a powerup
	    nStartSignal      =>  A_Spare0,             -- '0' => started FG if broadcast_en = '1'
	    Data_to_SCUB      =>  fg1_Data_to_SCUB,     -- connect read sources to SCUB-Macro
	    Dtack_to_SCUB     =>  fg1_Dtack_to_SCUB,    -- connect Dtack to SCUB-Macro
	    dreq              =>  open,                 -- data request interrupt for new SW3
	    read_cycle_act    =>  fg1_read_cycle,       -- this macro has data for the SCU-Bus
	    sw_out            =>  fg1_sw,       
	    new_data_strobe   =>  fg1_data_str,         -- sw_out changed
	    data_point_strobe =>  fg1_data_point        -- data point reached
    );


testport_mux:	process	(A_SEL, test_port_in_0, fg1_sw, Timing_Pattern_LA)
variable test_out: std_logic_vector(31 downto 0);
begin
  case (A_SEL) is
    when X"0" => test_out := test_port_in_0;
    when X"1" => test_out := fg1_sw(31 downto 0);
    when X"2" => test_out := Timing_Pattern_LA;
    when others =>  test_out := (others => '0');
  end case;
    A_TB <= test_out(31 downto 16);
    A_TA <= test_out(15 downto 0);
end process testport_mux;


rd_port_mux:	process	(fg1_rd_cycle, fg1_data_to_SCUB, user_reg1_data_to_SCUB)
begin
  if fg1_rd_cycle = '1' then
    Data_to_SCUB <= fg1_data_to_SCUB;
  else
    Data_to_SCUB <= user_reg1_data_to_SCUB;
  end if;
end process rd_port_mux;


process(clk)
begin
IF (RISING_EDGE(clk)) THEN
  DFF_inst10 <= SYNTHESIZED_WIRE_0;
end IF;
end process;


test_port_in_0 <= X"0000" &                                                             -- bit31..16
                  SCLR & clk & Ena_Every_100ns & Ena_Every_166ns &                      -- bit15..12
                  Ena_Every_250ns & Ena_Every_500ns & pll_1_locked &  pll_2_locked &    -- bit11..8
                  la_clk & SysClock_pll_out & A_RnW & A_nDS &                           -- bit7..4
                  Timing_Pattern_RCV &	fg1_data_str & fg1_data_point & SCU_Dtack;      -- bit3..0

A_ME_BZI <= A_MIL1_BZI;
A_ME_BOI <= A_MIL1_BOI;


fl : flash_loader
port map (
	    noe_in  =>  '0'
	  );


PLL_1: pll_sio
port map  (
	    inclk0  =>	clk_fpga,
	    c0	    =>	clk,
	    c2	    =>	A_ME_12MHZ,
	    c3	    =>	la_clk,
	    locked  =>	pll_1_locked
	  );


PLL_2: SysClock
port map  (
	    inclk0  =>	A_SysClock,
	    c0	    =>	SysClock_pll_out,
	    c1	    =>	F_8_MHz,
	    locked  =>	pll_2_locked
	);


b2v_PU_Reset : pu_reset
generic map (
	      PU_Reset_in_clks	=>  16
	    )
port map  (
	    Clk     =>  clk,
	    PU_Res  =>	SCLR
	  );


SCU_Slave : scu_bus_slave
generic map (
	      CLK_in_Hz		=>  CLK_in_Hz,
	      Firmware_Release	=>  0,
	      Firmware_Version	=>  0,
	      CID_System       	=>  55,
	      CID_Group       	=>  0,
	      Intr_Edge_Trig	=>  "111111111111110",
	      Intr_Enable	=>  "000000000000001",
	      Intr_Level_Neg	=>  "000000000000000",
	      Slave_ID		=>  SCU_SIO2_ID
	    )
port map  (
	    SCUB_Addr           =>  A_A,
	    SCUB_Data           =>  A_D,
	    nSCUB_DS            =>  A_nDS,
	    SCUB_RDnWR	        =>  A_RnW,
	    clk                 =>  clk,
	    nSCUB_Slave_Sel     =>  A_nBoardSel,
	    nSCUB_Reset_in      =>  A_nReset,
	    nSCUB_Timing_Cyc    =>  A_nEvent_Str,
	    Dtack_to_SCUB       =>  Dtack_to_SCUB,
	    User_Ready	        =>  DFF_inst10,
	    Data_to_SCUB        =>  Data_to_SCUB,
	    Intr_In             =>  "000000000000000",
	    nSCUB_Dtack_Opdrn   =>  open,
	    SCUB_Dtack	        =>  SCU_Dtack,
	    nSCUB_SRQ_Opdrn     =>  open,
	    SCUB_SRQ            =>  SCUB_SRQ,
	    nSel_Ext_Data_Drv   =>  A_nSel_Ext_Data_Drv,
	    Ext_Data_Drv_Rd     =>  A_Ext_Data_RD,
	    ADR_from_SCUB_LA    =>  ADR_from_SCUB_LA,
	    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,
	    Ext_Adr_Val	        =>  Ext_Adr_Val,
	    Ext_Rd_active       =>  Ext_Rd_active,
	    Ext_Rd_fin	        =>  Ext_Rd_fin,
	    Ext_Rd_Fin_ovl      =>  open,
	    Ext_Wr_active       =>  Ext_Wr_active,
	    Ext_Wr_fin	        =>  Ext_Wr_fin,
	    Ext_Wr_Fin_ovl      =>  open,
	    Timing_Pattern_LA   =>  Timing_Pattern_LA,
	    Timing_Pattern_RCV	=>  Timing_Pattern_RCV
	);

Dtack_to_SCUB <=  user_reg1_Dtack or fg1_Dtack_to_SCUB;

A_nDtack <= NOT(SCU_Dtack);
A_nSRQ <= NOT(SCUB_SRQ);


zeit1 : zeitbasis
generic map (
	      CLK_in_Hz =>  CLK_in_Hz,
	      diag_on 	=>  1
	    )
port map  (
	    Res             =>  SCLR,
	    Clk             =>  Clk,
	    Ena_every_100ns =>  Ena_Every_100ns,
	    Ena_every_166ns =>  Ena_Every_166ns,
	    Ena_every_250ns =>  Ena_Every_250ns,
	    Ena_every_500ns =>  Ena_Every_500ns,
	    Ena_every_1us   =>  open,
	    Ena_Every_20ms  =>  Ena_Every_20ms
	);

end arch_scu_sio;
