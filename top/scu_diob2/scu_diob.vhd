library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.fg_quad_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;
use work.blackbox_config_pkg.all;


entity scu_diob is
generic (
  CLK_sys_in_Hz : integer := 125000000;
  g_card_type   : string  := "diob"
);

port (
  ------------------------------ Clocks -------------------------------------------------------------------------
  CLK_20MHz_A: in std_logic; -- Clock_A
  CLK_20MHz_B: in std_logic; -- Clock_B
  CLK_20MHz_C: in std_logic; -- Clock_C
  CLK_20MHz_D: in std_logic; -- Clock_D

  --------- parallel scu bus signals ----------------------------------------------------------------------------
  A_A                   : in std_logic_vector(15 downto 0);     -- address bus signals
  A_nADR_EN             : out std_logic := '0';                 -- '0' => external address driver active 
  A_nADR_FROM_SCUB      : out std_logic := '0';                 -- '0' => external address driver direction master => slave
  A_D                   : inout std_logic_vector(15 downto 0);  -- databus signals
  A_nDS                 : in std_logic;                         -- data strobe driven by master
  A_RnW                 : in std_logic;                         -- read/write, driven by master
  A_nSel_Ext_Data_Drv   : out std_logic;                        -- '0' => external data driver is active
  A_Ext_Data_RD         : out std_logic;                        -- '0' => external data driver direction master => slave
                                                                -- (besser default 0, oder Treiber A/B tauschen) -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
  A_nDtack              : out std_logic;                        -- '0' => data acknowlege is active; opendrain driver
  A_nSRQ                : out std_logic;                        -- '0' => service request is active; opendrain driver
  A_nBoardSel           : in std_logic;                         -- '0' => selects this slave 
  A_nEvent_Str          : in std_logic;                         -- '0' => signals a timing cycle
  A_SysClock            : in std_logic;                         -- sys clock driver by the bus master
  A_Spare0              : in std_logic;                         -- spare line; direction master => slave
  A_Spare1              : in std_logic;                         -- spare line; direction master => slave
  A_nReset              : in std_logic;                         -- '0' => reset is active; driven by master

  A_nSEL_Ext_Signal_DRV : out std_logic;                        -- '0' => external signal driver is active
  A_nExt_Signal_in      : out std_logic;                        -- '0' => external signal direction master => slave

  ----------------- OneWire ----------------------------------------------------------------------------------------
  A_OneWire: inout std_logic;                                   -- temperature sensor on the slave board

  ------------ Logic analyser Signals -------------------------------------------------------------------------------
  A_SEL  : in std_logic_vector(3 downto 0);                     -- use to select sources for the logic analyser ports
  A_Tclk : out std_logic;                                       -- Clock  for Logikanalysator Port A
  A_TA   : out std_logic_vector(15 downto 0);                   -- test port a

  ---------------------------------- Diagnose-LED's -----------------------------------------------------------------
  A_nLED_D2 : out std_logic;                                    -- debug led d2 on the base board
  A_nLED_D3 : out std_logic;                                    -- debug led d3 on the base board

  ------------ User I/O zur VG-Connector -------------------------------------------------------------------------------
  A_nUser_EN : out std_logic;                                   -- Enable User-I/O
  UIO        : inout std_logic_vector(15 downto 0);             -- User I/O VG-Connector

  ---------------- User I/O to the user module -----------------------------------------------------------------
  CLK_IO : in std_logic;                                        -- clock from the user module
  PIO    : inout std_logic_vector(150 downto 16)                -- Dig. User I/0 to Piggy
  );
end scu_diob;



architecture scu_diob_arch of scu_diob is


--  +============================================================================================================================+
--  |                                 Firmware_Version/Firmware_Release und Basis-Adressen                                       |
--  +============================================================================================================================+

    CONSTANT c_Firmware_Version:    Integer := 16#0100#;      -- Firmware_Version
    CONSTANT c_Firmware_Release:    Integer := 0;     -- Firmware_release Stand 19.05.2021 ( + neuer Zwischen-Backplane )

    CONSTANT clk_switch_status_cntrl_addr:       unsigned := x"0030";
    CONSTANT c_lm32_ow_Base_Addr:   unsigned(15 downto 0):=  x"0040";  -- housekeeping/LM32

