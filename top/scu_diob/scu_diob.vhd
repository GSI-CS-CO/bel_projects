library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.wb_scu_reg_pkg.all;
use work.fg_quad_pkg.all;




----------------------------------------------------------------------------------------------------------------------
--  Vers: 1 Revi: 0: erstellt am 28.01.2014, Autor: R.Hartmann                                                      --
--                                                                                                                  --
--                                                                                                                  --
--    Config-Register-Layout                                                                                        --
--                                                                                                                  --
--      Base_addr   : Config-Register-Layout                                                                        --
--                                ------+-----------------------------------------------------------------------    --
--                     Lesen Bit:   15  | Leiterplatten-Test-Mode;  1 = Test                                        --
--                                      |                           0 = Betrieb                                     --
--                                ------+-----------------------------------------------------------------------    --
--																      |                                                                           --
--																14..5 | immer '0'                                                                 --
--																      |                                                                           --
---                                ------+-----------------------------------------------------------------------   --
--                                  4   | FG_mode;  1 = Funktiongenerator-Mode, DAC-Werte kommen von FG_Data und    --
--                                      |               werden mit FG_Strobe uebernommen. Kein externer Trigger!    --
--                                      |           0 = Software-Mode, DAC-Werte, kommen vom SCU-Bus-Slave.         --
--                                      |               Externe Triggerung mit pos. oder neg. Flanke, kann einge-   --
--                                      |               schaltet werden.                                            --
--                                ------+-----------------------------------------------------------------------    --
--                                  3   | dac_neg_edge_conv;  1 = neg. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                      |                     0 = pos. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                ------+-----------------------------------------------------------------------    --
--                                  2   | dac_conv_extern;    1 = externer Trigger ist selektiert                   --
--                                      |                     0 = direkt nach der seriellen Uebertragung, wird der  --
--                                      |                         DAC-Wert eingestellt.                             --
--                                ------+-----------------------------------------------------------------------    --
--                                  1   | CLR_DAC_active;   1 = der Reset des DACs ist noch nicht beendet (200ns).  --
--                                ------+-----------------------------------------------------------------------    --
--                                  0   |               --
--                                ------+-----------------------------------------------------------------------    --
--                                                                                                                  --
--                  
--                                ------+-----------------------------------------------------------------------    --
--                 Schreiben Bit:   15  | Leiterplatten-Test-Mode;  1 = Test                                        --
--                                      |                           0 = Betrieb                                     --
--                                ------+-----------------------------------------------------------------------    --
--																      |                                                                           --
--																14..5 | kein Einfluss                                                             --
--																      |                                                                           --
--                                ------+-----------------------------------------------------------------------    --
--                                  4   | FG_mode;  1 = Funktiongenerator-Mode                                      --
--                                ------+-----------------------------------------------------------------------    --
--                                  4   | FG_mode;  1 = Funktiongenerator-Mode                                      --
--                                      |           0 = Software-Mode                                               --
--                                      |           Ein Wechsel dieses Bits hat immer einen Reset des gesamten      --
--                                      |           dac714-Makros zur Folge. D.h. beim Umschalten von FG-Mode auf   --
--                                      |           SW-Mode kann nicht der exterene Trigger und die entsprechende   --
--                                      |           Trigger-Flanke vorgegen werden, weil waehrend des Resets        --
--                                      |           diese Bits auf null gesetzt werden. Nachdem der Reset beendet   --
--                                      |           ist (CLR_DAC_active = 0), koennen die Bits entsprechend gesetzt --
--                                      |           werden.                                                         --
--                                ------+-----------------------------------------------------------------------    --
--                                  3   | dac_neg_edge_conv;  1 = neg. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                      |                         Laesst sich nur im SW-Mode setzen.                --
--                                      |                     0 = pos. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                ------+-----------------------------------------------------------------------    --
--                                  2   | dac_conv_extern;    1 = externer Trigger wird selektiert                  --
--                                      |                         Laesst sich nur im SW-Mode setzen.                --
--                                      |                     0 = direkt nach der seriellen Uebertragung, wird      --
--                                      |                         der DAC-Wert eingestellt.                         --
--                                ------+-----------------------------------------------------------------------    --
--                                  1   | CLR_DAC;    1 = ein Reset des DACs wird ausgefuehrt (200ns)               --
--                                ------+-----------------------------------------------------------------------    --
--                                  0   |  --
--                                      |  --
--                                      |  --
--                                ------+-----------------------------------------------------------------------    --
--                                                                                                                  --
----------------------------------------------------------------------------------------------------------------------



entity scu_diob is
generic (
		CLK_sys_in_Hz:			integer := 125000000
		);
port	(
		------------------------------ Clocks -------------------------------------------------------------------------
		CLK_20MHz_A: in std_logic; -- Clock_A
		CLK_20MHz_B: in std_logic; -- Clock_B
		CLK_20MHz_C: in std_logic; -- Clock_C
		CLK_20MHz_D: in std_logic; -- Clock_D
   
    --------- Parallel SCU-Bus-Signale ----------------------------------------------------------------------------
    A_A: in std_logic_vector(15 downto 0); -- SCU-Adressbus
    A_nADR_EN: out std_logic := '0'; -- '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_FROM_SCUB: out std_logic := '0'; -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave
    A_D: inout std_logic_vector(15 downto 0); -- SCU-Datenbus
    A_nDS: in std_logic; -- Data-Strobe vom Master gertieben
    A_RnW: in std_logic; -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv: out std_logic; -- '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD: out std_logic; -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack: out std_logic; -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nSRQ: out std_logic; -- Service-Request null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nBoardSel: in std_logic; -- '0' => Master aktiviert diesen Slave
    A_nEvent_Str: in std_logic; -- '0' => Master sigalisiert Timing-Zyklus
    A_SysClock: in std_logic; -- Clock vom Master getrieben.
    A_Spare0: in std_logic; -- vom Master getrieben
    A_Spare1: in std_logic; -- vom Master getrieben
    A_nReset: in std_logic; -- Reset (aktiv '0'), vom Master getrieben

		A_nSEL_Ext_Signal_DRV: out std_logic; -- '0' => Treiber fr SCU-Bus-Steuer-Signale aktiv
		A_nExt_Signal_In: out std_logic; -- '0' => Treiber fr SCU-Bus-Steuer-Signale-Richtung: SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)

		----------------- OneWire ----------------------------------------------------------------------------------------
		A_OneWire: inout std_logic; -- Temp.-OneWire auf dem Slave
		
    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL: in std_logic_vector(3 downto 0); -- use to select sources for the logic analyser ports
		A_Tclk: out std_logic; -- Clock  for Logikanalysator Port A
    A_TA: inout std_logic_vector(15 downto 0); -- test port a

		---------------------------------- Diagnose-LED's -----------------------------------------------------------------
		A_nLED_D2: out std_logic;	-- Diagnose-LED_D2 auf dem Basis-Board
		A_nLED_D3: out std_logic;	-- Diagnose-LED_D3 auf dem Basis-Board

		------------ User I/O zur VG-Leiste -------------------------------------------------------------------------------
		A_nUser_EN: out std_logic; -- Enable User-I/O
		UIO: inout std_logic_vector(15 downto 0); -- User I/O VG-Leiste
	
		---------------- bergabestecker fr Anwender-I/O -----------------------------------------------------------------
		CLK_IO: in std_logic; -- Clock vom Anwender_I/0
		PIO: inout std_logic_vector(150 downto 16)	-- Dig. User I/0 to Piggy
		);
end scu_diob;



architecture scu_diob_arch of scu_diob is


constant scu_diob1_id: integer range 16#0220# to 16#022F# := 16#0220#;


--	+============================================================================================================================+
--	| 																Anfang: Component    																		  |
--	+============================================================================================================================+


component aw_io_reg
generic (
		Base_addr : integer
		);
port (Adr_from_SCUB_LA		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		Data_from_SCUB_LA		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		Ext_Adr_Val		:	 IN STD_LOGIC;
		Ext_Rd_active		:	 IN STD_LOGIC;
		Ext_Rd_fin		:	 IN STD_LOGIC;
		Ext_Wr_active		:	 IN STD_LOGIC;
		Ext_Wr_fin		:	 IN STD_LOGIC;
		clk		:	 IN STD_LOGIC;
		nReset		:	 IN STD_LOGIC;
		AWIn1		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWIn2		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWIn3		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWIn4		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWIn5		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWIn6		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		AW_Config			:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg1		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg2		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg3		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg4		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg5		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AWOut_Reg6		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		AW_Config_Wr	:	 OUT STD_LOGIC;	 
		AWOut_Reg1_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg2_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg3_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg4_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg5_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg6_Wr	:	 OUT STD_LOGIC;	 	
		AWOut_Reg_rd_active		:	 OUT STD_LOGIC;
		Data_to_SCUB		:	 OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		Dtack_to_SCUB		:	 OUT STD_LOGIC
	);
