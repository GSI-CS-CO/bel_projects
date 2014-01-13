library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;

entity pci_control is
  port(
    clk_20m_vcxo_i    : in std_logic;  -- 20MHz VCXO clock
--    clk_125m_pllref_i : in std_logic;  -- 125 MHz PLL reference
    clk_125m_local_i  : in std_logic;  -- local clk from 125Mhz oszillator
    
    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_refclk_i  : in  std_logic;
    pcie_rx_i      : in  std_logic_vector(3 downto 0);
    pcie_tx_o      : out std_logic_vector(3 downto 0);
    nPCI_RESET     : in std_logic;
    
    pe_smdat        : inout std_logic; -- !!!
    pe_snclk        : out std_logic;   -- !!!
    pe_waken        : out std_logic;   -- !!!
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk       : out std_logic;
    dac_din        : out std_logic;
    ndac_cs        : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data        : inout std_logic;
    
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    di              : out std_logic_vector(6 downto 0);
    ai              : in  std_logic_vector(1 downto 0);
    dout_LCD        : in  std_logic;
    wrdis           : out std_logic := '0';
    dres            : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    fpga_res        : in std_logic;
    nres            : in std_Logic;
    pbs2            : in std_logic;
    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
    ant             : inout std_logic_vector(26 downto 1) := (others => 'Z'); -- trigger bus
    
    p1              : inout std_logic := 'Z';
    p2              : in    std_logic := 'Z'; -- BlackCat1 Chn 1
    p3              : out   std_logic := 'Z'; -- BlackCat1 PG1 (active high)
    p4              : in    std_logic := 'Z'; -- BlackCat1 Chn 2
    p5              : out   std_logic := 'Z'; -- BlackCat1 Chn 3
    p6              : out   std_logic := 'Z'; -- BlackCat1 PG2 (active high)
    p7              : out   std_logic := 'Z'; -- BlackCat1 Chn 4
    p8              : inout std_logic := 'Z';
    p9              : inout std_logic := 'Z';
    p10             : inout std_logic := 'Z';
    p11             : inout std_logic := 'Z';
    p12             : inout std_logic := 'Z';
    p13             : inout std_logic := 'Z';
    p14             : inout std_logic := 'Z';
    p15             : inout std_logic := 'Z';
    p16             : inout std_logic := 'Z';
    p17             : out   std_logic := 'Z'; -- BlackCat1 TTLEN1 (active low)
    p18             : out   std_logic := 'Z'; -- BlackCat1 TTLEN2 (active low)
    p19             : out   std_logic := 'Z'; -- BlackCat1 TTLEN3 (active low)
    p21             : inout std_logic := 'Z';
    p22             : inout std_logic := 'Z';
    p23             : inout std_logic := 'Z';
    p24             : inout std_logic := 'Z';
    p25             : out   std_logic := 'Z'; -- BlackCat1 Chn 5
    p26             : out   std_logic := 'Z'; -- BlackCat1 Chn 6
    p27             : in    std_logic := 'Z'; -- BlackCat1 Chn 7
    p28             : inout std_logic := 'Z';
    p29             : inout std_logic := 'Z';
    p30             : inout std_logic := 'Z';
    n1              : inout std_logic := 'Z';
    n2              : inout std_logic := 'Z';
    n3              : inout std_logic := 'Z';
    n4              : inout std_logic := 'Z';
    n5              : inout std_logic := 'Z';
    n6              : inout std_logic := 'Z';
    n7              : inout std_logic := 'Z';
    n8              : inout std_logic := 'Z';
    n9              : inout std_logic := 'Z';
    n10             : inout std_logic := 'Z';
    n11             : inout std_logic := 'Z';
    n12             : inout std_logic := 'Z';
    n13             : inout std_logic := 'Z';
    n14             : inout std_logic := 'Z';
    n15             : inout std_logic := 'Z';
    n16             : inout std_logic := 'Z';
    n17             : inout std_logic := 'Z';
    n18             : inout std_logic := 'Z';
    n19             : inout std_logic := 'Z';
    n21             : inout std_logic := 'Z';
    n22             : inout std_logic := 'Z';
    n23             : inout std_logic := 'Z';
    n24             : inout std_logic := 'Z';
    n25             : inout std_logic := 'Z';
    n26             : inout std_logic := 'Z';
    n27             : inout std_logic := 'Z';
    n28             : inout std_logic := 'Z';
    n29             : inout std_logic := 'Z';
    n30             : inout std_logic := 'Z';
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con             : out std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd            : out   std_logic;
    slwr            : out   std_logic;
    fd              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    pa              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl             : in    std_logic_vector(2 downto 0);
    uclk            : in    std_logic;
    ures            : out   std_logic;
    
    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    led             : out std_logic_vector(8 downto 1) := (others => '1');
    
    -----------------------------------------------------------------------
    -- leds SFPs
    -----------------------------------------------------------------------
    ledsfpr          : out std_logic_vector(4 downto 1);
    ledsfpg          : out std_logic_vector(4 downto 1);

    sfp234_ref_clk_i    : in  std_logic;

    -----------------------------------------------------------------------
    -- SFP1  
    -----------------------------------------------------------------------
    
    sfp1_tx_disable_o : out std_logic := '0';
    sfp1_tx_fault     : in std_logic;
    sfp1_los          : in std_logic;
    
    --sfp1_txp_o        : out std_logic;
    --sfp1_rxp_i        : in  std_logic;
    
    sfp1_mod0         : in    std_logic; -- grounded by module
    sfp1_mod1         : inout std_logic; -- SCL
    sfp1_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP2
    -----------------------------------------------------------------------
    
    sfp2_tx_disable_o : out std_logic := '0';
    sfp2_tx_fault     : in  std_logic;
    sfp2_los          : in  std_logic;
    
    --sfp2_txp_o        : out std_logic;
    --sfp2_rxp_i        : in  std_logic;
    
    sfp2_mod0         : in    std_logic; -- grounded by module
    sfp2_mod1         : inout std_logic; -- SCL
    sfp2_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP3 
    -----------------------------------------------------------------------
       
    sfp3_tx_disable_o : out std_logic := '0';
    sfp3_tx_fault     : in std_logic;
    sfp3_los          : in std_logic;
    
    --sfp3_txp_o        : out std_logic;
    --sfp3_rxp_i        : in  std_logic;
    
    sfp3_mod0         : in    std_logic; -- grounded by module
    sfp3_mod1         : inout std_logic; -- SCL
    sfp3_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP4 
    -----------------------------------------------------------------------
    
    sfp4_tx_disable_o : out std_logic := '0';
    sfp4_tx_fault     : in std_logic;
    sfp4_los          : in std_logic;
    
    sfp4_txp_o        : out std_logic;
    sfp4_rxp_i        : in  std_logic;
    
    sfp4_mod0         : in    std_logic; -- grounded by module
    sfp4_mod1         : inout std_logic; -- SCL
    sfp4_mod2         : inout std_logic); -- SDA
