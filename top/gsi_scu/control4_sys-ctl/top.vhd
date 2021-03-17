library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;


entity top is
  generic (
    CLK_sys_in_Hz:      integer := 100000000
        );
  port (
    clk_base_i        : in    std_logic;
    fx2_clk           : in    std_logic;                      -- Clock from FX2 USB Controller
    scu_cb_revision   : in    std_logic_vector(3 downto 0);   -- must be assigned with weak pull ups
    fpga_con_io       : inout std_logic_vector(7 downto 0);   -- Connection to Arria 10
    --I2C to COMX
    i2c_scl           : in    std_logic;
    i2c_sda           : inout std_logic;
    smb_scl           : in    std_logic;                      --SMB clock
    smb_sda           : inout std_logic :='Z';                --SMB data
	 
	 --UART-to COMX
	 uart_tx				 : in 	std_logic;
	 uart_rx				 : out 	std_logic;

    pGood             : in    std_logic_vector (3 downto 0);  -- Power good (0.95V, 1.8V, 3.3V, 5V)
    jtag_present	  	: in		std_logic;							        -- JTAG TCK Signal
    nPB_user_in       : in    std_logic;                      -- User Push-Button In
    wdt               : in    std_logic;                      -- Watchdog COMX
    rtc_voltage       : in    std_logic;                      -- Voltage Level for RTC-VCC 

    -----------------------------------------------------------------------
    -- LPC interface from ComExpress
    -----------------------------------------------------------------------
    LPC_AD         : inout std_logic_vector(3 downto 0);
    LPC_FPGA_CLK   : in    std_logic;
    LPC_SERIRQ     : inout std_logic;
    --nLPC_DRQ0      : in    std_logic;
    nLPC_FRAME     : in    std_logic;

    --Arria 10 status
    CONF_DONE         : in    std_logic;
    INIT_DONE         : in    std_logic;
    nSTATUS           : in    std_logic;

    --Reset In
    nCB_rst           : in    std_logic;                    -- Reset from COMX
    nSCUext_rst_in    : in    std_logic;                    -- Reset form SCU-Bus Extension
    nExt_rst_in       : in    std_logic;                    -- Reset form Extension Connector
    nPB_rst_in        : in    std_logic;                    -- Reset form Push Button
    nFPGA_rst_in      : in    std_logic;                    -- Reset from Arria10
    --Reset Out
    nSYS_rst          : out   std_logic :='0';              -- Reset Out to COMX
	 nArria_rst			 : out	std_logic :='0';					-- Reset Out to Arria10 
    nPCI_rst_out      : out   std_logic :='0';              -- PCI Reset Out
    nExt_rst_out      : out   std_logic :='0';              -- Reset to Extension Connector
    --Power
    core_en           : out   std_logic :='0';              -- Enable 0.95V Core voltage
    volt_1_8_en       : out   std_logic :='0';              -- Enable 1.8V Rail
    volt_1_8_IO_en    : out   std_logic :='0';              -- Enable IO 1.8V Rail (MOSFET)
    volt_5_en         : out   std_logic :='0';              -- Enable 5V Rail
    pwr_ok				    : out	  std_logic :='0';				      -- Power Ok COMX
    wake              : out   std_logic :='Z';              -- PCIe Wake 

    IO_enable         : out   std_logic;                    -- Enable Levelshifter 1.8V  ->  3.3V
    nPB_user_out      : out   std_logic;                    -- User Push-Button Out

    led_status_o      : out   std_logic_vector(2 downto 0)
  );
