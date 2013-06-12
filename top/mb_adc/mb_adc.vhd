library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

library work;
use work.adc_pkg.all;

entity mb_adc is
  port (
    VCXO_50MHz:   in std_logic;
    A_nMB_Reset:  in std_logic;
    
    --------------------------------------------------
    -- modulbus signals                             --
    --------------------------------------------------
    A_VG_ID:        in std_logic_vector(7 downto 0);
    A_VG_A:         in std_logic_vector(4 downto 0);
    A_A:            in std_logic_vector(4 downto 0);
    A_Sub_Adr:      in std_logic_vector(7 downto 0);
    A_RDnWR:        in std_logic;
    A_nDS:          in std_logic;
    A_VG_K1_INP:    in std_logic;
    A_VG_K0_INP:    in std_logic;
    A_GR0_APK_ID:   in std_logic;
    A_GR0_16BIT:    in std_logic;
    A_VG_K1_MOD:    in std_logic_vector(1 downto 0);
    A_VG_K0_MOD:    in std_logic_vector(1 downto 0);
    A_nGR0_ID_SEL:  out std_logic;
    A_Mod_Data:     inout std_logic_vector(7 downto 0);
    A_nDTACKA:      out std_logic;
    A_nDTACKB:      out std_logic;
    A_nINTERLOCKA:  out std_logic;
    A_nINTERLOCKB:  out std_logic;
    nID_OK_LED:     out std_logic;
    nDT_LED:        out std_logic;
    n_EXT_DATA_EN:  out std_logic;
    
    -- switches for modulbus data lines running over external drivers
    -- drivers are always switched to OUT and active when the card is selected
    SW_SUB_ADR_OUT: out std_logic; 
    nSW_SUB_ADR_EN: out std_logic;
    SW_B19_24_OUT:  out std_logic;
    nSW_B19_24_EN:  out std_logic;
    SW_DS_OUT:      out std_logic;
    nSW_DS_EN:      out std_logic;
    SW_MOD_ADR_OUT: out std_logic;
    nSW_MOD_ADR_EN: out std_logic;
    
    --------------------------------------------------
    -- leds on front panel                          --
    --------------------------------------------------
    nSKAL_OK_LED:   out std_logic := '1';
    nLED:         out std_logic_vector(15 downto 0);
    
    --------------------------------------------------
    -- switches on front panel                      --
    --------------------------------------------------
    nSEL_LED_GRP:       in std_logic_vector(1 downto 0);
    ADC_SEL_B:          in std_logic_vector(3 downto 0);
    
    V_Cntrl_Res:        in std_logic;
    A_nMANUAL_RES:      out std_logic := '1';
    
    ------------------------------------------------------------
    -- ADC signals                                            --
    ------------------------------------------------------------
    ADC_DB:             inout   std_logic_vector(15 downto 0);
    ADC_CONVST_A:       buffer  std_logic;
    ADC_CONVST_B:       buffer  std_logic;
    nADC_CS:            buffer  std_logic;
    nADC_RD_SCLK:       buffer  std_logic;
    ADC_BUSY:           in      std_logic;
    ADC_RESET:          buffer  std_logic;
    ADC_OS:             buffer  std_logic_vector(2 downto 0);
    nADC_PAR_SER_SEL:   buffer  std_logic;
    ADC_Range:          buffer  std_logic;
    ADC_FRSTDATA:       in      std_logic;    
    Ext_Trig:           in      std_logic;
    
    -------------------------------------------------------
    -- test ports                                        --
    -------------------------------------------------------
    TP:                 out std_logic_vector(8 downto 1);
    A_TEST:             out std_logic_vector(15 downto 0);
    A_LA_CLK:           out std_logic;
    
    -------------------------------------------------------
    -- IRQ                                               --
    -------------------------------------------------------
    A_nSRQB:            out     std_logic;
    A_nSRQA:            out     std_logic;
    A_nDRQB:            out     std_logic;
    A_nDRQA:            out     std_logic;
    
    ADC_FP_In_EN:       out     std_logic;
    nDiff_In_EN:        buffer  std_logic;
    A_I2C_SCL:          out     std_logic;
    
    -------------------------------------------------------
    -- channel 0                                         --
    -------------------------------------------------------
    A_K0_D:             inout std_logic_vector(15 downto 0);
    A_K0_LB_OUTS:       out std_logic;
    nA_K0_LB_ENA:       out std_logic;
    A_K0_HB_OUTS:       out std_logic;
    nA_K0_HB_ENA:       out std_logic;
    A_nK0_CTRL:         inout std_logic_vector(2 downto 1);
    A_nK0_ID_EN:        inout std_logic;
    
    
    -------------------------------------------------------
    -- channel 1                                         --
    -------------------------------------------------------
    A_nK1_ID_EN:        inout std_logic;
    A_nK1_CTRL:         inout std_logic_vector(2 downto 1)
    
    
    );
    
    constant clk_in_hz: integer := 50000000;