end component aw_io_reg;


component flash_loader_v01
port	(
		noe_in:		in		std_logic
		);
end component;


component pll_sio
port	(
		inclk0:		in		std_logic;
		c0:			out		std_logic;
		c1:			out		std_logic;
		locked:		out		std_logic
		);
end component;


component SysClock
	PORT
	(
		areset		: IN STD_LOGIC  := '0';
		inclk0		: IN STD_LOGIC  := '0';
		inclk1		: IN STD_LOGIC  := '0';
		activeclock		: OUT STD_LOGIC ;
		c0		: OUT STD_LOGIC ;
		c1		: OUT STD_LOGIC ;
		c2		: OUT STD_LOGIC ;
		clkbad0		: OUT STD_LOGIC ;
		clkbad1		: OUT STD_LOGIC ;
		locked		: OUT STD_LOGIC 
	);
end component;


component pu_reset
generic	(
		PU_Reset_in_clks : integer
		);
port	(
		Clk:		in		std_logic;
		PU_Res:		out		std_logic
		);
end component;



component zeitbasis
generic	(
		CLK_in_Hz:			integer;
		diag_on:			integer
		);
port	(
		Res:				in	std_logic;
		Clk:				in	std_logic;
		Ena_every_100ns:	out	std_logic;
		Ena_every_166ns:	out	std_logic;
		Ena_every_250ns:	out	std_logic;
		Ena_every_500ns:	out	std_logic;
		Ena_every_1us:		out	std_logic;
		Ena_Every_20ms:		out	std_logic
		);
end component;


--	+============================================================================================================================+
--	| 																	Ende: Component    																		  |
--	+============================================================================================================================+



constant c_xwb_owm : t_sdb_device := (
    abi_class => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian => c_sdb_endian_big,
    wbd_width => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first => x"0000000000000000",
    addr_last => x"00000000000000ff",
    product => (
    vendor_id => x"000000000000CE42", -- CERN
    device_id => x"779c5443",
    version => x"00000001",
    date => x"20120603",
    name => "WR-Periph-1Wire    ")));
        
constant c_xwb_uart : t_sdb_device := (
    abi_class => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian => c_sdb_endian_big,
    wbd_width => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first => x"0000000000000000",
    addr_last => x"00000000000000ff",
    product => (
    vendor_id => x"000000000000CE42", -- CERN
    device_id => x"e2d13d04",
    version => x"00000001",
    date => x"20120603",
    name => "WR-Periph-UART     ")));
  
  signal clk_sys, clk_cal, locked : std_logic;

  signal SCUB_SRQ: std_logic;
  signal SCUB_Dtack: std_logic;
  signal convst: std_logic;
  signal rst: std_logic;
  
  signal Dtack_to_SCUB: std_logic;
  
  signal ADR_from_SCUB_LA: std_logic_vector(15 downto 0);
  signal Data_from_SCUB_LA: std_logic_vector(15 downto 0);
  signal Ext_Adr_Val: std_logic;
  signal Ext_Rd_active: std_logic;
  signal Ext_Wr_active: std_logic;
  signal Ext_Wr_fin_ovl: std_logic;
  signal SCU_Ext_Wr_fin: std_logic;
  signal nPowerup_Res: std_logic;

  signal extension_cid_system:  std_logic_vector(15 downto 0); -- in,	extension card: cid_system
  signal extension_cid_group:  std_logic_vector(15 downto 0); --in, extension card: cid_group
  signal DAC_Out: std_logic_vector(15 downto 0);
 
  signal fg_1_dtack: std_logic;
  signal fg_1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal fg_1_rd_active: std_logic;
  signal fg_1_sw: std_logic_vector(31 downto 0);
  signal fg_1_strobe: std_logic;
  signal fg_1_dreq: std_logic;
  
  signal led_ena_cnt: std_logic;

  signal Data_to_SCUB: std_logic_vector(15 downto 0);
  
  signal reset_clks : std_logic_vector(0 downto 0);
  signal reset_rstn : std_logic_vector(0 downto 0);
  signal clk_sys_rstn : std_logic;
  signal lm32_interrupt : std_logic_vector(31 downto 0);
  signal lm32_rstn : std_logic;
  
  -- Top crossbar layout
  constant c_slaves : natural := 4;
  constant c_masters : natural := 2;
  constant c_dpram_size : natural := 32768; -- in 32-bit words (64KB)
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size), x"00000000"),
    1 => f_sdb_embed_device(c_xwb_owm, x"00100600"),
    2 => f_sdb_embed_device(c_xwb_uart, x"00100700"),
    3 => f_sdb_embed_device(c_xwb_scu_reg, x"00100800"));
  constant c_sdb_address : t_wishbone_address := x"00100000";

  signal cbar_slave_i : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  signal owr_pwren_o: std_logic_vector(1 downto 0);
  signal owr_en_o: std_logic_vector(1 downto 0);
  signal owr_i:        std_logic_vector(1 downto 0);
  
  signal wb_scu_rd_active: std_logic;
  signal wb_scu_dtack: std_logic;
  signal wb_scu_data_to_SCUB: std_logic_vector(15 downto 0);
  
  signal irqcnt:  unsigned(12 downto 0);
  

	signal Powerup_Res: std_logic; -- only for modelsim!
	signal Powerup_Done: std_logic;     -- this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
	signal WRnRD: std_logic; -- only for modelsim!

  signal clk, la_clk: std_logic;
	signal Deb_SCUB_Reset_out: std_logic;
	signal Standard_Reg_Acc:		std_logic;
	signal Ext_Rd_fin: std_logic;
	signal Ext_Wr_fin: std_logic;
	signal pll_1_locked: std_logic;
	signal pll_2_locked: std_logic;
	signal clkbad0, clkbad1: std_logic;
	signal SysClock_pll_out: 	std_logic;

	
	signal Ena_Every_100ns: std_logic;
	signal Ena_Every_166ns: std_logic;
	signal Ena_Every_20ms: std_logic;
	signal Ena_Every_1us: std_logic;
	signal Ena_Every_250ms: std_logic;
	signal Ena_Every_500ms: std_logic;
 
	signal F_8_MHz: std_logic;
	signal F_12p5_MHz: std_logic;
	 
	signal test_port_in_0: std_logic_vector(15 downto 0);
	signal test_clocks: std_logic_vector(15 downto 0);
	
	signal s_nLED_Sel: std_logic; -- LED = Sel
	signal s_nLED_Dtack: std_logic; -- LED = Dtack
	signal s_nLED_INR: std_logic; -- LED = Interrupt
	signal s_nLED_PU: std_logic; -- LED = Powerup_Reset
	 
	signal s_nLED: std_logic_vector(7 downto 0); -- LED's
	signal s_nLED_Out: std_logic_vector(7 downto 0); -- LED's
	signal s_AW_ID: std_logic_vector(7 downto 0); -- Anwender_ID
	 
	signal AWIn1: std_logic_vector(15 downto 0);
	signal AWIn2: std_logic_vector(15 downto 0);
	signal AWIn3: std_logic_vector(15 downto 0);
	signal AWIn4: std_logic_vector(15 downto 0);
	signal AWIn5: std_logic_vector(15 downto 0);
	signal AWIn6: std_logic_vector(15 downto 0);
	signal AW_Config: std_logic_vector(15 downto 0);
	signal AWOut_Reg1: std_logic_vector(15 downto 0);
	signal AWOut_Reg2: std_logic_vector(15 downto 0);
	signal AWOut_Reg3: std_logic_vector(15 downto 0);
	signal AWOut_Reg4: std_logic_vector(15 downto 0);
	signal AWOut_Reg5: std_logic_vector(15 downto 0);
	signal AWOut_Reg6: std_logic_vector(15 downto 0);
	signal AW_Config_Wr:  std_logic;	
	signal AWOut_Reg1_Wr: std_logic;
	signal AWOut_Reg2_Wr: std_logic;
	signal AWOut_Reg3_Wr: std_logic;
	signal AWOut_Reg4_Wr: std_logic;
	signal AWOut_Reg5_Wr: std_logic;
	signal AWOut_Reg6_Wr: std_logic;
	signal AWOut_Reg_rd_active: std_logic;
	signal aw_port1_Dtack: std_logic;
	signal aw_port1_data_to_SCUB: std_logic_vector(15 downto 0);
	 
	 
	signal IO_Test_Port0: std_logic_vector(15 downto 0);
	signal IO_Test_Port1: std_logic_vector(15 downto 0);
	signal IO_Test_Port2: std_logic_vector(15 downto 0);
	signal IO_Test_Port3: std_logic_vector(15 downto 0);
	
	
	signal s_deb_In1:  std_logic;
	signal s_deb_In2:  std_logic;
	signal s_deb_In3:  std_logic;
	signal s_deb_In4:  std_logic;
	signal s_deb_Out1: std_logic;
	signal s_deb_Out2: std_logic;
	signal s_deb_Out3: std_logic;
	signal s_deb_Out4: std_logic;
	 
	signal s_Led_Bu1: std_logic;
	signal s_Led_Bu2: std_logic;
	signal s_Led_Bu3: std_logic;
	signal s_Led_Bu4: std_logic;
	signal s_Led_Bu1_Out: std_logic;
	signal s_Led_Bu2_Out: std_logic;
	signal s_Led_Bu3_Out: std_logic;
	signal s_Led_Bu4_Out: std_logic;
	 
	signal s_str_shift1: STD_LOGIC_VECTOR(2  DOWNTO 0);	-- Shift-Reg.
	signal s_str_In1: STD_LOGIC;								-- Signal-Input
	signal s_str_Out1: STD_LOGIC;								-- Strobe-Output
	signal s_str_shift2: STD_LOGIC_VECTOR(2  DOWNTO 0);	-- Shift-Reg.
	signal s_str_In2: STD_LOGIC;								-- Signal-Input
	signal s_str_Out2: STD_LOGIC;								-- Strobe-Output
	signal s_str_shift3: STD_LOGIC_VECTOR(2  DOWNTO 0);	-- Shift-Reg.
	signal s_str_In3: STD_LOGIC;								-- Signal-Input
	signal s_str_Out3: STD_LOGIC;								-- Strobe-Output
	signal s_str_shift4: STD_LOGIC_VECTOR(2  DOWNTO 0);	-- Shift-Reg.
	signal s_str_In4:	STD_LOGIC;								-- Signal-Input
	signal s_str_Out4: STD_LOGIC;								-- Strobe-Output
	 
	signal s_str_shift_EE_20ms:	STD_LOGIC_VECTOR(2  DOWNTO 0);	-- Shift-Reg.
	signal s_str_EE_20ms:	STD_LOGIC;								-- Strobe-Output
	 
	 
	signal dff1_Reset: std_logic;
	signal dff1_clk_En: std_logic;
	signal dff1_d: std_logic;
	signal dff1_q: std_logic;
	 
	signal clk_blink: std_logic;
	
	
	
	constant	C_Debounce_Input_in_ns: integer := 5000;
	constant	Clk_in_ns: integer:= 1000000000/clk_sys_in_Hz;
	constant	stretch_cnt: integer := 5;
	

	CONSTANT c_AW_P37IO: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000001";	-- FG900_700
	CONSTANT c_AW_P25IO: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000010";	-- FG900_710
	CONSTANT c_AW_OCIN: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000011";	---- FG900_720
	CONSTANT c_AW_OCIO: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000100";	---- FG900_730
	CONSTANT c_AW_UIO: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000101";	---- FG900_740
	CONSTANT c_AW_DA: STD_LOGIC_VECTOR(7 DOWNTO 0):= B"00000110";	------ FG900_750


