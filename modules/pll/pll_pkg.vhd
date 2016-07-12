library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package pll_pkg is

  subtype phase_offset is unsigned(9 downto 0);
  type phase_offset_vector is array(natural range <>) of phase_offset;
  type natural_vector is array(natural range <>) of natural;

  component altera_reset is
    generic(
      g_plls    : natural := 4;
      g_clocks  : natural := 2;
      g_areset  : natural := 1024;  -- length of pll_arst_o
      g_stable  : natural := 1024); -- duration locked must be stable
    port(
      clk_free_i : in  std_logic; -- external free running clock
      rstn_i     : in  std_logic; -- external reset button
      pll_lock_i : in  std_logic_vector(g_plls-1 downto 0);
      pll_arst_o : out std_logic;
      clocks_i   : in  std_logic_vector(g_clocks-1 downto 0);
      rstn_o     : out std_logic_vector(g_clocks-1 downto 0));
  end component;
  
  component dmtd_pll is  -- arria2
    port(
      areset : in  std_logic;
      inclk0 : in  std_logic := '0'; -- 20   MHz
      c0     : out std_logic;        -- 62.5 MHz
      locked : out std_logic);
  end component;

  component dmtd_pll5 is -- arria5
    port(
      refclk   : in  std_logic := 'X'; -- 20   MHz
      outclk_0 : out std_logic;        -- 62.5 MHz
      rst      : in  std_logic := 'X';
      locked   : out std_logic);
  end component;
  
  component ref_pll is   -- arria2
    port(
      areset : in  std_logic;
      inclk0 : in  std_logic := '0'; -- 125 MHz
      c0     : out std_logic;        -- 125 MHz
      c1     : out std_logic;        -- 200 MHz
      c2     : out std_logic;        --  25 MHz
      locked : out std_logic;
      scanclk            : in  std_logic;
      phasecounterselect : in  std_logic_vector(3 downto 0);
      phasestep          : in  std_logic;
      phaseupdown        : in  std_logic;
      phasedone          : out std_logic);
  end component;

  component ref_pll5 is  -- arria5
    port(
      refclk     : in  std_logic := 'X'; -- 125 MHz
      outclk_0   : out std_logic;        -- 125 MHz
      outclk_1   : out std_logic;        -- 200 MHz
      outclk_2   : out std_logic;        --  25 MHz
      outclk_3   : out std_logic;        --1000 MHz
      outclk_4   : out std_logic;        -- 125 MHz, 1/8 duty cycle, -1.5ns phase
      rst        : in  std_logic := 'X';
      locked     : out std_logic;
      scanclk    : in  std_logic;
      cntsel     : in  std_logic_vector(4 downto 0);
      phase_en   : in  std_logic;
      updn       : in  std_logic;
      phase_done : out std_logic);
  end component;
  
  component sys_pll is   -- arria2
    port(
      areset : in  std_logic;
      inclk0 : in  std_logic := '0'; -- 125   MHz
      c0     : out std_logic;        --  62.5 MHz
      c1     : out std_logic;        --  50   MHz (flash+reconfig)
      c2     : out std_logic;        --  20   MHz (display+scubus)
      c3     : out std_logic;        --  10   MHz (remote update)
      locked : out std_logic);
  end component;

  component sys_pll5 is  -- arria5
    port(
      refclk   : in  std_logic := 'X'; -- 125   MHz
      outclk_0 : out std_logic;        --  62.5 MHz
      outclk_1 : out std_logic;        -- 100   MHz (flash+reconfig)
      outclk_2 : out std_logic;        --  20   MHz (display+scubus)
      outclk_3 : out std_logic;        --  10   MHz (remote update)
      outclk_4 : out std_logic;        --  50   MHz
      rst      : in  std_logic := 'X';
      locked   : out std_logic);
  end component;

  component altera_phase is
    generic(
      g_select_bits   : natural;
      g_outputs       : natural;
      g_base          : integer; -- base phase shift relative to input
      g_vco_freq      : natural;
      g_output_freq   : natural_vector;
      g_output_select : natural_vector);
    port(
      clk_i       : in  std_logic; 
      rstn_i      : in  std_logic; -- phase counters were zero'd
      clks_i      : in  std_logic_vector(g_outputs-1 downto 0);
      rstn_o      : out std_logic_vector(g_outputs-1 downto 0);
      offset_i    : in  phase_offset_vector(g_outputs-1 downto 0);
      phasedone_i : in  std_logic;
      phasesel_o  : out std_logic_vector(g_select_bits-1 downto 0);
      phasestep_o : out std_logic);
  end component;
  
  component altera_butis is
    port(
      clk_ref_i : in  std_logic;
      clk_25m_i : in  std_logic;
      pps_i     : in  std_logic;
      phase_o   : out phase_offset);
  end component;
  
end pll_pkg;
