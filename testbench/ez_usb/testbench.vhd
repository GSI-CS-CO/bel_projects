library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;
use work.ez_usb_pkg.all;


-- use with socat pseudo terminals:
--   socat -d -d pty,raw,echo=0 pty,raw,echo=0  # creates /dev/pts/40 and /dev/pts/39
--   socat -u -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/42
--   socat -U -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/44
-- then start simulation and call:
--   eb-read -p dev/pts/39 0x01000000/4
entity testbench is
generic (
    PTS_NUMBER : integer := 30
  );
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';
  signal rstn_sys         : std_logic := '0';
  constant clk_50_period  : time      := 20 ns;
  constant clk_125_period : time      :=  8 ns;
  constant clk_sys_period : time      := 16 ns;
  signal clk_50           : std_logic := '1';
  signal clk_125          : std_logic := '1';
  signal clk_sys          : std_logic := '1';


  -- wb signals
  signal wb_mosi : t_wishbone_master_out;
  signal wb_miso : t_wishbone_master_in;

  signal uart_usb     : std_logic := '0';
  signal uart_wrc     : std_logic := '0';
  signal usb_rstn     : std_logic := '0';
  signal usb_ebcyc    : std_logic := '0';
  signal usb_readyn   : std_logic := '0';
  signal usb_fifoadr  : std_logic_vector(1 downto 0) := (others => '0');
  signal usb_fulln    : std_logic := '0';
  signal usb_sloen    : std_logic := '0';
  signal usb_emptyn   : std_logic := '0';
  signal usb_slrdn    : std_logic := '0';
  signal usb_slwrn    : std_logic := '0';
  signal usb_pktendn  : std_logic := '0';
  signal usb_fd_io    : std_logic_vector(7 downto 0) := (others => 'Z');
  signal s_usb_fd     : std_logic_vector(7 downto 0) := (others => '0');
  signal s_usb_fd_oen : std_logic := '0';

  signal counter : integer := 0;

begin


  ---- generate clock and reset signal -------
  clk_50  <= not clk_50  after clk_50_period/2;
  clk_125 <= not clk_125 after clk_125_period/2;
  clk_sys <= not clk_sys after clk_sys_period/2;
  rst     <= '0'         after clk_50_period*20;
  rst_n   <= not rst;
  rstn_sys<= not rst;
  --------------------------------------------

  ---- instance of EZUSB-chip 
  chip : entity work.ez_usb_chip
    port map (
      rstn_i    => usb_rstn,
      ebcyc_o   => usb_ebcyc,
      readyn_o  => usb_readyn,
      fifoadr_i => usb_fifoadr,
      fulln_o   => usb_fulln,
      emptyn_o  => usb_emptyn,
      sloen_i   => usb_sloen,
      slrdn_i   => usb_slrdn,
      slwrn_i   => usb_slwrn,
      pktendn_i => usb_pktendn,
      fd_io     => usb_fd_io
      );


  ---- instance of ez_usb component
  --usb_readyn <= 'Z';
  usb_fd_io <= s_usb_fd when s_usb_fd_oen='1' else (others => 'Z');
  usb : ez_usb
    generic map(
      g_sdb_address => x"10000000",
      g_sys_freq => 10 -- this tells the component our frequency is only 10kHz. 
                        -- reason: so it doesn't wait too many clock tics until 
                        --  it releases the ez_usb chip from its reset
                        )
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => wb_miso,--top_bus_slave_o(c_topm_usb),
      master_o  => wb_mosi,--top_bus_slave_i(c_topm_usb),
      uart_o    => uart_usb,
      uart_i    => uart_wrc,
      rstn_o    => usb_rstn,
      ebcyc_i   => usb_ebcyc,
      speed_i   => '0',
      shift_i   => '0',
      readyn_i  => usb_readyn,
      fifoadr_o => usb_fifoadr,
      fulln_i   => usb_fulln,
      sloen_o   => usb_sloen,
      emptyn_i  => usb_emptyn,
      slrdn_o   => usb_slrdn,
      slwrn_o   => usb_slwrn,
      pktendn_o => usb_pktendn,
      fd_i      => usb_fd_io,
      fd_o      => s_usb_fd,
      fd_oen_o  => s_usb_fd_oen); 

  --wr_uart_o <= uart_wrc;
  --uart_mux <= uart_usb and wr_uart_i;


  minislave : entity work.wb_minislave
  port map (
    clk_i   => clk_sys,
    rst_n_i => rst_n,
    slave_i => wb_mosi,
    slave_o => wb_miso
  );



end architecture;