--	+============================================================================================================================+
--	| 																			Begin   		 																		  |
--	+============================================================================================================================+


  begin


	A_nADR_EN 						<= '0';
	A_nADR_FROM_SCUB 			<= '0';
	A_nExt_Signal_In 			<= '0';
	A_nSEL_Ext_Signal_DRV <= '0';
	A_nUser_EN 						<= '0';
	

	Powerup_Res <= not nPowerup_Res;  -- only for modelsim!
	WRnRD       <= not A_RnW;  				-- only for modelsim!


			
aw_port1: aw_io_reg			
generic map(
			Base_addr	=>	16#200#
			)
port map	(			
			Adr_from_SCUB_LA		=>	ADR_from_SCUB_LA,
			Data_from_SCUB_LA		=>	Data_from_SCUB_LA,
			Ext_Adr_Val					=>	Ext_Adr_Val,
			Ext_Rd_active				=>	Ext_Rd_active,
			Ext_Rd_fin					=>	Ext_Rd_fin,
			Ext_Wr_active				=>	Ext_Wr_active,
			Ext_Wr_fin					=>	Ext_Wr_fin,
			clk									=>	clk_sys,
			nReset							=>	nPowerup_Res,
			AWIn1								=>	AWIn1,
			AWIn2								=>	AWIn2,
			AWIn3								=>	AWIn3,
			AWIn4								=>	AWIn4,
			AWIn5								=>	AWIn5,
			AWIn6								=>	AWIn6,
			AW_Config						=>	AW_Config,
			AWOut_Reg1					=>	AWOut_Reg1,
			AWOut_Reg2					=>	AWOut_Reg2,
			AWOut_Reg3					=>	AWOut_Reg3,
			AWOut_Reg4					=>	AWOut_Reg4,
			AWOut_Reg5					=>	AWOut_Reg5,
			AWOut_Reg6					=>	AWOut_Reg6,
			AW_Config_Wr				=>	AW_Config_Wr,	
			AWOut_Reg1_Wr				=>	AWOut_Reg1_Wr,	
			AWOut_Reg2_Wr				=>	AWOut_Reg2_Wr,
			AWOut_Reg3_Wr				=>	AWOut_Reg3_Wr,	
			AWOut_Reg4_Wr				=>	AWOut_Reg4_Wr,	
			AWOut_Reg5_Wr				=>	AWOut_Reg5_Wr,	
			AWOut_Reg6_Wr				=>	AWOut_Reg6_Wr,	
			AWOut_Reg_rd_active	=>	AWOut_Reg_rd_active,
			Dtack_to_SCUB				=>	aw_port1_Dtack,
			Data_to_SCUB				=>	aw_port1_data_to_SCUB
			);
			
	
    timer_irq: process (clk_sys, reset_rstn)
    begin
      if reset_rstn = "0" then
        irqcnt <= '0' & x"FFF";
      elsif rising_edge(clk_sys) then
        if irqcnt(irqcnt'high) = '1' then
          irqcnt <= '0' & x"FFF";
        else
          irqcnt <= irqcnt - 1;
        end if;
      end if;
    end process;



		
testport_mux:	process	(A_SEL, AW_Config, AWIn1, AWOut_Reg1,
                       IO_Test_Port0, IO_Test_Port1, IO_Test_Port2, IO_Test_Port3,
											 test_port_in_0, test_clocks)
variable test_out: std_logic_vector(15 downto 0);
begin
	case (not A_SEL) is
		when X"0" => test_out := AW_Config;
		when X"1" => test_out := AWOut_Reg1;
		when X"2" => test_out := AWIn1;
		when X"3" => test_out := X"0000";
--
		when X"4" => test_out := IO_Test_Port0;
		when X"5" => test_out := IO_Test_Port1;
		when X"6" => test_out := IO_Test_Port2;
		when X"7" => test_out := X"0000";
--
		when X"8" => test_out := X"0000";
		when X"9" => test_out := X"0000";
		when X"A" => test_out := X"0000";
		when X"B" => test_out := X"0000";
--
		when X"C" => test_out := X"0000";
		when X"D" => test_out := X"0000";
		when X"E" => test_out := test_clocks;
		when X"F" => test_out := test_port_in_0;
		when others =>	test_out := (others => '0');
	end case;
	--A_TA <= test_out(15 downto 0);
end process testport_mux;



A_Tclk	<=	 la_clk;	-- Clock fr Logikanalysator: = Sysclk x 2 				

test_port_in_0 <=	nPowerup_Res 	& clk 						& Ena_Every_100ns & Ena_Every_166ns &	-- bit15..12
									Ext_Wr_active & SCU_Ext_Wr_fin  & AWOut_Reg1_wr		& fg_1_strobe			& -- bit11..8
									la_clk 			  & pll_2_locked		& A_RnW & A_nDS		&										-- bit7..4
									A_nBoardSel   & fg_1_strobe 		& '0' 	& SCUB_Dtack									-- bit3..0
									;

						
test_clocks <=	X"00" &
						A_SysClock 			&	CLK_20MHz_D 	& clkbad0			&	clkbad1 		&		-- bit7..4					
						pll_2_locked		&	clk 					&	F_8_MHz 		&	la_clk 					-- bit3..0
						;					

IO_Test_Port1 <=	x"00" &	s_AW_ID(7 DOWNTO 0);-- Anwender_ID
IO_Test_Port2 <=	AWOut_Reg1(15 DOWNTO 0);
IO_Test_Port3 <=	AWOut_Reg2(15 DOWNTO 0);

						

--rd_port_mux:	process	(fg1_rd_cycle, fg1_data_to_SCUB, User_Reg_rd_active, AWOut_Reg_rd_active, user_reg1_data_to_SCUB, aw_port1_data_to_SCUB)
--begin
--	if fg1_rd_cycle = '1' then
--		Data_to_SCUB <= fg1_data_to_SCUB;
--	elsif User_Reg_rd_active = '1' then
--		Data_to_SCUB <= user_reg1_data_to_SCUB;
--		elsif AWOut_Reg_rd_active = '1' then
--		Data_to_SCUB <= aw_port1_data_to_SCUB;
--	else
--		Data_to_SCUB <= (others => '-');
--	end if;
--end process rd_port_mux;
--


--p_ready:	process(clk, nPowerup_Res, Deb_SCUB_Reset_out)
--begin
--	if (nPowerup_Res = '0') or (Deb_SCUB_Reset_out = '1') then
--		ready <= '0';
--	elsif rising_edge(clk) then
--		ready <= '1';
--	end if;
--end process;

						
fl : flash_loader_v01
port map	(
			noe_in	=>	'0'
			);
	
	PLL_1: pll_sio
port map	(
			inclk0	=>	CLK_20MHz_D,
			c0		=>	F_12p5_MHz,				-- clk,
			c1		=>	open,							-- la_clk,
			locked	=>	pll_1_locked
			);


PLL_2: SysClock
port map	(
			areset		=>	'0',
			inclk0		=>	A_SysClock,		-- 12,5 Mhz vom SCU-Bus
			inclk1		=>	F_12p5_MHz,		-- 12,5 Mhz von PLL1 aus 20MHz vom Onboard Oszillator
			activeclock => open,
			c0				=>	clk_sys,						
			c1				=>	F_8_MHz,
			c2				=>	la_clk,
			clkbad0		=>	clkbad0,
			clkbad1		=>	clkbad1,
			
			locked		=>	pll_2_locked
			);
		
	
		
  -- open drain buffer for one wire
        owr_i(0) <= A_OneWire;
        A_OneWire <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';
  
  -- The top-most Wishbone B.4 crossbar
  interconnect : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves => c_slaves,
     g_registered => true,
     g_wraparound => false, -- Should be true for nested buses
     g_layout => c_layout,
     g_sdb_addr => c_sdb_address)
   port map(
     clk_sys_i => clk_sys,
     rst_n_i => nPowerup_Res,
     -- Master connections (INTERCON is a slave)
     slave_i => cbar_slave_i,
     slave_o => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i => cbar_master_i,
     master_o => cbar_master_o);
     
  -- The LM32 is master 0+1
  LM32 : xwb_lm32
    generic map(
      g_profile => "medium_icache_debug") -- Including JTAG and I-cache (no divide)
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => nPowerup_Res,
      irq_i => lm32_interrupt,
      dwb_o => cbar_slave_i(0), -- Data bus
      dwb_i => cbar_slave_o(0),
      iwb_o => cbar_slave_i(1), -- Instruction bus
      iwb_i => cbar_slave_o(1));
  -- The other 31 interrupt pins are unconnected
  lm32_interrupt(31 downto 1) <= (others => '0');
  
  -- WB Slave 0 is the RAM
  ram : xwb_dpram
    generic map(
      g_size => c_dpram_size,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity => BYTE,
      g_slave2_granularity => WORD,
      g_init_file => "scu_diob.mif")
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => nPowerup_Res,
      -- First port connected to the crossbar
      slave1_i => cbar_master_o(0),
      slave1_o => cbar_master_i(0),
      -- Second port disconnected
      slave2_i => cc_dummy_slave_in, -- CYC always low
      slave2_o => open);
      
      
  --------------------------------------
  -- 1-WIRE
  --------------------------------------
  ONEWIRE : xwb_onewire_master
    generic map(
      g_interface_mode => PIPELINED,
      g_address_granularity => BYTE,
      g_num_ports => 2,
      g_ow_btp_normal => "5.0",
      g_ow_btp_overdrive => "1.0"
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => nPowerup_Res,

      -- Wishbone
      slave_i => cbar_master_o(1),
      slave_o => cbar_master_i(1),
      desc_o => open,

                owr_pwren_o => owr_pwren_o,
      owr_en_o => owr_en_o,
      owr_i => owr_i
      );
  --------------------------------------
  -- UART
  --------------------------------------
  UART : xwb_simple_uart
    generic map(
      g_with_virtual_uart => false,
      g_with_physical_uart => true,
      g_interface_mode => PIPELINED,
      g_address_granularity => BYTE
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => nPowerup_Res,

      -- Wishbone
      slave_i => cbar_master_o(2),
      slave_o => cbar_master_i(2),
      desc_o => open,

      uart_rxd_i => '0',
      uart_txd_o => A_TA(0)
      );
  
  SCU_WB_Reg: wb_scu_reg
    generic map (
      Base_addr => x"0050",
      register_cnt => 16 )
    port map (
      clk_sys_i => clk_sys,
      rst_n_i => nPowerup_Res,

      -- Wishbone
      slave_i => cbar_master_o(3),
      slave_o => cbar_master_i(3),
      
      Adr_from_SCUB_LA => ADR_from_SCUB_LA,
      Data_from_SCUB_LA => Data_from_SCUB_LA,
      Ext_Adr_Val => Ext_Adr_Val,
      Ext_Rd_active => Ext_Rd_active,
      Ext_Wr_active => Ext_Wr_active,
      user_rd_active => wb_scu_rd_active,
      Data_to_SCUB => wb_scu_data_to_SCUB,
      Dtack_to_SCUB => wb_scu_dtack );

    