end entity;


architecture mb_adc_arch of mb_adc is
  component adc_pll
    PORT
    ( 
      inclk0		: IN STD_LOGIC  := '0';
      c0		    : OUT STD_LOGIC ;
      c1		    : OUT STD_LOGIC ;
      locked		: OUT STD_LOGIC 
    );
  end component;
  
  component pow_reset is
	port (
		clk:	        in std_logic;
		nreset:	      buffer std_logic
		);
  end component;
  
  component modulbus_v7 is
	generic (
			St_160_pol	  : INTEGER := 0;			-- 	0 ==> VG96-Modulbus,	1 ==> 160 poliger Modulbus (5-Reihig)				--
			Mod_Id		    : INTEGER := 16#55#;
			CLK_in_Hz	    : INTEGER := 50000000;	-- Damit das Design schnell auf eine andere Frequenz umgestellt werden kann, wird diese		--
												-- in Hz festgelegt. Z�hler die betimmte Zeiten realisieren sollen z.B. 'time_out_cnt'		--
												-- werden entprechend berechnet.
			Loader_Base_Adr	: INTEGER := 240;
			Res_Deb_in_ns	  : INTEGER := 100;
			nDS_Deb_in_ns	  : INTEGER := 20;
			Use_LPM		      : INTEGER := 0;
			Test		        : INTEGER := 0
			);
	port (
		Epld_Vers		  : IN	STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
		VG_Mod_Id		  : IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Der an diesem Modul-Bus-Steckplatz erwartete Karten-Typ (an der VG-Leiste fest verdrahtet).	--
		VG_Mod_Adr		: IN	STD_LOGIC_VECTOR(4 DOWNTO 0);	-- Adresse des Modul-Bus-Steckplatzes (an der VG-Leiste fest verdrahtet).						--
		VG_Mod_Skal		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Modul-Bus-Skalierung, f�r jeden Kartentyp unterschiedl. Bedeutung (an VG-Leiste verdrahtet).	--
		St_160_Skal		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);
		St_160_Auxi		: IN	STD_LOGIC_VECTOR(5 DOWNTO 0);
		Stat_IN			  : IN	STD_LOGIC_VECTOR(7 DOWNTO 2);
		Macro_Activ		: IN	STD_LOGIC := '1';
		Macro_Skal_OK	: IN	STD_LOGIC := '1';
		Mod_Adr			  : IN	STD_LOGIC_VECTOR(4 DOWNTO 0);	-- Adresse des gerade laufenden Modul-Bus-Zyklusses.											--	
		Sub_Adr			  : IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Sub-Adresse des gerade laufenden Modul-Bus-Zyklusses.										--	
		RDnWR			    : IN	STD_LOGIC;						-- Lese/Schreibsignal des Modul-Busses. RD/WR = 1 => Lesen.										--
		nDS				    : IN	STD_LOGIC;						-- Datenstrobe des Modul-Busses. /DS = 0 => aktiv.												--
		CLK				    : IN	STD_LOGIC;						-- Systemtakt des restlichen Designs sollte >= 12 Mhz sein.										--
		nMB_Reset		  : IN	STD_LOGIC;
		V_Data_Rd		  : IN	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Data to Modulbus, alle Daten-Quellen die au�erhalb dieses Macros liegen sollten hier �ber	--
																-- Multiplexer angeschlossen werden.															--
		nExt_Data_En	: OUT	STD_LOGIC;						-- Signal = 0, schaltet externen Datentreiber des Modul-Busses ein.

		Mod_Data		  : INOUT	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Daten-Bus des Modul-Busses.																	--

		nDt_Mod_Bus		: OUT	STD_LOGIC;						-- Data-Acknowlege zum Modul-Bus.																--
		Sub_Adr_La		: OUT	STD_LOGIC_VECTOR(7 DOWNTO 1);	
		Data_Wr_La		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	
		Extern_Wr_Activ	: OUT	STD_LOGIC;			
		Extern_Wr_Fin	: OUT	STD_LOGIC;		
		Extern_Rd_Activ	: OUT	STD_LOGIC;	
		Extern_Rd_Fin	: OUT	STD_LOGIC;
		Extern_Dtack	: IN	STD_LOGIC;						-- Alle extern dekodierten Modul-Bus-Aktionen, m�ssen hier ihr Dtack anlegen.					--
		Powerup_Res		: OUT	STD_LOGIC := '0';
		nInterlock		: OUT	STD_LOGIC;
		Timeout			: OUT	STD_LOGIC;
		Id_OK			: OUT	STD_LOGIC;
		nID_OK_Led		: OUT	STD_LOGIC;
		Led_Ena			: OUT	STD_LOGIC;
		nPower_Up_Led	: OUT	STD_LOGIC;
		nSel_Led		: OUT	STD_LOGIC;
		nDt_Led			: OUT	STD_LOGIC
	);
  end component;
  
  signal clk:           std_logic;
  signal locked:        std_logic;
  signal nrst:          std_logic;
  signal VG_Mod_Skal:   std_logic_vector(7 downto 0);
  
  signal nDt_Mod_Bus:     std_logic;
  signal nInterlock:      std_logic;
  signal ID_OK:           std_logic;
  signal V_Data_Rd:       std_logic_vector(15 downto 0);
  signal adc_dt_to_mb:    std_logic;
  signal sub_adr_la:      std_logic_vector(7 downto 1);
  signal data_wr_la:      std_logic_vector(15 downto 0);
  signal Extern_Wr_Activ: std_logic;
  signal Extern_Rd_Activ: std_logic;
  signal Powerup_Res:     std_logic;
  signal Led_Ena:         std_logic;
  
  signal  ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4: std_logic_vector(15 downto 0);
  signal  ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8: std_logic_vector(15 downto 0);
  
  constant c_led_cnt:				integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(250000000)));
	constant c_led_cnt_width:	integer := integer(floor(log2(real(c_led_cnt)))) + 2;
	signal	s_led_cnt:				unsigned( c_led_cnt_width-1 downto 0) := (others => '0');
  
  signal s_led_en:      std_logic;
  signal s_test_vector: std_logic_vector(15 downto 0) := x"8000";
  
  
