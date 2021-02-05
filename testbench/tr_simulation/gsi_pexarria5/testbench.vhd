library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

entity testbench is
generic (g_en_simbridge : boolean);
end entity;

architecture simulation of testbench is

    signal clk_20m_vcxo_i    : std_logic := '1';  -- 20MHz VCXO clock
    signal clk_125m_pllref_i : std_logic := '1';  -- 125 MHz PLL reference
    signal clk_125m_local_i  : std_logic := '1';  -- local clk from 125Mhz oszillator

    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    signal pcie_refclk_i  : std_logic := '1';
    signal pcie_rx_i      : std_logic_vector(3 downto 0) := (others => '0');
    signal pcie_tx_o      : std_logic_vector(3 downto 0);
    signal nPCI_RESET     : std_logic := '0';

    signal pe_smdat        : std_logic := 'Z'; -- !!!
    signal pe_snclk        : std_logic;   -- !!!
    signal pe_waken        : std_logic;   -- !!!

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    signal dac_sclk       : std_logic;
    signal dac_din        : std_logic;
    signal ndac_cs        : std_logic_vector(2 downto 1);

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    signal rom_data        : std_logic := 'Z';

    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    signal di              : std_logic_vector(6 downto 0);
    signal ai              : std_logic_vector(1 downto 0) := (others => '0');
    signal dout_LCD        : std_logic := '1';
    signal wrdis           : std_logic := '0';
    signal dres            : std_logic := '1';

    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    signal fpga_res        : std_logic := '0';
    signal nres            : std_logic := '1';
    signal pbs2            : std_logic := '0'; -- connected to core_rstn_i of monster
    signal hpw             : std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
    signal ant             : std_logic_vector(26 downto 1) := (others => 'Z'); -- trigger bus

    -----------------------------------------------------------------------
    -- pexaria5db1/2
    -----------------------------------------------------------------------
    signal p1              : std_logic := 'Z'; -- HPWX0 logic analyzer: 3.3V
    signal n1              : std_logic := 'Z'; -- HPWX1
    signal p2              : std_logic := 'Z'; -- HPWX2
    signal n2              : std_logic := 'Z'; -- HPWX3
    signal p3              : std_logic := 'Z'; -- HPWX4
    signal n3              : std_logic := 'Z'; -- HPWX5
    signal p4              : std_logic := 'Z'; -- HPWX6
    signal n4              : std_logic := 'Z'; -- HPWX7
    signal p5              : std_logic := 'Z'; -- LED1 1-6: 3.3V (red)   1|Z=off, 0=on
    signal n5              : std_logic := 'Z'; -- LED2           (blue)
    signal p6              : std_logic := 'Z'; -- LED3           (green)
    signal n6              : std_logic := 'Z'; -- LED4           (white)
    signal p7              : std_logic := 'Z'; -- LED5           (red)
    signal n7              : std_logic := 'Z'; -- LED6           (blue)
    signal p8              : std_logic := 'Z'; -- LED7 7-8: 2.5V (green)
    signal n8              : std_logic := 'Z'; -- LED8           (white)

    signal p9              : std_logic := 'Z'; -- TERMEN1 = terminate TTLIO1, 1=x, 0|Z=x (Q2 BSH103 -- G pin)
    signal n9              : std_logic := 'Z'; -- TERMEN2 = terminate TTLIO2, 1=x, 0|Z=x
    signal p10             : std_logic := 'Z'; -- TERMEN3 = terminate TTLIO3, 1=x, 0|Z=x
    signal n10             : std_logic := 'Z'; -- TTLEN1  = TTLIO1 output enable, 0=enable, 1|Z=disable
    signal p11             : std_logic := 'Z'; -- n/c
    signal n11             : std_logic := 'Z'; -- TTLEN3  = TTLIO2 output enable, 0=enable, 1|Z=disable
    signal p12             : std_logic := 'Z'; -- n/c
    signal n12             : std_logic := 'Z'; -- n/c
    signal p13             : std_logic := 'Z'; -- n/c
    signal n13             : std_logic := 'Z'; -- n/c
    signal p14             : std_logic := 'Z'; -- n/c
    signal n14             : std_logic := 'Z'; -- TTLEN5  = TTLIO3 output enable, 0=enable, 1|Z=disable
    signal p15             : std_logic := 'Z'; -- n/c
    signal n15             : std_logic := 'Z'; -- ROM_DATA
    signal p16             : std_logic := 'Z'; -- FPLED5  = TTLIO3 (red)  0=on, Z=off
    signal n16             : std_logic := 'Z'; -- FPLED6           (blue)

    signal p17             : std_logic := '0';        -- N_LVDS_1 / SYnIN
    signal n17             : std_logic := '1';        -- P_LVDS_1 / SYpIN
    signal p18             : std_logic := '0';        -- N_LVDS_2 / TRnIN
    signal n18             : std_logic := '1';        -- P_LVDS_2 / TRpIN
    signal p19             : std_logic;        -- N_LVDS_3 / CK200n
     --n19             : out   std_logic;        -- P_LVDS_3 / CK200p -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
    signal p21             : std_logic := '0';        -- N_LVDS_6  = TTLIO1 in
    signal n21             : std_logic := '1';        -- P_LVDS_6
    signal p22             : std_logic := '0';        -- N_LVDS_8  = TTLIO2 in
    signal n22             : std_logic := '1';        -- P_PVDS_8
    signal p23             : std_logic := '0';        -- N_LVDS_10 = TTLIO3 in
    signal n23             : std_logic := '1';        -- P_LVDS_10
    signal p24             : std_logic := '0';        -- N_LVDS_4 / SYnOU
     --n24             : out   std_logic;        -- P_LVDS_4 / SYpOU -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
    signal p25             : std_logic := '0';        -- N_LVDS_5  = TTLIO1 out
    signal n25             : std_logic := '1';        -- P_LVDS_5
    signal p26             : std_logic := 'Z'; -- FPLED3    = TTLIO2 (red)  0=on, Z=off
    signal n26             : std_logic := 'Z'; -- FPLED4             (blue)
    signal p27             : std_logic := '0';        -- N_LVDS_7  = TTLIO2 out
    signal n27             : std_logic := '1';        -- P_LVDS_7
    signal p28             : std_logic := '0';        -- N_LVDS_9  = TTLIO3 out
    signal n28             : std_logic := '1';        -- P_LVDS_9
    signal p29             : std_logic := 'Z'; -- FPLED1    = TTLIO1 (red)  0=on, Z=off
    signal n29             : std_logic := 'Z'; -- FPLED2             (blue)
    signal p30             : std_logic := 'Z'; -- n/c
    signal n30             : std_logic := 'Z'; -- n/c

    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    signal con             : std_logic_vector(5 downto 1);

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    signal slrd            : std_logic;
    signal slwr            : std_logic;
    signal fd              : std_logic_vector(7 downto 0) := (others => 'Z');
    signal pa              : std_logic_vector(7 downto 0) := (others => 'Z');
    signal ctl             : std_logic_vector(2 downto 0) := (others => 'Z');
    signal uclk            : std_logic := '0';
    signal ures            : std_logic;

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    signal led             : std_logic_vector(8 downto 1) := (others => '1');

    -----------------------------------------------------------------------
    -- leds SFPs
    -----------------------------------------------------------------------
    signal ledsfpr          : std_logic_vector(4 downto 1);
    signal ledsfpg          : std_logic_vector(4 downto 1);
    signal sfp234_ref_clk_i : std_logic := '1';

    -----------------------------------------------------------------------
    -- SFP1
    -----------------------------------------------------------------------

    signal sfp1_tx_disable_o : std_logic := '0';
    signal sfp1_tx_fault     : std_logic := '0';
    signal sfp1_los          : std_logic := '0';

     --sfp1_txp_o        : out std_logic;
     --sfp1_rxp_i        : in  std_logic;

    signal sfp1_mod0         : std_logic := 'Z'; -- grounded by module
    signal sfp1_mod1         : std_logic := 'Z'; -- SCL
    signal sfp1_mod2         : std_logic := 'Z'; -- SDA

    -----------------------------------------------------------------------
    -- SFP2
    -----------------------------------------------------------------------

    signal sfp2_tx_disable_o : std_logic := '0';
    signal sfp2_tx_fault     : std_logic := '0';
    signal sfp2_los          : std_logic := '0';

     --sfp2_txp_o        : out std_logic;
     --sfp2_rxp_i        : in  std_logic;

    signal sfp2_mod0         : std_logic := 'Z'; -- grounded by module
    signal sfp2_mod1         : std_logic := 'Z'; -- SCL
    signal sfp2_mod2         : std_logic := 'Z'; -- SDA

    -----------------------------------------------------------------------
    -- SFP3
    -----------------------------------------------------------------------

    signal sfp3_tx_disable_o : std_logic := '0';
    signal sfp3_tx_fault     : std_logic := '0';
    signal sfp3_los          : std_logic := '0';

     --sfp3_txp_o        : out std_logic;
     --sfp3_rxp_i        : in  std_logic;

    signal sfp3_mod0         : std_logic := 'Z'; -- grounded by module
    signal sfp3_mod1         : std_logic := 'Z'; -- SCL
    signal sfp3_mod2         : std_logic := 'Z'; -- SDA

    -----------------------------------------------------------------------
    -- SFP4
    -----------------------------------------------------------------------

    signal sfp4_tx_disable_o : std_logic := '0';
    signal sfp4_tx_fault     : std_logic := '0';
    signal sfp4_los          : std_logic := '0';

    signal sfp4_txp_o        : std_logic;
    signal sfp4_rxp_i        : std_logic := '0';

    signal sfp4_mod0         : std_logic := 'Z'; -- grounded by module
    signal sfp4_mod1         : std_logic := 'Z'; -- SCL
    signal sfp4_mod2         : std_logic := 'Z'; -- SDA