zeit1 : zeitbasis
generic map	(
			CLK_in_Hz	=>	clk_sys_in_Hz,
			diag_on		=>	1
			)
port map	(
			Res					=>	Powerup_Res,
			Clk					=>	clk_sys,
			Ena_every_100ns		=>	Ena_Every_100ns,
			Ena_every_166ns		=>	Ena_Every_166ns,
			Ena_every_250ns		=>	open,
			Ena_every_500ns		=>	open,
			Ena_every_1us			=>	Ena_every_1us,
			Ena_Every_20ms		=>	Ena_Every_20ms
			);


p_led_sel: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys, Sig_In => not A_nBoardSel, nLED => s_nLED_Sel);-- LED: sel Board
	
p_led_dtack: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,	Sig_In => SCUB_Dtack, nLED => s_nLED_Dtack);-- LED: Dtack to SCU-Bus

p_led_inr: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,	Sig_In => SCUB_SRQ, nLED => s_nLED_INR);-- LED: Interrupt

p_led_pu: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,	Sig_In => not (nPowerup_Res), nLED => s_nLED_PU);-- LED: nPowerup_Reset


	
A_nLED_D2	<=	 s_nLED_Sel;		-- Diagnose-LED_D2 = BoardSelekt	
A_nLED_D3	<=	 s_nLED_Dtack;	-- Diagnose-LED_D3 = Dtack




