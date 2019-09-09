library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;
use work.ez_usb_pkg.all;
use work.file_access.all; -- a library implemented in C


-- use with socat pseudo terminals:
--   socat -d -d pty,raw,echo=0 pty,raw,echo=0  # creates /dev/pts/40 and /dev/pts/39
--   socat -u -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/42
--   socat -U -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/44
-- then start simulation and call:
--   eb-read -p dev/pts/39 0x01000000/4
entity testbench is
generic (
    DEVICE_READ : string := "/dev/pts/42" ;
    DEVICE_WRITE : string := "/dev/pts/44"
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

  signal uart_usb      : std_logic := '0';
  signal uart_wrc      : std_logic := '0';
  signal usb_rstn_o    : std_logic := '0';
  signal usb_ebcyc_i   : std_logic := '0';
  signal usb_speed_i   : std_logic := '0';
  signal usb_shift_i   : std_logic := '0';
  signal usb_readyn_io : std_logic := '0';
  signal usb_fifoadr_o : std_logic_vector(1 downto 0) := (others => '0');
  signal usb_fulln_i   : std_logic := '0';
  signal usb_sloen_o   : std_logic := '0';
  signal usb_emptyn_i  : std_logic := '0';
  signal usb_slrdn_o   : std_logic := '0';
  signal usb_slwrn_o   : std_logic := '0';
  signal usb_pktendn_o : std_logic := '0';
  signal usb_fd_io     : std_logic_vector(7 downto 0) := (others => '0');
  signal s_usb_fd_o    : std_logic_vector(7 downto 0) := (others => '0');
  signal s_usb_fd_oen  : std_logic := '0';

  signal counter : integer := 0;
  --signal in_value : integer := 0;
  --signal out_value : integer := 0;

begin


  ---- generate clock and reset signal -------
  clk_50  <= not clk_50  after clk_50_period/2;
  clk_125 <= not clk_125 after clk_125_period/2;
  clk_sys <= not clk_sys after clk_sys_period/2;
  rst     <= '0'         after clk_50_period*20;
  rst_n   <= not rst;
  rstn_sys<= not rst;
  --------------------------------------------


  ---- instance of ez_usb component
  --usb_readyn_io <= 'Z';
  usb_fd_io <= s_usb_fd_o when s_usb_fd_oen='1' else (others => 'Z');
  usb : ez_usb
    generic map(
      g_sdb_address => x"10000000",
      g_sys_freq => 10 -- this tells the component our frequency is only 10kHz. 
                        -- reason: so it doesnt't wait too many clock tics until 
                        --  it releases the ez_usb chip from its reset
                        )
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => wb_miso,--top_bus_slave_o(c_topm_usb),
      master_o  => wb_mosi,--top_bus_slave_i(c_topm_usb),
      uart_o    => uart_usb,
      uart_i    => uart_wrc,
      rstn_o    => usb_rstn_o,
      ebcyc_i   => usb_ebcyc_i,
      speed_i   => usb_speed_i,
      shift_i   => usb_shift_i,
      readyn_i  => usb_readyn_io,
      fifoadr_o => usb_fifoadr_o,
      fulln_i   => usb_fulln_i,
      sloen_o   => usb_sloen_o,
      emptyn_i  => usb_emptyn_i,
      slrdn_o   => usb_slrdn_o,
      slwrn_o   => usb_slwrn_o,
      pktendn_o => usb_pktendn_o,
      fd_i      => usb_fd_io,
      fd_o      => s_usb_fd_o,
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

  USB_chip: process 
    variable in_value : integer := 0;
    variable out_value : integer := 0;
  begin

    
    usb_readyn_io <= '1';
    wait until rising_edge(rst_n);
    wait until rising_edge(usb_rstn_o);

    for i in 0 to 50 loop wait until rising_edge(clk_sys); end loop;    

    -- I'm the EZ-USB chip. My cpu booted, I'll tell the world that I'm ready
    usb_readyn_io <= '0';
    usb_fulln_i <= '1'; -- I'm never full (I'm a never-ending fifo)
    wait until rising_edge(clk_sys);



    -- I'm the EZ-USB chip, nobody knows where my data really comes from...
    -- ...  I'll read it from a file in this case (not from the D+/D- wires)
    --file_open(char_file_read, DEVICE_READ, read_mode); 
    --file_open(char_file_write, DEVICE_WRITE, write_mode); 
    file_access_init;

    usb_ebcyc_i <= '1';
    while true loop    

      --wait until rising_edge(clk_sys);

      in_value := file_access_read;
      say_hello(in_value);
      while in_value >= 0 loop
        usb_emptyn_i <= '1'; -- we have data -> de-assert empty line
        usb_fd_io <= std_logic_vector(to_signed(in_value, 8));
        wait until falling_edge(usb_slrdn_o);  wait until rising_edge(usb_slrdn_o);
        in_value := file_access_read;
      end loop;

      --wait until rising_edge(clk_sys);
      usb_fd_io <= (others => 'Z');
      usb_emptyn_i <= '0'; -- we are empty
      wait until rising_edge(clk_sys);

      if usb_slwrn_o = '0' then 
        while usb_pktendn_o = '1' loop
          wait until rising_edge(usb_slwrn_o) or falling_edge(usb_pktendn_o);
          if usb_pktendn_o = '1' then
            out_value := to_integer(unsigned(usb_fd_io));
            file_access_write(out_value);
          end if;
        end loop;
      end if;

    end loop;
    usb_ebcyc_i <= '1';


    --file_close(char_file_write);
    --file_close(char_file_read);

  end process;


end architecture;