--  +============================================================================================================================+
--  |                                                 CONSTANT                                                                   |
--  +============================================================================================================================+


    constant  Clk_in_ns:      integer  :=  1000000000 /  clk_sys_in_Hz;          -- (=8ns,    bei 125MHz)
    CONSTANT  CLK_sys_in_ps:  INTEGER  := (1000000000 / (CLK_sys_in_Hz / 1000));  -- muss eigentlich clk-halbe sein


--  +============================================================================================================================+
--  |                                                       component                                                            |
--  +============================================================================================================================+
component io_blackbox 
generic(
		nr_diob_ios:           integer; -- range 0 to 256:=256;
		nr_virt_ios:           integer; -- range 0 to 96;
		nr_backplane_ios:       integer; -- range 0 to 16;
		max_frontend_plugins:  integer; --
		max_proc_plugins: 		 integer;
		max_user_plugins: 	   integer;
    frontend_status_bits:  integer;
		frontend_sel_bits:     integer;
		proc_sel_bits:         integer;
		user_sel_bits:         integer;
		addr_bus_width:        integer;
		data_bus_width:        integer
	);
  port(
		-- Common
		clock:                  in    std_logic;            								
		reset:                  in    std_logic;             								  
		-- Frontend
		diob_io:                inout	std_logic_vector(nr_diob_ios-1 downto 0); 		    -- Connection to DIOB I/O
		frontend_plugin_select: in	  std_logic_vector(frontend_sel_bits-1 downto 0); --I/O plugin selection
		backplane_io:           inout	std_logic_vector(nr_backplane_ios-1 downto 0); --Backplane input/output fed (almost) directly	to user plugin
		-- SCU-bus
    addr:                   in std_logic_vector(addr_bus_width-1 downto 0);	    --(Adr_from_SCUB_LA)
		data_w:                 in std_logic_vector(data_bus_width-1 downto 0);   -- (Data_from_SCUB_LA)
		data_r:                 out	std_logic_vector(data_bus_width-1 downto 0);	--(Data_to_SCUB)

		addr_strobe:            in std_logic;	-- (Ext_Adr_Val)
	  read_trg:               in std_logic; -- (Ext_Rd_active)
		write_trg:              in std_logic; -- (Ext_Wr_active)
		read_fin:               in std_logic; -- (Ext_Rd_fin)
		write_fin:              in std_logic;	-- (Ext_Wr_fin)
		event_trg:              in std_logic;									
    
    dtack:                  out std_logic;--(Dtack_to_SCUB)
		data_r_act:             out std_logic --(Reg_rd_active)	
    );
  end component io_blackbox;


--  +============================================================================================================================+
--  |                                                         signal                                                             |
--  +============================================================================================================================+

  signal clk_sys, clk_cal, locked : std_logic;
  signal Debounce_cnt:              integer range 0 to 16383;   -- Clock's für die Entprellzeit

  signal sys_clk_is_bad        : std_logic;
  signal rstn_sys              : std_logic;
  signal pll_locked            : std_logic;
  signal clk_switch_rd_data    : std_logic_vector(15 downto 0);
  signal clk_switch_rd_active  : std_logic;
  signal clk_switch_dtack      : std_logic;
  signal signal_tap_clk_250mhz : std_logic;
  signal clk_update            : std_logic;
  signal clk_flash             : std_logic;
  signal rstn_stc              : std_logic;
  signal rstn_update           : std_logic;
  signal rstn_flash            : std_logic;
  constant c_is_arria5: boolean := false;
  signal local_clk_is_bad      : std_logic;


  signal SCUB_SRQ:            std_logic;
  signal SCUB_Dtack:          std_logic;

  signal Dtack_to_SCUB:       std_logic;

  signal ADR_from_SCUB_LA:    std_logic_vector(15 downto 0);
  signal Data_from_SCUB_LA:   std_logic_vector(15 downto 0);
  signal Ext_Adr_Val:         std_logic;
  signal Ext_Rd_active:       std_logic;
  signal Ext_Wr_active:       std_logic;
  signal Ext_Wr_fin_ovl:      std_logic;
  signal Ext_RD_fin_ovl:      std_logic;
  signal SCU_Ext_Wr_fin:      std_logic;
  signal nPowerup_Res:        std_logic;
  signal Timing_Pattern_LA:   std_logic_vector(31 downto 0);--  latched timing pattern from SCU_Bus for external user functions
  signal Timing_Pattern_RCV:  std_logic;----------------------  timing pattern received

  signal Data_to_SCUB:       std_logic_vector(15 downto 0);

  signal owr_pwren_o:        std_logic_vector(1 downto 0);
  signal owr_en_o:           std_logic_vector(1 downto 0);
  signal owr_i:              std_logic_vector(1 downto 0);

  signal wb_scu_rd_active:    std_logic;
  signal wb_scu_dtack:        std_logic;
  signal wb_scu_data_to_SCUB: std_logic_vector(15 downto 0);

  signal Powerup_Done:    std_logic;  -- this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.

  signal Deb_SCUB_Reset_out:  std_logic;
  signal Standard_Reg_Acc:    std_logic;
  signal Ext_Rd_fin:          std_logic;
  signal Ext_Wr_fin:          std_logic;

  signal uart_txd_out:    std_logic;

  signal b_box_rd_active: std_logic;
  signal b_box_rd_data: std_logic_vector(15 downto 0);
  signal bb_dtack: std_logic;
  signal b_backplane:std_logic_vector(15 downto 0);
 