sel_every_250ms: div_n
  generic map (n => 12, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 13x20ms = 260ms
    port map  ( res => Powerup_Res,
                clk => clk_sys,
                ena => Ena_Every_20ms,
                div_o => ENA_every_250ms
              );
							
sel_every_500ms: div_n
  generic map (n => 25, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 25x20ms = 500ms
    port map  ( res => Powerup_Res,
                clk => clk_sys,
                ena => Ena_Every_20ms,
                div_o => ENA_every_500ms
              );

							
p_clk_blink:	
process (clk_sys, Powerup_Res, ENA_every_250ms)
begin
	if  ( Powerup_Res  	 = '1') then
			clk_blink 	<= '0';
	elsif (rising_edge(clk_sys)) then
		if (ENA_every_500ms = '1') then
			clk_blink <= not clk_blink;
		end if;
	end if;
end process;
	
						
							
Deb_IN1:	Debounce
	generic map (
				DB_Cnt => C_Debounce_Input_in_ns / clk_in_ns		-- Entprellung (C_Debounce_Input_in_ns / clk_in_ns)
				)
	port map	(
				DB_In => s_deb_In1,
				Reset	=> Powerup_Res,
				clk => clk_sys,	
				DB_Out => s_deb_Out1
				);

Deb_IN2:	Debounce
	GENERIC MAP (DB_Cnt => C_Debounce_Input_in_ns / clk_in_ns)		-- Entprellung (C_Debounce_Input_in_ns / clk_in_ns)
	PORT MAP	(DB_In => s_deb_In2,	Reset	=> Powerup_Res,	clk => clk_sys,	DB_Out => s_deb_Out2);

Deb_IN3:	Debounce
	GENERIC MAP (DB_Cnt => C_Debounce_Input_in_ns / clk_in_ns)		-- Entprellung (C_Debounce_Input_in_ns / clk_in_ns)
	PORT MAP	(DB_In => s_deb_In3,	Reset	=> Powerup_Res,	clk => clk_sys,	DB_Out => s_deb_Out3);

Deb_IN4:	Debounce
	GENERIC MAP (DB_Cnt => C_Debounce_Input_in_ns / clk_in_ns)		-- Entprellung (C_Debounce_Input_in_ns / clk_in_ns)
	PORT MAP	(DB_In => s_deb_In4,	Reset	=> Powerup_Res,	clk => clk_sys,	DB_Out => s_deb_Out4);


			
p_led_Bu1: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,		Sig_In => s_Led_Bu1,		nLED => s_Led_Bu1_Out);-- LED: sel Board
			
p_led_Bu2: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,		Sig_In => s_Led_Bu2,		nLED => s_Led_Bu2_Out);-- LED: sel Board

p_led_Bu3: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,		Sig_In => s_Led_Bu3,		nLED => s_Led_Bu3_Out);-- LED: sel Board

p_led_Bu4: led_n
	generic	map	(stretch_cnt => stretch_cnt)
	port map			(ena => Ena_Every_20ms, CLK => clk_sys,		Sig_In => s_Led_Bu4,		nLED => s_Led_Bu4_Out);-- LED: sel Board
	


						
					
				
			
			
--------- Strobe1 als Input-Signal (1 Clock breit) --------------------