end pci_control;

architecture rtl of pci_control is

  signal led_link_up  : std_logic;
  signal led_link_act : std_logic;
  signal led_track    : std_logic;
  signal led_pps      : std_logic;

begin

  main : monster
    generic map(
      g_family     => "Arria V",
      g_project    => "pci_control",
      g_inputs     => 2,
      g_outputs    => 6,
      g_flash_bits => 25,
      g_pll_skew   => 6500/(1000/8), -- 6500ps shift
      g_en_pcie    => true,
      g_en_usb     => true,
      g_en_lcd     => true)
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => sfp234_ref_clk_i, -- clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_clk_wr_ref_o      => p7,  -- BlackCat Chn 4
      core_clk_butis_o       => p5,  -- BlackCat Chn 3
      gpio_o(5 downto 2)     => led(8 downto 5),
      gpio_o(1)              => p26, -- BlackCat Chn 6
      gpio_o(0)              => p25, -- BlackCat Chn 5
      gpio_i(1)              => p4,  -- BlackCat Chn 2
      gpio_i(0)              => p27, -- BlackCat Chn 7
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
      wr_dac_sclk_o          => dac_sclk,
      wr_dac_din_o           => dac_din,
      wr_ndac_cs_o           => ndac_cs,
      -- no external clock input
      led_link_up_o          => led_link_up,
      led_link_act_o         => led_link_act,
      led_track_o            => led_track,
      led_pps_o              => led_pps,
      pcie_refclk_i          => pcie_refclk_i,
      pcie_rstn_i            => nPCI_RESET,
      pcie_rx_i              => pcie_rx_i,
      pcie_tx_o              => pcie_tx_o,
      usb_rstn_o             => ures,
      usb_ebcyc_i            => pa(3),
      usb_speed_i            => pa(0),
      usb_shift_i            => pa(1),
      usb_readyn_io          => pa(7),
      usb_fifoadr_o          => pa(5 downto 4),
      usb_sloen_o            => pa(2),
      usb_fulln_i            => ctl(1),
      usb_emptyn_i           => ctl(2),
      usb_slrdn_o            => slrd,
      usb_slwrn_o            => slwr,
      usb_pktendn_o          => pa(6),
      usb_fd_io              => fd,
      lcd_scp_o              => di(3),
      lcd_lp_o               => di(1),
      lcd_flm_o              => di(2),
      lcd_in_o               => di(0));

  sfp1_tx_disable_o <= '1';
  sfp2_tx_disable_o <= '1';
  sfp3_tx_disable_o <= '1';
  sfp4_tx_disable_o <= '0';

  wrdis <= '0';
  dres  <= '1';
  di(5) <= '0' when (not led_link_up)                   = '1' else 'Z'; -- red
  di(6) <= '0' when (    led_link_up and not led_track) = '1' else 'Z'; -- blue
  di(4) <= '0' when (    led_link_up and     led_track) = '1' else 'Z'; -- green

  -- LEDs
  led(1) <= not (led_link_act and led_link_up); -- red   = traffic/no-link
  led(2) <= not led_link_up;                    -- blue  = link
  led(3) <= not led_track;                      -- green = timing valid
  led(4) <= not led_pps;                        -- white = PPS
  
  ledsfpg(3 downto 1) <= (others => '1');
  ledsfpr(3 downto 1) <= (others => '1');
  ledsfpg(4) <= not led_link_up;
  ledsfpr(4) <= not led_link_act;
      
  p3  <= '1'; -- BlackCat1 PG1 (active high)
  p6  <= '1'; -- BlackCat1 PG2 (active high)
  p17 <= '0'; -- BlackCat1 TTLEN1 (active low)
  p18 <= '0'; -- BlackCat1 TTLEN2 (active low)
  p19 <= '0'; -- BlackCat1 TTLEN3 (active low)
  
  -- Wires to CPLD, currently unused
  con <= (others => 'Z');
  
end rtl;