end top;
--
architecture rtl of top is

  component zeitbasis
    generic (
        CLK_in_Hz:      integer;
        diag_on:      integer
        );
    port  (
        Res:        in  std_logic;
        Clk:        in  std_logic;
        Ena_every_100ns:  out std_logic;
        Ena_every_166ns:  out std_logic;
        Ena_every_250ns:  out std_logic;
        Ena_every_500ns:  out std_logic;
        Ena_every_1us:    out std_logic;
        Ena_Every_20ms:   out std_logic
        );
    end component;
	 
	 component main_pll
	 port
		(
			areset	: in std_logic  := '0';
			inclk0	: in std_logic  := '0';
			c0		: out std_logic ;
			locked	: out std_logic 
		);
	end component;


  COMPONENT adc_control
	PORT
	(
		clk		:	 IN STD_LOGIC;
		clk_pll		:	 IN STD_LOGIC;
		pll_locked		:	 IN STD_LOGIC;
		nreset	:	 IN STD_LOGIC;
		channel_0: out STD_LOGIC_VECTOR (11 downto 0);  
		channel_1: out STD_LOGIC_VECTOR (11 downto 0); 
		channel_2: out STD_LOGIC_VECTOR (11 downto 0);  
		channel_3: out STD_LOGIC_VECTOR (11 downto 0); 
		channel_4: out STD_LOGIC_VECTOR (11 downto 0);  
		channel_5: out STD_LOGIC_VECTOR (11 downto 0);
		channel_6: out STD_LOGIC_VECTOR (11 downto 0); 
		channel_7: out STD_LOGIC_VECTOR (11 downto 0);  
		channel_8: out STD_LOGIC_VECTOR (11 downto 0); 
		tsd: out STD_LOGIC_VECTOR  (11 downto 0));  
	END COMPONENT;


  COMPONENT debounce
	PORT(
		clk : IN std_logic;
		input : IN std_logic;          
		output : OUT std_logic;
		en_deb : in std_logic
		);
	END COMPONENT;


  constant  vcc12_up_thres:     integer   := 3615; --11.9V / ( 812muV * 4.01 )   
  constant  vcc12_fail_thres:   integer   := 3038; --10.0V / ( 812muV * 4.01 )
  constant  vccCore_up_thres:   integer   := 1052; --(0.95V * 0.9) / 812muV
  constant  vccCore_down_thres: integer   := 117;  --(0.95V * 0.1) /812mV 
  constant  vcc1_8_up_thres:    integer   := 1995; --(1.8V * 0.9) / 812muV
  constant  vcc1_8_down_thres:  integer   := 221;  --(1.8V * 0.1) /812mV 

  signal clk_40MHz	  : std_logic;
  signal pll_locked	  : std_logic;
  signal countx       : std_logic_vector(15 downto 0);
  signal rst_n        : std_logic;

  signal Ena_Every_100ns: std_logic;
  signal Ena_Every_166ns: std_logic;
  signal Ena_Every_250ns: std_logic;
  signal Ena_Every_500ns: std_logic;
  signal Ena_Every_20ms:  std_logic;
  signal Ena_Every_1us:   std_logic;

  signal adc_value_Vcc_12:        std_logic_vector (11 downto 0) := X"000";  
  signal adc_value_Vcc_3_3:       std_logic_vector (11 downto 0) := X"000";  
  signal adc_value_Vcc_5_0:       std_logic_vector (11 downto 0) := X"000";
  signal adc_value_Vcc_1_8IO:     std_logic_vector (11 downto 0) := X"000";
  signal adc_value_Vcc_3_3clean:  std_logic_vector (11 downto 0) := X"000";  
  signal adc_value_Vcc_0_95:      std_logic_vector (11 downto 0) := X"000";
  signal adc_value_Vcc_1_8:       std_logic_vector (11 downto 0) := X"000";
  signal adc_value_Vcch_gxb:      std_logic_vector (11 downto 0) := X"000";
  signal adc_value_Vccrt_gxb:     std_logic_vector (11 downto 0) := X"000";

  signal vcc12_up:       std_logic;
  signal nVcc12_fail:    std_logic;
  signal VccCore_up:     std_logic;
  signal nVccCore_down:  std_logic;
  signal Vcc1_8_up:      std_logic;
  signal nVcc1_8_down:   std_logic;
  signal Vcc1_8IO_up:    std_logic;
  signal nVcc1_8IO_down: std_logic;
  signal vcc_deb_in:     std_logic_vector (7 downto 0);
  signal vcc_deb_out:    std_logic_vector (7 downto 0);

  begin

    main_pll_inst : main_pll
		port map (
			areset	 => '0',
			inclk0	 => clk_base_i,
			c0		   => clk_40MHz,
			locked	 => pll_locked
		);




  process(clk_base_i, rst_n) begin
    if (rst_n = '0') then
      countx <= (others => '0');
    elsif (rising_edge(clk_base_i)) then
      countx <= countx + 1;
    end if;
  end process;

  led_status_o(0) <= countx(15);
  led_status_o(1) <= countx(0);

  fpga_con_io <= (others => 'Z');


  rst_n <= nSCUext_rst_in and nExt_rst_in and nPB_rst_in and nFPGA_rst_in;
  nArria_rst <= nSCUext_rst_in and nExt_rst_in and nPB_rst_in;
  nSYS_rst <= rst_n;
  nPCI_rst_out <= nCB_rst;
  nExt_rst_out <= nCB_rst;

  IO_enable <= INIT_DONE;
  nPB_user_out <= nPB_user_in;


  zeit1 : zeitbasis
    generic map (
          CLK_in_Hz =>  clk_sys_in_Hz,
          diag_on   =>  1
          )
    port map  (
          Res               =>  not rst_n,
          Clk               =>  clk_base_i,
          Ena_every_100ns   =>  Ena_Every_100ns,
          Ena_every_166ns   =>  Ena_Every_166ns,
          Ena_every_250ns   =>  Ena_every_250ns,
          Ena_every_500ns   =>  Ena_every_500ns,
          Ena_every_1us     =>  Ena_every_1us,
          Ena_Every_20ms    =>  Ena_Every_20ms
          );