p_strobe1:	PROCESS (clk_sys, Powerup_Res, s_str_In1)
	BEGIN
		IF Powerup_Res 	= '1' THEN
			s_str_shift1	<= (OTHERS => '0');
			s_str_Out1		<= '0';

		ELSIF rising_edge(clk_sys) THEN
			s_str_shift1 <= (s_str_shift1(s_str_shift1'high-1 DOWNTO 0) & (s_str_In1));

			IF s_str_shift1(s_str_shift1'high) = '0' AND s_str_shift1(s_str_shift1'high-1) = '1' THEN
				s_str_Out1 <= '1';
			ELSE
				s_str_Out1 <= '0';
			END IF;
		END IF;
	END PROCESS p_strobe1;


  
dff1:	
process (clk_sys, dff1_clk_En, dff1_Reset, Powerup_Res)
begin
	-- Reset whenever the reset signal goes low, regardless of the clock
	-- or the clock enable
	if  ( Powerup_Res  	 = '1') then
			dff1_q 	<= '0';
	elsif ( dff1_Reset = '1') then
			dff1_q 	<= '0';

			-- If not resetting, and the clock signal is enabled on this register, 
	-- update the register output on the clock's rising edge
	elsif (rising_edge(clk_sys)) then
		if (dff1_clk_En = '1') then
			dff1_q <= dff1_d;
		end if;
	end if;
end process;
	



SCU_Slave: SCU_Bus_Slave
generic map (
    CLK_in_Hz => clk_sys_in_Hz,
    Firmware_Release        => 0,
    Firmware_Version        => 1,
    CID_System => 55, -- important: 55 => CSCOHW
    CID_Group => 26, --- important: 26 => "FG900500_SCU_Diob1"
    Intr_Enable          => b"0000_0000_0000_0001",
    Slave_ID => scu_diob1_id)
port map (
    SCUB_Addr => A_A, -- in,        SCU_Bus: address bus
    nSCUB_Timing_Cyc         => A_nEvent_Str, -- in,        SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data => A_D, -- inout,        SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel => A_nBoardSel, -- in, SCU_Bus: '0' => SCU master select slave
    nSCUB_DS => A_nDS, -- in,        SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR => A_RnW, -- in, SCU_Bus: '1' => SCU master read slave
    clk => clk_sys,
    nSCUB_Reset_in => A_nReset, -- in,        SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB => Data_to_SCUB, -- in,        connect read sources from external user functions
    Dtack_to_SCUB => Dtack_to_SCUB, -- in,        connect Dtack from from external user functions
    Intr_In => "0000000000000" & irqcnt(irqcnt'high) & fg_1_dreq, -- in,        interrupt(15 downto 1)
    User_Ready => '1',
    extension_cid_system => extension_cid_system, -- in,	extension card: cid_system
		extension_cid_group => extension_cid_group, --in,	    extension card: cid_group
		Data_from_SCUB_LA => Data_from_SCUB_LA, -- out,        latched data from SCU_Bus for external user functions
    ADR_from_SCUB_LA => ADR_from_SCUB_LA, -- out,        latched address from SCU_Bus for external user functions
    Timing_Pattern_LA => open, -- out,        latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV => open, -- out,        timing pattern received
    nSCUB_Dtack_Opdrn => open, -- out,        for direct connect to SCU_Bus opendrain signal
                                                    -- '0' => slave give dtack to SCU master
    SCUB_Dtack => SCUB_Dtack, -- out,        for connect via ext. open collector driver
                                                    -- '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn => open, -- out,        for direct connect to SCU_Bus opendrain signal
                                                    -- '0' => slave service request to SCU ma
    SCUB_SRQ => SCUB_SRQ, -- out,        for connect via ext. open collector driver
                                                    -- '1' => slave service request to SCU master
    nSel_Ext_Data_Drv => A_nSel_Ext_Data_Drv, -- out,        '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd         => A_Ext_Data_RD, -- out,        '1' => direction of the external data driver on the
                                                    -- SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc => Standard_Reg_Acc, -- out,        '1' => mark the access to register of this macro
    Ext_Adr_Val => Ext_Adr_Val, -- out,        for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active => Ext_Rd_active, -- out,        '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin => open, -- out,        marks end of read cycle, active one for one clock period
                                                    -- of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl => open, -- out,        marks end of read cycle, active one for one clock period
                                                    -- of clk during cycle end (overlap)
    Ext_Wr_active => Ext_Wr_active, -- out,        '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin => SCU_Ext_Wr_fin, -- out,        marks end of write cycle, active high for one clock period
                                                    -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl => Ext_Wr_fin_ovl, -- out, marks end of write cycle, active high for one clock period
                                                    -- of clk before write cycle finished (with overlap)
    Deb_SCUB_Reset_out => Deb_SCUB_Reset_out, -- out,        the debounced 'nSCUB_Reset_In'-signal, is active high,
                                                    -- can be used to reset
                                                    -- external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res => nPowerup_Res, -- out,        this macro generated a power up reset
    Powerup_Done => Powerup_Done          -- out  this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
    );


		Dtack_to_SCUB <= wb_scu_dtack or fg_1_dtack or aw_port1_Dtack;


		A_nDtack <= NOT(SCUB_Dtack);
		A_nSRQ 	<= NOT(SCUB_SRQ);


    
fg_1: fg_quad_scu_bus
  generic map (
    Base_addr => x"0300",
    clk_in_hz => clk_sys_in_Hz,
    diag_on_is_1 => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => nPowerup_Res,          -- in, '0' => resets the fg_1
    Rd_Port           => fg_1_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    user_rd_active    => fg_1_rd_active,        -- '1' = read data available at 'Rd_Port'-output
    Dtack             => fg_1_dtack,            -- connect Dtack to SCUB-Macro
    dreq              => fg_1_dreq,             -- request of new parameter set

    -- fg output
    sw_out            => fg_1_sw,               -- 24bit output from fg
    sw_strobe         => fg_1_strobe            -- signals new output data
  );



rd_port_mux:	process	(fg_1_rd_active, AWOut_Reg_rd_active,
											fg_1_data_to_SCUB, aw_port1_data_to_SCUB)  

  variable sel: unsigned(2 downto 0);
  begin
    sel :=  wb_scu_rd_active & fg_1_rd_active  & AWOut_Reg_rd_active;
    case sel IS
      when "001" => Data_to_SCUB <= aw_port1_data_to_SCUB;
      when "010" => Data_to_SCUB <= fg_1_data_to_SCUB;
      when "100" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when others =>
		Data_to_SCUB <= (others => '-');
    end case;
  end process rd_port_mux;




p_AW_MUX:	PROCESS (clk_sys, Powerup_Res, Powerup_Done, s_AW_ID, s_nLED_Out, PIO, A_SEL, fg_1_sw, fg_1_strobe, dff1_q,
									 AWIn1, AWIn2, AWIn3, AWIn4, AWIn5, AWIn6, 
									 AWOut_Reg1, AWOut_Reg2, AWOut_Reg3, AWOut_Reg4, AWOut_Reg5, AWOut_Reg6,
									 IO_Test_Port0, IO_Test_Port1, IO_Test_Port2, IO_Test_Port3,
									 CLK_IO, clk_blink, s_nLED_Sel, s_nLED_Dtack, s_nLED_INR, 
									 Ena_Every_1us, ena_Every_20ms, ena_Every_250ms,
									 s_deb_In1, s_deb_In2, s_deb_In3, s_deb_In4,
									 s_deb_Out1, s_deb_Out2, s_deb_Out3, s_deb_Out4,
									 s_str_In1, s_str_In2, s_str_In3, s_str_In4,
									 s_str_Out1, s_str_Out2, s_str_Out3, s_str_Out4, 
									 s_Led_Bu1_Out, s_led_Bu2_Out, s_led_Bu3_Out, s_led_Bu4_Out		 
									 )
	
BEGIN
	

		--#################################################################################
		--#################################################################################
		--###																																						###
		--###											IO-Stecker-Test mit "Brckenkarte											###
		--###																																						###
		--#################################################################################
	
		--############################# Set Defaults ######################################

		PIO(150 DOWNTO 16)	<= (OTHERS => 'Z');	-- setze alle IO-Pins auf Input;

		s_Led_Bu1	<= '0';	s_Led_Bu2	<= '0';	s_Led_Bu3	<= '0'; s_Led_Bu4	<= '0';
		s_deb_In1	<= '0';	s_deb_In2	<= '0';	s_deb_In3	<= '0'; s_deb_In4	<= '0';
		s_str_In1	<= '0';	s_str_In2	<= '0';	s_str_In3	<= '0'; s_str_In4	<= '0';
		dff1_Reset		<= '0'; dff1_clk_En		<=	'0'; dff1_d		<=	'0';
--		IO_Test_Port0				<=	x"0000";	-- IO-Test-Port 0
--		IO_Test_Port1				<=	x"0000";	-- IO-Test-Port 1
--		IO_Test_Port2				<=	x"0000";	-- IO-Test-Port 2
--		IO_Test_Port3				<=	x"0000";	-- IO-Test-Port 3
		AWIn1(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 1
		AWIn2(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 2
		AWIn3(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 3
		AWIn4(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 4
		AWIn5(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 5
		AWIn6(15 DOWNTO 0)	<=	x"0000";	-- AW-In-Register 6
		s_AW_ID(7 DOWNTO 0)	<=	x"FF";		-- Anwender-Karten ID
		UIO(15 DOWNTO 0)		<=	(OTHERS => '0');	-- USER-IO-Pins zu VG-Leiste
		
		extension_cid_system <=	(OTHERS => '0');	-- extension card: cid_system
		extension_cid_group  <=	(OTHERS => '0');	-- extension card: cid_group

		
		
    IF 	AW_Config(15) = '1'  THEN		-- Config-Reg Bit15 = 1  --> Testmode 
--  IF 	A_SEL = not (X"F")   THEN		-- Codierschalter = x"F" --> Testmode 


			UIO(15 DOWNTO 0)		<=	AWOut_Reg6(15 DOWNTO 0);	-- Test USER-IO-Pins zu VG-Leiste ber die "USER-Register" 
	

			AWIn1(15 DOWNTO 0)	<=	(		CLK_IO,	PIO(16),	PIO(17),	PIO(18),	PIO(19),	PIO(20),	PIO(21),	PIO(22),
																PIO(23),	PIO(24),	PIO(25),	PIO(26),	PIO(27),	PIO(28),	PIO(29),	PIO(30)		);

					(	PIO(61),	PIO(62),	PIO(59),	PIO(60),	PIO(57),	PIO(58),	PIO(55),	PIO(56),											
						PIO(53),	PIO(54),	PIO(51),	PIO(52),	PIO(49),	PIO(50),	PIO(47),	PIO(48)		)		<=	AWOut_Reg1(15 DOWNTO 0) ;					
	

			AWIn2(15 DOWNTO 0)	<=	(	PIO(31),	PIO(32),	PIO(33),	PIO(34),	PIO(35),	PIO(36),	PIO(37),	PIO(38),
																PIO(39),	PIO(40),	PIO(41),	PIO(42),	PIO(43),	PIO(44),	PIO(45),	PIO(46)		);

					(	PIO(77),	PIO(78),	PIO(75),	PIO(76),	PIO(73),	PIO(74),	PIO(71),	PIO(72),
						PIO(69),	PIO(70),	PIO(67),	PIO(68),	PIO(65),	PIO(66),	PIO(63),	PIO(64)		)		<=	AWOut_Reg2(15 DOWNTO 0) ;


			AWIn3(15 DOWNTO 0)	<=	(	PIO(79),	PIO(80),	PIO(81),	PIO(82),	PIO(83),	PIO(84),	PIO(85),	PIO(86),
																PIO(87),	PIO(88),	PIO(89),	PIO(90),	PIO(91),	PIO(92),	PIO(93),	PIO(94)		);
							
					(	PIO(125),	PIO(126),	PIO(123),	PIO(124),	PIO(121),	PIO(122),	PIO(119),	PIO(120),
						PIO(117),	PIO(118),	PIO(115),	PIO(116),	PIO(113),	PIO(114),	PIO(111),	PIO(112)	)		<=	AWOut_Reg3(15 DOWNTO 0) ;

						
			AWIn4(15 DOWNTO 0)	<=	(	PIO(95),	PIO(96),	PIO(97),	PIO(98),	PIO(99),	PIO(100),	PIO(101),	PIO(102),
																PIO(103),	PIO(104),	PIO(105),	PIO(106),	PIO(107),	PIO(108),	PIO(109),	PIO(110)	);
													
					(	PIO(141),	PIO(142),	PIO(139),	PIO(140),	PIO(137),	PIO(138),	PIO(135),	PIO(136),								
						PIO(133),	PIO(134),	PIO(131),	PIO(132),	PIO(129),	PIO(130),	PIO(127),	PIO(128)	)		<=	AWOut_Reg4(15 DOWNTO 0) ;


			AWIn5(15 DOWNTO 0)	<=	(	AWOut_Reg5(15), AWOut_Reg5(14), AWOut_Reg5(13), AWOut_Reg5(12), --+ 	Input [15..0] ist eine
																AWOut_Reg5(11), AWOut_Reg5(10),	AWOut_Reg5(9), 	AWOut_Reg5(8), 	--+-> Copy der Output-Bits, weil das	
																AWOut_Reg5(7),	AWOut_Reg5(6),	AWOut_Reg5(5),	AWOut_Reg5(4),	--+		Testprog. nur 16 Bit Vergleiche kann.
																PIO(143),				PIO(144),				PIO(149),				PIO(150) );

						(	PIO(147),	PIO(148),	PIO(145),	PIO(146) )		<=	AWOut_Reg5(3 DOWNTO 0) ;
			

	else
	
		--#################################################################################
		--#################################################################################
		--###																																						###
		--###													Stecker Anwender I/O															###
		--###																																						###
		--#################################################################################
		--#################################################################################
	
	

		--Input: Anwender_ID ---      
			s_AW_ID(7 DOWNTO 0)		<=	(PIO(150),PIO(149),PIO(148),PIO(147),PIO(146),PIO(145),PIO(144),PIO(143));
      AWIn5(15 DOWNTO 0)		<=	x"00" &	s_AW_ID(7 DOWNTO 0);-- Anwender_ID
		
	
		--	--- Output: Anwender-LED's ---

			PIO(17) <= s_nLED_Sel;				-- LED7 = sel Board 
			PIO(19) <= s_nLED_Dtack;			-- LED6 = Dtack 
			PIO(21) <= s_nLED_INR;				-- LED5 = Interrupt
			PIO(23) <= (Powerup_Done AND not (clk_blink And Powerup_Done ));	-- -- LED4 = Powerup	
			PIO(25) <= '1';
			PIO(27) <= '1';	 
			PIO(29) <= '1';	 
			PIO(31) <= ((clkbad0 or clkbad1) AND not (clk_blink And (clkbad0 or clkbad1) ));	-- -- LED4 = Int. Clock	
		
		
	CASE s_AW_ID(7 DOWNTO 0) IS
	

	WHEN	c_AW_P37IO =>

		--#################################################################################
		--####									Anwender-IO: P37IO	-- FG900_700												###
		--#################################################################################

		extension_cid_system <=	x"0037";	-- extension card: cid_system, CSCOHW=55
		extension_cid_group  <=	x"001B";	-- extension card: cid_group, "FG900700_P37IO1" = 27
		
		
		-- Input Start --
			s_deb_In1	<=	not PIO(139);					-- Debounce, Input "Start" H-Aktiv
			s_str_In1	<=	s_deb_Out1;						-- pos. Flanke des entprellten Start-Pulses wird ein Puls (1-clk)
							
			s_Led_Bu1	<=	s_deb_Out1;						-- Stretch Deb_Inputsignal fr LED
			PIO(33)		<=	s_Led_Bu1_Out;				--	Output "nLED_Start"
					
					
		-- Input Stop --
			s_deb_In2	<=	not PIO(141);					-- Debounce, Input "Stop" H-aktiv
			s_Led_Bu2	<=	s_deb_Out2;						-- Stretch Deb_Inputsignal fr LED
			PIO(35)		<=	s_Led_Bu2_Out;				-- Output "nLED_Stop"
		

		-- Input Reset --
			s_deb_In3		<=		not PIO(133);			-- Debounce, Input "Resknapp" (L-akiv)

					
			-- Output BNC --
			dff1_Reset  	<= (s_deb_Out3 or s_deb_Out2);	-- clear FF, mit "ext.-Stop" oder "ext.-Reset"
			dff1_clk_En 	<= 	s_str_Out1;									-- Start-Puls (1-clk)							
			dff1_d      	<=		'1';											-- auf VCC
			PIO(51)				<=	dff1_q;											-- Output "CO_BNC"
			s_Led_Bu3			<=	dff1_q;											-- Stretch, Output-Signal, Buchse BNC
			PIO(37)				<=	s_Led_Bu3_Out;							-- Output "nLED_BNC"
		
			
			IO_Test_Port0	<=	s_nLED_Sel        &	s_nLED_Dtack			& s_nLED_INR      & '0'			    	&		-- bit 15..12
												clk_blink    			&	ENA_every_250ms		&	Ena_every_20ms	&	Ena_every_1us	&		-- bit 11..8					
												dff1_q 						&	s_str_Out1 				& s_deb_Out3			&	s_deb_In3			&		-- bit 7..4					
												s_deb_Out2				&	s_deb_In2 				&	s_deb_Out1 			&	s_deb_In1; 				-- bit 3..0
			
			



			PIO(39)	<=	'0';	-------------------------------	Output_Enable (nach Init vom ALTERA)
			PIO(41)	<=	'0';	-------------------------------	Output_Enable (nach Init vom ALTERA)
			PIO(43)	<=	'0';	-------------------------------	Output_Enable (nach Init vom ALTERA)

			PIO(65)	<=	not AWOut_Reg1(7);	--	Output "CO_D7"
			PIO(69)	<=	not AWOut_Reg1(6);	--	Output "CO_D6"
			PIO(61)	<=	not AWOut_Reg1(5);	--	Output "CO_D5"
			PIO(67)	<=	not AWOut_Reg1(4);	--	Output "CO_D4"
			PIO(63)	<=	not AWOut_Reg1(3);	--	Output "CO_D3"
			PIO(71)	<=	not AWOut_Reg1(2);	--	Output "CO_D2"
			PIO(55)	<=	not AWOut_Reg1(1);	--	Output "CO_D1"
			PIO(53)	<=	not AWOut_Reg1(0);	--	Output "CO_D0"
			PIO(57)	<=	not AWOut_Reg2(1);	--	Output "CO_FAULT"
			PIO(59)	<=	not AWOut_Reg2(0);	--	Output "CO_STAT"
                
 
			AWIn1(15)	<=	PIO(131);	--	Input "HI7"
			AWIn1(14)	<=	PIO(129);	--	Input "HI6"
			AWIn1(13)	<=	PIO(127);	--	Input "HI5"
			AWIn1(12)	<=	PIO(125);	--	Input "HI4"
			AWIn1(11)	<=	PIO(123);	--	Input "HI3"
			AWIn1(10)	<=	PIO(121);	--	Input "HI2"
			AWIn1(9)	<=	PIO(119);	--	Input "HI1"
			AWIn1(8)	<=	PIO(117);	--	Input "HI0"
		
			AWIn1(7)	<=	PIO(115);	--	Input "LO7"
			AWIn1(6)	<=	PIO(113);	--	Input "LO6"
			AWIn1(5)	<=	PIO(111);	--	Input "LO5"
			AWIn1(4)	<=	PIO(109);	--	Input "LO4"
			AWIn1(3)	<=	PIO(107);	--	Input "LO3"
			AWIn1(2)	<=	PIO(105);	--	Input "LO2"
			AWIn1(1)	<=	PIO(103);	--	Input "LO1"
			AWIn1(0)	<=	PIO(101);	--	Input "LO0"
	


	WHEN   c_AW_P25IO =>
	
		--#################################################################################
		--####										Anwender-IO: P25IO	-- FG900_710											###
		--#################################################################################

		extension_cid_system <=	x"0037";	-- extension card: cid_system, CSCOHW=55
		extension_cid_group  <=	x"001C";	-- extension card: cid_group, "FG900710_P25IO1" = 28



		IF 	(AW_Config(4) = '1')  THEN
	
--           FG_mode; DAC-Werte kommen von FG_Data und werden mit FG_Strobe uebernommen. Kein externer Trigger!	

             PIO(105)               <=	not fg_1_strobe;	-- vom Funktionsgen.
             DAC_Out(15 DOWNTO 0)		<=	fg_1_sw(31 DOWNTO 16);	
				Else
--           Software-Mode, DAC-Werte, kommen vom SCU-Bus-Slave. Externe Triggerung mit pos. oder neg. Flanke, kann eingeschaltet werden. 

             PIO(105)               <=	AWOut_Reg1_wr; --	
				     DAC_Out(15 DOWNTO 0)		<=	AWOut_Reg1(15 DOWNTO 0);
			END IF;	
			
			PIO(107)	<=	DAC_Out(15);	-- Output Bit-15
			PIO(109)	<=	DAC_Out(14);	-- Output Bit-14
			PIO(111)	<=	DAC_Out(13);	-- Output Bit-13
			PIO(113)	<=	DAC_Out(12);	-- Output Bit-12
			PIO(115)	<=	DAC_Out(11);	-- Output Bit-11
			PIO(117)	<=	DAC_Out(10);	-- Output Bit-10
			PIO(119)	<=	DAC_Out(9);		-- Output Bit-9
			PIO(121)	<=	DAC_Out(8);		-- Output Bit-8
			PIO(123)	<=	DAC_Out(7);		-- Output Bit-7
			PIO(125)	<=	DAC_Out(6);		-- Output Bit-6
			PIO(127)	<=	DAC_Out(5);		-- Output Bit-5
			PIO(129)	<=	DAC_Out(4);		-- Output Bit-4
			PIO(131)	<=	DAC_Out(3);		-- Output Bit-3
			PIO(133)	<=	DAC_Out(2);		-- Output Bit-2
			PIO(135)	<=	DAC_Out(1);		-- Output Bit-1
			PIO(137)	<=	DAC_Out(0);		-- Output Bit-0

			
			

			

			AWIn1(15)	<=	PIO(41);	--------------	Input "HI7"
			AWIn1(14)	<=	PIO(43);	--------------	Input "HI6"
			AWIn1(13)	<=	PIO(37);	--------------	Input "HI5"
			AWIn1(12)	<=	PIO(39);	--------------	Input "HI4"
			AWIn1(11)	<=	PIO(33);	--------------	Input "HI3"
			AWIn1(10)	<=	PIO(35);	--------------	Input "HI2"
			AWIn1(9)	<=	PIO(49);	--------------	Input "HI1"
			AWIn1(8)	<=	PIO(51);	--------------	Input "HI0"
			AWIn1(7)	<=	PIO(53);	--	Input "Lo7"
			AWIn1(6)	<=	PIO(55);	--	Input "LO6"
			AWIn1(5)	<=	PIO(45);	--	Input "LO5"
			AWIn1(4)	<=	PIO(47);	--	Input "LO4"
			AWIn1(3)	<=	PIO(57);	--	Input "LO3"
			AWIn1(2)	<=	PIO(59);	--	Input "LO2"
			AWIn1(1)	<=	PIO(61);	--	Input "LO1"
			AWIn1(0)	<=	PIO(63);	--	Input "LO0"

	
	
	WHEN   c_AW_OCIN =>

		--#################################################################################
		--####										Anwender-IO: OCIN	-- FG900_720												###
		--#################################################################################

		extension_cid_system <=	x"0037";	-- extension card: cid_system, CSCOHW=55
		extension_cid_group  <=	x"001D";	-- extension card: cid_group, "FG900720_OCIN1" = 29


			AWIn1(15)	<=	'0';			--------------	Frei
			AWIn1(14)	<=	'0';			--------------	Frei
			AWIn1(13)	<=	PIO(73);	--------------	Input "B5"
			AWIn1(12)	<=	PIO(75);	--------------	Input "B4"
			AWIn1(11)	<=	PIO(93);	--------------	Input "B3"
			AWIn1(10)	<=	PIO(95);	--------------	Input "B2"
			AWIn1(9)	<=	PIO(97);	--------------	Input "B1"
			AWIn1(8)	<=	PIO(99);	--------------	Input "B0"
			AWIn1(7)	<=	PIO(117);	--	Input "A7"
			AWIn1(6)	<=	PIO(119);	--	Input "A6"
			AWIn1(5)	<=	PIO(121);	--	Input "A5"
			AWIn1(4)	<=	PIO(123);	--	Input "A4"
			AWIn1(3)	<=	PIO(133);	--	Input "A3"
			AWIn1(2)	<=	PIO(135);	--	Input "A2"
			AWIn1(1)	<=	PIO(137);	--	Input "A1"
			AWIn1(0)	<=	PIO(139);	--	Input "A0"

	
			AWIn2(15)	<=	PIO(81);	--------------	Input "D7"
			AWIn2(14)	<=	PIO(79);	--------------	Input "D6"
			AWIn2(13)	<=	PIO(77);	--------------	Input "D5"
			AWIn2(12)	<=	PIO(83);	--------------	Input "D4"
			AWIn2(11)	<=	PIO(85);	--------------	Input "D3"
			AWIn2(10)	<=	PIO(87);	--------------	Input "D2"
			AWIn2(9)	<=	PIO(89);	--------------	Input "D1"
			AWIn2(8)	<=	PIO(91);	--------------	Input "D0"
			AWIn2(7)	<=	PIO(109);	--	Input "C7"
			AWIn2(6)	<=	PIO(111);	--	Input "C6"
			AWIn2(5)	<=	PIO(113);	--	Input "C5"
			AWIn2(4)	<=	PIO(115);	--	Input "C4"
			AWIn2(3)	<=	PIO(125);	--	Input "C3"
			AWIn2(2)	<=	PIO(127);	--	Input "C2"
			AWIn2(1)	<=	PIO(129);	--	Input "C1"
			AWIn2(0)	<=	PIO(131);	--	Input "C0"
		
		
			PIO(39)		<=	'0';	-------------------------------	Output_Enable (nach Init vom ALTERA)

			PIO(49)	<=	not AWOut_Reg1(3);	--	Output "2CB2"
			PIO(47)	<=	not AWOut_Reg1(2);	--	Output "2CA2"
			PIO(45)	<=	not AWOut_Reg1(1);	--	Output "1CB2"
			PIO(43)	<=	not AWOut_Reg1(0);	--	Output "1CA2"


		
	WHEN   c_AW_OCIO =>
	
		--#################################################################################
		--####											Anwender-IO: OCIO	-- FG900_730											###
		--#################################################################################

		extension_cid_system <=	x"0037";	-- extension card: cid_system, CSCOHW=55
		extension_cid_group  <=	x"001E";	-- extension card: cid_group, "FG900730_OCIO1" = 30

		
		

			AWIn1(15)	<=	PIO(45);	--------------	Input "C7"
			AWIn1(14)	<=	PIO(47);	--------------	Input "C6"
			AWIn1(13)	<=	PIO(51);	--------------	Input "C5"
			AWIn1(12)	<=	PIO(53);	--------------	Input "C4"
			AWIn1(11)	<=	PIO(37);	--------------	Input "C3"
			AWIn1(10)	<=	PIO(39);	--------------	Input "C2"
			AWIn1(9)	<=	PIO(41);	--------------	Input "C1"
			AWIn1(8)	<=	PIO(43);	--------------	Input "C0"
			AWIn1(7)	<=	PIO(127);	--	Input "A7"
			AWIn1(6)	<=	PIO(125);	--	Input "A6"
			AWIn1(5)	<=	PIO(123);	--	Input "A5"
			AWIn1(4)	<=	PIO(121);	--	Input "A4"
			AWIn1(3)	<=	PIO(99);	--	Input "A3"
			AWIn1(2)	<=	PIO(97);	--	Input "A2"
			AWIn1(1)	<=	PIO(95);	--	Input "A1"
			AWIn1(0)	<=	PIO(93);	--	Input "A0"



			AWIn2(15 DOWNTO 8)	<=	(OTHERS => '0');	-- INPUT = 0;	
			AWIn2(7)	<=	PIO(89);	--	Input "D7"
			AWIn2(6)	<=	PIO(91);	--	Input "D6"
			AWIn2(5)	<=	PIO(119);	--	Input "D5"
			AWIn2(4)	<=	PIO(117);	--	Input "D4"
			AWIn2(3)	<=	PIO(87);	--	Input "D3"
			AWIn2(2)	<=	PIO(85);	--	Input "D2"
			AWIn2(1)	<=	PIO(83);	--	Input "D1"
			AWIn2(0)	<=	PIO(81);	--	Input "D0"
			
		

			PIO(77)		<=	'0';	-------------------------------	Output_Enable (nach Init vom ALTERA)
			
			PIO(105)	<=	not AWOut_Reg1(11);	----------------	Output "CD2"
			PIO(61) 	<=	not AWOut_Reg1(10);	----------------	Output "CC2"
			PIO(107)	<=	not AWOut_Reg1(9);	----------------	Output "CB2"
			PIO(115)	<=	not AWOut_Reg1(8);	----------------	Output "CA2"
			PIO(109)	<=	not AWOut_Reg1(7);	--	Output "B7"
			PIO(111)	<=	not AWOut_Reg1(6);	--	Output "B6"
			PIO(113)	<=	not AWOut_Reg1(5);	--	Output "B5"
			PIO(101)	<=	not AWOut_Reg1(4);	--	Output "B4"
			PIO(103)	<=	not AWOut_Reg1(3);	--	Output "B3"
			PIO(59)		<=	not AWOut_Reg1(2);	--	Output "B2"
			PIO(57)		<=	not AWOut_Reg1(1);	--	Output "B1"
			PIO(55)		<=	not AWOut_Reg1(0);	--	Output "B0"


			
	WHEN   c_AW_UIO =>
	
		--###################################################################################
		--####											Anwender-IO: UIO	-- FG900_740												###
		--###################################################################################



	WHEN   c_AW_DA =>
			
		--###################################################################################
		--####									Anwender-IO: DA(DAC/ADC)	-- FG900_75											###
		--###################################################################################
	

			
	WHEN OTHERS =>

		--	--- Output: Anwender-LED's ---

			PIO(17)		<=		NOT (s_AW_ID(7)	AND	clk_blink);	-- LED7
			PIO(19)		<=		NOT (s_AW_ID(6)	AND	clk_blink);	-- LED7		
			PIO(21)		<=		NOT (s_AW_ID(5)	AND	clk_blink);	-- LED7 		
			PIO(23)		<=		NOT (s_AW_ID(4)	AND	clk_blink);	-- LED7	  
			PIO(25)		<=		NOT (s_AW_ID(3)	AND	clk_blink);	-- LED7	 	
			PIO(27)		<=		NOT (s_AW_ID(2)	AND	clk_blink);	-- LED7	 		 
			PIO(29)		<=		NOT (s_AW_ID(1)	AND	clk_blink);	-- LED7	 		 
			PIO(31)		<=		NOT (s_AW_ID(0)	AND	clk_blink);	-- LED7	 	
           
	END CASE;


	END IF;
--
--	
END PROCESS p_AW_MUX;


	
	
	
	
	
	
	
	
	
	
	





A_TA(1) <= '1'; -- drives the external max level shifter
  

end architecture;