begin

  pll: adc_pll
    port map (
      inclk0  => VCXO_50MHz,
      c0      => open,
      c1      => clk,
      locked  => locked);
   
  mb_macro: modulbus_v7
    generic map (
      St_160_pol      => 0,
      Mod_Id          => 43,
      Clk_in_Hz       => 50000000,
      Loader_Base_Adr => 240,
      Res_Deb_in_ns   => 100,
      nDS_Deb_in_ns   => 20,
      Use_LPM         => 0,
      Test            => 0)
    port map (
      Epld_Vers         => x"12",
      VG_Mod_Id         => A_VG_ID,
      VG_Mod_Adr        => A_VG_A,
      VG_Mod_Skal       => VG_Mod_Skal,
      St_160_Skal       => "00000000",
      St_160_Auxi       => "000000",
      Stat_IN           => "000000",
      Macro_Activ       => ID_OK,
      Macro_Skal_OK     => ID_OK,
      Mod_Adr           => A_A,
      Sub_Adr           => A_Sub_Adr,
      RDnWR             => A_RDnWR,
      nDS               => A_nDS,
      clk               => clk,
      nMB_Reset         => A_nMB_Reset,
      V_Data_Rd         => V_Data_Rd,
      Extern_Dtack      => adc_dt_to_mb,
      nExt_Data_En      => n_EXT_DATA_EN,
      Mod_Data          => A_Mod_Data,
      nDt_Mod_Bus       => nDt_Mod_Bus,
      Sub_Adr_La        => Sub_Adr_La,
      Data_Wr_La        => Data_Wr_La,
      Extern_Wr_Activ   => Extern_Wr_Activ,
      Extern_Wr_Fin     => open,
      Extern_Rd_Activ   => Extern_Rd_Activ,
      Extern_Rd_Fin     => open,
      Powerup_Res       => Powerup_Res,
      nInterlock        => nInterlock,
      Id_OK             => ID_OK,
      nID_OK_Led        => nID_OK_LED,
      Led_Ena           => Led_Ena,
      nPower_Up_Led     => open,
      nSel_Led          => open,
      nDt_Led           => nDT_LED
      );
  
  adc: adc_modul_bus
  generic map (
    clk_in_Hz     => 50000000,
    diag_on_is_1  => 1)
  port map (
    clk           =>  clk,
    nrst          =>  locked,
    
    db            => ADC_DB(13 downto 0),
    db14_hben     => ADC_DB(14),
    db15_byte_sel => ADC_DB(15),
    convst_a      => ADC_CONVST_A,
    convst_b      => ADC_CONVST_B,
    n_cs          => nADC_CS,
    n_rd_sclk     => nADC_RD_SCLK,
    busy          => ADC_BUSY,
    adc_reset     => ADC_RESET,
    os            => ADC_OS,
    par_ser_sel   => nADC_PAR_SER_SEL,
    adc_range     => ADC_Range,
    firstdata     => ADC_FRSTDATA,
    nDiff_In_En   => NDIFF_IN_EN,
    
    sub_adr_la    => Sub_Adr_La,
    data_wr_la    => Data_Wr_La,
		read_data     => V_Data_Rd,
		rd_activ      => Extern_Rd_Activ,
		wr_activ      => Extern_Wr_Activ,
    adc_dt_to_mb  => adc_dt_to_mb,    

    channel_1 => ADC_channel_1,
    channel_2 => ADC_channel_2,
    channel_3 => ADC_channel_3,
    channel_4 => ADC_channel_4,
    channel_5 => ADC_channel_5,
    channel_6 => ADC_channel_6,
    channel_7 => ADC_channel_7,
    channel_8 => ADC_channel_8);
   
  
  VG_Mod_Skal(7) <= A_VG_K1_INP;
  VG_Mod_Skal(6) <= A_VG_K0_INP;
  VG_Mod_Skal(5) <= A_GR0_APK_ID;
  VG_Mod_Skal(4) <= A_GR0_16BIT;
  VG_Mod_Skal(3) <= A_VG_K1_MOD(1);
  VG_Mod_Skal(2) <= A_VG_K1_MOD(0);
  VG_Mod_Skal(1) <= A_VG_K0_MOD(1);
  VG_Mod_Skal(0) <= A_VG_K0_MOD(0);
  
  nrst <= locked;
 
  -------------------------------------------------------------------------------
	-- precsaler for an enable per second
	-------------------------------------------------------------------------------
	sec_prescale:	process(clk, nrst)
	begin
		if nrst = '0' then
			s_led_en <= '0';
		elsif rising_edge(clk) then
			if s_led_cnt = to_unsigned(c_led_cnt, c_led_cnt_width) then
				s_led_en <= '1';
				s_led_cnt <= (others => '0');
			else
				s_led_en <= '0';
				s_led_cnt <= s_led_cnt  + 1;
			end if;
		end if;
	end process;
	
	-------------------------------------------------------------------------------
	-- rotating bit as a test vector for led testing
	-------------------------------------------------------------------------------
	test_signal: process(clk, nrst, s_led_en)
	begin
		if nrst = '0' then
			s_test_vector <= ('1', others => '0');
		elsif rising_edge(clk) and s_led_en = '1' then
			s_test_vector <= s_test_vector(s_test_vector'high-1 downto 0) & s_test_vector(s_test_vector'high);
		end if;
	end process;
 
  p_led_mux: process (
    ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4,
    ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8,
    ADC_SEL_B(3 downto 0), nSEL_LED_GRP(1 downto 0),
    nADC_PAR_SER_SEL, NDIFF_IN_EN
    )
  begin
    if nSEL_LED_GRP = "11" then
      nLED <= not nADC_PAR_SER_SEL & nADC_PAR_SER_SEL & NDIFF_IN_EN & "1" & x"FFF";
    elsif nSEL_LED_GRP = "01" then
      case not ADC_SEL_B IS
        when X"1" => nLED <= not ADC_channel_1;
        when X"2" => nLED <= not ADC_channel_2;
        when X"3" => nLED <= not ADC_channel_3;
        when X"4" => nLED <= not ADC_channel_4;
        when X"5" => nLED <= not ADC_channel_5;
        when X"6" => nLED <= not ADC_channel_6;
        when X"7" => nLED <= not ADC_channel_7;
        when X"8" => nLED <= not ADC_channel_8;
        when others =>
          nLED <= (others => '1');
      end case;
    elsif nSEL_LED_GRP = "10" then
      nLED <= not s_test_vector;
    else
      nLED <= (others => '1');
    end if;
  end process p_led_mux;
 
 
 
  A_nMANUAL_RES <= '1';
  A_nDTACKA     <= nDt_Mod_Bus;
  A_nDTACKB     <= nDt_Mod_Bus;
  A_nINTERLOCKA <= nInterlock;
  A_nINTERLOCKB <= nInterlock;
  nSKAL_OK_LED  <= '1';
  
  A_nK0_CTRL  <= "ZZ";
  A_nK0_ID_EN <= 'Z';
  A_nK1_ID_EN <= 'Z';
  A_nK1_CTRL  <= "ZZ";
  
  A_nSRQB     <= 'Z';
  A_nSRQA     <= 'Z';
  A_nDRQB     <= 'Z';
  A_nDRQA     <= 'Z';
  
  A_I2C_SCL     <= 'Z';
  A_nGR0_ID_SEL <= 'Z';
  
  ADC_FP_In_EN  <= '1';

end architecture;