--ADC and ADC-based Signals
------------------------------------------------------------------          
          adc_0 : adc_control 
          Port map ( 
            clk 		=> clk_40MHz,
            clk_pll 	=> clk_40MHz,
            pll_locked  => pll_locked,
            nreset 		=> rst_n,
            channel_0   => adc_value_Vcc_12,  
            channel_1   => adc_value_Vcc_3_3, 
            channel_2   => adc_value_Vcc_5_0,  
            channel_3   => adc_value_Vcc_1_8IO, 
            channel_4   => adc_value_Vcc_3_3clean,  
            channel_5   => adc_value_Vcc_0_95,
            channel_6   => adc_value_Vcc_1_8, 
            channel_7   => adc_value_Vcch_gxb,  
            channel_8   => adc_value_Vccrt_gxb, 
            tsd			=> open
        );
        


        vcc_deb_in(0)  <= '1'  when to_integer(unsigned(adc_value_Vcc_12))    >= vcc12_up_thres else '0';
        vcc_deb_in(1)  <= '1'  when to_integer(unsigned(adc_value_Vcc_12))    >= vcc12_fail_thres else '0';
  		  vcc_deb_in(2)  <= '1'  when to_integer(unsigned(adc_value_Vcc_0_95))  >= vccCore_up_thres else '0';
        vcc_deb_in(3)  <= '1'  when to_integer(unsigned(adc_value_Vcc_0_95))  >= vccCore_down_thres else '0';
        vcc_deb_in(4)  <= '1'  when to_integer(unsigned(adc_value_Vcc_1_8))   >= vcc1_8_up_thres else '0';
        vcc_deb_in(5)  <= '1'  when to_integer(unsigned(adc_value_Vcc_1_8))   >= vcc1_8_down_thres else '0';
        vcc_deb_in(6)  <= '1'  when to_integer(unsigned(adc_value_Vcc_1_8IO)) >= vcc1_8_up_thres else '0';
        vcc_deb_in(7)  <= '1'  when to_integer(unsigned(adc_value_Vcc_1_8IO)) >= vcc1_8_down_thres else '0';

    ADC_Deb:  for I in 0 to 7 generate
    Inst_debounce_I: debounce PORT MAP(
      clk => clk_base_i,
      input => vcc_deb_in(I),
      output => vcc_deb_out(I),
      en_deb => Ena_every_1us
    );
    end generate ADC_Deb;

    vcc12_up       <= vcc_deb_out(0);
    nVcc12_fail    <= vcc_deb_out(1);
    vccCore_up     <= vcc_deb_out(2);
    nVccCore_down  <= vcc_deb_out(3);
    vcc1_8_up      <= vcc_deb_out(4);
    nVcc1_8_down   <= vcc_deb_out(5);
    vcc1_8IO_up    <= vcc_deb_out(6);
    nVcc1_8IO_down <= vcc_deb_out(7);

end;