begin

  simbridge_y: if g_en_simbridge generate
    chip : entity work.ez_usb_chip
      generic map(g_stop_until_client_connects => false)
      port map (
        rstn_i    => '0', -- always in reset
        wu2_o     => pa(3),
        readyn_o  => pa(7),
        fifoadr_i => "00",
        fulln_o   => ctl(1),
        emptyn_o  => ctl(2),
        sloen_i   => '1',
        slrdn_i   => '1',
        slwrn_i   => '1',
        pktendn_i => pa(6),
        fd_io     => fd
        );
  end generate;
  simbridge_n: if not g_en_simbridge generate
    chip : entity work.ez_usb_chip
      generic map(g_stop_until_client_connects => false)
      port map (
        rstn_i    => ures,
        wu2_o     => pa(3),
        readyn_o  => pa(7),
        fifoadr_i => pa(5 downto 4),
        fulln_o   => ctl(1),
        emptyn_o  => ctl(2),
        sloen_i   => pa(2),
        slrdn_i   => slrd,
        slwrn_i   => slwr,
        pktendn_i => pa(6),
        fd_io     => fd
        );
  end generate;


  --wrex : entity work.wr_timing
  --port map(
  --  dac_hpll_load_p1_i => dac_hpll_load_p1,
  --  dac_hpll_data_i    => dac_hpll_data,
  --  dac_dpll_load_p1_i => dac_dpll_load_p1,
  --  dac_dpll_data_i    => dac_dpll_data,
  --  clk_ref_125_o      => clk_ref,
  --  clk_sys_62_5_o     => open,
  --  clk_dmtd_20_o      => clk_dmtd
  --);

  tr : entity work.pci_control 
  generic map (
    g_simulation => true,
    g_en_simbridge => g_en_simbridge
    )
  port map(
    clk_20m_vcxo_i    => clk_20m_vcxo_i,
    clk_125m_pllref_i => clk_125m_pllref_i,
    clk_125m_local_i  => clk_125m_local_i,

    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_refclk_i     => pcie_refclk_i,
    pcie_rx_i         => pcie_rx_i,
    pcie_tx_o         => pcie_tx_o,
    nPCI_RESET        => nPCI_RESET,

    pe_smdat          => pe_smdat,
    pe_snclk          => pe_snclk,
    pe_waken          => pe_waken,

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk          => dac_sclk,
    dac_din           => dac_din,
    ndac_cs           => ndac_cs,

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data          => rom_data,

    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    di                => di,
    ai                => ai,
    dout_LCD          => dout_LCD,
    wrdis             => wrdis,
    dres              => dres,

    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    fpga_res          => fpga_res,
    nres              => nres,
    pbs2              => pbs2,
    hpw               => hpw,
    ant               => ant,

    -----------------------------------------------------------------------
    -- pexaria5db1/2
    -----------------------------------------------------------------------
    p1                => p1,
    n1                => n1,
    p2                => p2,
    n2                => n2,
    p3                => p3,
    n3                => n3,
    p4                => p4,
    n4                => n4,
    p5                => p5,
    n5                => n5,
    p6                => p6,
    n6                => n6,
    p7                => p7,
    n7                => n7,
    p8                => p8,
    n8                => n8,

    p9                => p9,
    n9                => n9,
    p10               => p10,
    n10               => n10,
    p11               => p11,
    n11               => n11,
    p12               => p12,
    n12               => n12,
    p13               => p13,
    n13               => n13,
    p14               => p14,
    n14               => n14,
    p15               => p15,
    n15               => n15,
    p16               => p16,
    n16               => n16,

    p17               => p17,
    n17               => n17,
    p18               => p18,
    n18               => n18,
    p19               => p19,
    --n19             : out   std_logic;        -- P_LVDS_3 / CK200p -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
    p21               => p21,
    n21               => n21,
    p22               => p22,
    n22               => n22,
    p23               => p23,
    n23               => n23,
    p24               => p24,
    --n24             : out   std_logic;        -- P_LVDS_4 / SYpOU -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
    p25               => p25,
    n25               => n25,
    p26               => p26,
    n26               => n26,
    p27               => p27,
    n27               => n27,
    p28               => p28,
    n28               => n28,
    p29               => p29,
    n29               => n29,
    p30               => p30,
    n30               => n30,

    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con               => con,

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd              => slrd,
    slwr              => slwr,
    fd                => fd,
    pa                => pa,
    ctl               => ctl,
    uclk              => uclk,
    ures              => ures,

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    led               => led,

    -----------------------------------------------------------------------
    -- leds SFPs
    -----------------------------------------------------------------------
    ledsfpr            => ledsfpr,
    ledsfpg            => ledsfpg,
    sfp234_ref_clk_i   => sfp234_ref_clk_i,

    -----------------------------------------------------------------------
    -- SFP1
    -----------------------------------------------------------------------

    sfp1_tx_disable_o   => sfp1_tx_disable_o,
    sfp1_tx_fault       => sfp1_tx_fault,
    sfp1_los            => sfp1_los,

    --sfp1_txp_o        : out std_logic;
    --sfp1_rxp_i        : in  std_logic;

    sfp1_mod0           => sfp1_mod0,
    sfp1_mod1           => sfp1_mod1,
    sfp1_mod2           => sfp1_mod2,

    -----------------------------------------------------------------------
    -- SFP2
    -----------------------------------------------------------------------

    sfp2_tx_disable_o   => sfp2_tx_disable_o,
    sfp2_tx_fault       => sfp2_tx_fault,
    sfp2_los            => sfp2_los,

    --sfp2_txp_o        : out std_logic;
    --sfp2_rxp_i        : in  std_logic;

    sfp2_mod0           => sfp2_mod0,
    sfp2_mod1           => sfp2_mod1,
    sfp2_mod2           => sfp2_mod2,

    -----------------------------------------------------------------------
    -- SFP3
    -----------------------------------------------------------------------

    sfp3_tx_disable_o   => sfp3_tx_disable_o,
    sfp3_tx_fault       => sfp3_tx_fault,
    sfp3_los            => sfp3_los,

    --sfp3_txp_o        : out std_logic;
    --sfp3_rxp_i        : in  std_logic;

    sfp3_mod0           => sfp3_mod0,
    sfp3_mod1           => sfp3_mod1,
    sfp3_mod2           => sfp3_mod2,

    -----------------------------------------------------------------------
    -- SFP4
    -----------------------------------------------------------------------

    sfp4_tx_disable_o   => sfp4_tx_disable_o,
    sfp4_tx_fault       => sfp4_tx_fault,
    sfp4_los            => sfp4_los,

    sfp4_txp_o          => sfp4_txp_o,
    sfp4_rxp_i          => sfp4_rxp_i,

    sfp4_mod0           => sfp4_mod0,
    sfp4_mod1           => sfp4_mod1,
    sfp4_mod2           => sfp4_mod2
    );


    clk_20m_vcxo_i    <= not clk_20m_vcxo_i    after 25 ns; -- 20MHz VCXO clock
    clk_125m_pllref_i <= not clk_125m_pllref_i after  4 ns; -- 125 MHz PLL reference
    clk_125m_local_i  <= not clk_125m_local_i  after  4 ns; -- local clk from 125Mhz oszillator
    pbs2              <= '1' after 50 ns;                   -- release reset


end architecture;