begin

  A_nADR_EN             <= '0';
  A_nADR_FROM_SCUB      <= '0';
  A_nExt_Signal_in      <= '0';
  A_nSEL_Ext_Signal_DRV <= '0';
  A_nUser_EN            <= '0';

  diob_clk_switch: slave_clk_switch
    generic map (
      Base_Addr => clk_switch_status_cntrl_addr,
      card_type => g_card_type
    )
    port map(
      local_clk_i             => CLK_20MHz_D,
      sys_clk_i               => A_SysClock,
      nReset                  => rstn_sys,
      master_clk_o            => clk_sys,               -- core clocking
      pll_locked              => pll_locked,
      sys_clk_is_bad          => sys_clk_is_bad,
      Adr_from_SCUB_LA        => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
      Data_from_SCUB_LA       => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
      Ext_Adr_Val             => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active           => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
      Ext_Wr_active           => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
      Rd_Port                 => clk_switch_rd_data,    -- output for all read sources of this macro
      Rd_Activ                => clk_switch_rd_active,  -- this acro has read data available at the Rd_Port.
      Dtack                   => clk_switch_dtack,
      signal_tap_clk_250mhz   => signal_tap_clk_250mhz,
      clk_update              => clk_update,
      clk_flash               => clk_flash,
      clk_encdec              => open
    );

   reset : altera_reset
    generic map (
      g_plls   => 1,
      g_clocks => 4,
      g_areset => f_pick(c_is_arria5, 100, 1)*1024,
      g_stable => f_pick(c_is_arria5, 100, 1)*1024)
    port map (
      clk_free_i    => clk_sys,
      rstn_i        => A_nReset,
      pll_lock_i(0) => pll_locked,
      pll_arst_o    => open,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => signal_tap_clk_250mhz,
      clocks_i(2)   => clk_update,
      clocks_i(3)   => clk_flash,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_stc,
      rstn_o(2)     => rstn_update,
      rstn_o(3)     => rstn_flash
    );

  -- open drain buffer for one wire
  owr_i(0) <= A_OneWire;
  A_OneWire <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';

  SCU_Slave: SCU_Bus_Slave
  generic map (
      CLK_in_Hz               => clk_sys_in_Hz,
      Firmware_Release        => c_Firmware_Release,  -------------------- important: => Firmware_Release
      Firmware_Version        => c_Firmware_Version,  -------------------- important: => Firmware_Version
      CID_System              => 55, ------------------------------------- important: => CSCOHW
      intr_Enable             => b"0000_0000_0000_0001")
  port map (
      SCUB_Addr               => A_A,                                   -- in, SCU_Bus: address bus
      nSCUB_Timing_Cyc        => A_nEvent_Str,                          -- in, SCU_Bus signal: low active SCU_Bus runs timing cycle
      SCUB_Data               => A_D,                                   -- inout, SCU_Bus: data bus (FPGA tri state buffer)
      nSCUB_Slave_Sel         => A_nBoardSel,                           -- in, SCU_Bus: '0' => SCU master select slave
      nSCUB_DS                => A_nDS,                                 -- in, SCU_Bus: '0' => SCU master activate data strobe
      SCUB_RDnWR              => A_RnW,                                 -- in, SCU_Bus: '1' => SCU master read slave
      clk                     => clk_sys,
      nSCUB_Reset_in          => A_nReset,                              -- in, SCU_Bus-Signal: '0' => 'nSCUB_Reset_in' is active
      Data_to_SCUB            => Data_to_SCUB,                          -- in, connect read sources from external user functions
      Dtack_to_SCUB           => Dtack_to_SCUB,                         -- in, connect Dtack from from external user functions
      intr_in                 => '0' & '0' & '0' & '0'                  -- bit 15..12
                               & '0' & '0' & '0' &'0'                   -- bit 11..8
                               & '0' & '0' & '0' & '0'                  -- bit 7..4
                               & '0' & '0' & '0',                       -- bit 3..1
      User_Ready              => '1',
      CID_GROUP               => 26,                                    -- important: => "FG900500_SCU_Diob1"
      extension_cid_system    => 0,                                    -- in, extension card: cid_system
      extension_cid_group     => 0,                                    -- in, extension card: cid_group
      Data_from_SCUB_LA       => Data_from_SCUB_LA,                     -- out, latched data from SCU_Bus for external user functions
      ADR_from_SCUB_LA        => ADR_from_SCUB_LA,                      -- out, latched address from SCU_Bus for external user functions
      Timing_Pattern_LA       => Timing_Pattern_LA,                     -- out, latched timing pattern from SCU_Bus for external user functions
      Timing_Pattern_RCV      => Timing_Pattern_RCV,                    -- out, timing pattern received
      nSCUB_Dtack_Opdrn       => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                        -- '0' => slave give dtack to SCU master
      SCUB_Dtack              => SCUB_Dtack,                            -- out, for connect via ext. open collector driver
                                                                        -- '1' => slave give dtack to SCU master
      nSCUB_SRQ_Opdrn         => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                        -- '0' => slave service request to SCU ma
      SCUB_SRQ                => SCUB_SRQ,                              -- out, for connect via ext. open collector driver
                                                                        -- '1' => slave service request to SCU master
      nSel_Ext_Data_Drv       => A_nSel_Ext_Data_Drv,                   -- out, '0' => select the external data driver on the SCU_Bus slave
      Ext_Data_Drv_Rd         => A_Ext_Data_RD,                         -- out, '1' => direction of the external data driver on the
                                                                        -- SCU_Bus slave is to the SCU_Bus
      Standard_Reg_Acc        => Standard_Reg_Acc,                      -- out, '1' => mark the access to register of this macro
      Ext_Adr_Val             => Ext_Adr_Val,                           -- out, for external user functions: '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active           => Ext_Rd_active,                         -- out, '1' => Rd-Cycle to external user register is active
      Ext_Rd_fin              => Ext_Rd_fin,                            -- out, marks end of read cycle, active one for one clock period
                                                                        -- of clk past cycle end (no overlap)
      Ext_Rd_Fin_ovl          => Ext_Rd_Fin_ovl,                        -- out, marks end of read cycle, active one for one clock period
                                                                        -- of clk during cycle end (overlap)
      Ext_Wr_active           => Ext_Wr_active,                         -- out, '1' => Wr-Cycle to external user register is active
      Ext_Wr_fin              => SCU_Ext_Wr_fin,                        -- out, marks end of write cycle, active high for one clock period
                                                                        -- of clk past cycle end (no overlap)
      Ext_Wr_fin_ovl          => Ext_Wr_fin_ovl,                        -- out, marks end of write cycle, active high for one clock period
                                                                        -- of clk before write cycle finished (with overlap)
      Deb_SCUB_Reset_out      => Deb_SCUB_Reset_out,                    -- out, the debounced 'nSCUB_Reset_in'-signal, is active high,
                                                                        -- can be used to reset
                                                                        -- external macros, when 'nSCUB_Reset_in' is '0'
      nPowerup_Res            => nPowerup_Res,                          -- out, this macro generates a power up reset
      Powerup_Done            => Powerup_Done                           -- out, this signal is set after powerup. Only the SCUB-Master can clear this bit.
      );

  lm32_ow: housekeeping
  generic map (
    Base_addr => c_lm32_ow_Base_Addr)
  port map (
    clk_sys     => clk_sys,
    clk_update  => clk_update,
    clk_flash   => clk_flash,
    rstn_sys    => rstn_sys,
    rstn_update => rstn_update,
    rstn_flash  => rstn_flash,


    ADR_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => wb_scu_rd_active,
    Data_to_SCUB      => wb_scu_data_to_SCUB,
    Dtack_to_SCUB     => wb_scu_dtack,

    owr_pwren_o       => owr_pwren_o,
    owr_en_o          => owr_en_o,
    owr_i             => owr_i,

    debug_serial_o    => uart_txd_out,
    debug_serial_i    => '0');


 rd_port_mux:  process(b_box_rd_active, wb_scu_rd_active, clk_switch_rd_active,b_box_rd_data,wb_scu_data_to_SCUB)

  variable sel: unsigned(2 downto 0);
  begin
  sel := b_box_rd_active & wb_scu_rd_active & clk_switch_rd_active;

    case sel is
      when "100" => Data_to_SCUB <= b_box_rd_data;
      when "010" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when "001" => Data_to_SCUB <= clk_switch_rd_data;

      when others      => Data_to_SCUB <= (others => '0');
    end case;
  end process rd_port_mux;



-------------- Dtack_to_SCUB -----------------------------

    Dtack_to_SCUB <= ( bb_dtack or wb_scu_dtack  or clk_switch_dtack );

    A_nDtack <= not(SCUB_Dtack);
    A_nSRQ   <= not(SCUB_SRQ);


UIO(15)<= Ext_Adr_Val;
UIO(14) <= Ext_Rd_active;
UIO(13) <= Ext_Wr_active;
UIO(12) <= bb_dtack;
UIO(11) <= b_box_rd_active;
UIO(10) <= wb_scu_dtack;
UIO(9) <= wb_scu_rd_active;
UIO(8) <= Dtack_to_SCUB;
UIO(7) <= not rstn_sys;
UIO(6) <= clk_sys;
UIO(5) <= not A_nEvent_Str;
UIO(4 downto 0) <= ADR_from_SCUB_LA(11 downto 7);

io_blackbox_el: io_blackbox 
generic map(
		nr_diob_ios          => BB_NR_DIOB_IOS,
		nr_virt_ios          =>	BB_NR_VIRT_IOS,
		nr_backplane_ios      => BB_NR_BACKPLANE_IOS,
		max_frontend_plugins => BB_MAX_FRONTEND_PLUGINS,
		max_proc_plugins     => BB_MAX_PROC_PLUGINS,
		max_user_plugins     => BB_MAX_USER_PLUGINS,
    frontend_status_bits   => BB_FRONTEND_STATUS_BITS,
		frontend_sel_bits    => integer(ceil (log2(real(BB_MAX_FRONTEND_PLUGINS)))),
		proc_sel_bits        => integer(ceil (log2(real(BB_MAX_PROC_PLUGINS)))),
		user_sel_bits        => integer(ceil (log2(real(BB_MAX_USER_PLUGINS)))),
		addr_bus_width       => BB_ADDR_BUS_WIDTH,
		data_bus_width       => BB_DATA_BUS_WIDTH
	)
  port map(
		-- Common
		clock                => clk_sys,          								
		reset                => not rstn_sys,           								  
		-- Frontend
		diob_io              => PIO(142 downto 16),     -- Connection to DIOB I/O 
		frontend_plugin_select => PIO(150 downto 143),  --I/O plugin selection
		backplane_io         => b_backplane, --UIO(15 downto 0),       --Backplane input/output fed (almost) directly	to user plugin
		-- SCU-bus
    addr                 => ADR_from_SCUB_LA,	    --(Adr_from_SCUB_LA)
		data_w               => Data_from_SCUB_LA,   -- (Data_from_SCUB_LA)
		data_r               => b_box_rd_data,	        --(Data_to_SCUB)

		addr_strobe          => Ext_Adr_Val, -- (Ext_Adr_Val)
	  read_trg             => Ext_Rd_active, -- (Ext_Rd_active)
		write_trg            => Ext_Wr_active, -- (Ext_Wr_active)
		read_fin             => Ext_Rd_fin, -- (Ext_Rd_fin)
		write_fin            => Ext_Wr_fin,	-- (Ext_Wr_fin)
		event_trg            => not A_nEvent_Str,   									
    
    dtack                => bb_dtack, --(Dtack_to_SCUB)
		data_r_act           => b_box_rd_active --(Reg_rd_active)	
    );
end architecture;


