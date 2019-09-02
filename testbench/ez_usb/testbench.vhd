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


  procedure write_word(constant data : in std_logic_vector(31 downto 0); 
                        signal slrd_i : in  std_logic; 
                        signal fd_o : out std_logic_vector(7 downto 0)) is
  begin
    fd_o   <= data(31 downto 24);
    wait until falling_edge(slrd_i);  wait until rising_edge(slrd_i);
    fd_o   <= data(23 downto 16);
    wait until falling_edge(slrd_i);  wait until rising_edge(slrd_i);
    fd_o   <= data(15 downto 8);
    wait until falling_edge(slrd_i);  wait until rising_edge(slrd_i);
    fd_o   <= data(7 downto 0);
    wait until falling_edge(slrd_i);  wait until rising_edge(slrd_i);
    fd_o <= (others => 'Z');
 end procedure;

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
    type char_file_t is file of character;
    file char_file_read : char_file_t;
    file char_file_write : char_file_t;
    variable char_in, char_out : character;
    subtype byte_t is natural range 0 to 255;
    variable byte_v : byte_t;
    variable counter : integer := 0;
  begin

    
    usb_readyn_io <= '1';
    wait until rising_edge(rst_n);
    wait until rising_edge(usb_rstn_o);

    for i in 0 to 50 loop wait until rising_edge(clk_sys); end loop;    

    -- I'm the EZ-USB chip. My cpu booted, I'll tell the world that I'm ready
    usb_readyn_io <= '0';
    usb_fulln_i <= '1'; -- I'm never full (I'm a never-ending fifo)
    wait until rising_edge(clk_sys);



    -- I'm the EZ-USB chip, nobody knows where my data actually comes from...
    -- ...  I'll read it from a file in this case (not from the D+/D- wires)
    file_open(char_file_read, DEVICE_READ, read_mode); 
    file_open(char_file_write, DEVICE_WRITE, write_mode); 
    -- I'm the EZ-USB chip. I received data and de-assert my emty line
    --  and set the cycle line 
    usb_ebcyc_i <= '1';
    wait until rising_edge(clk_sys);
    while not endfile(char_file_read) loop

      usb_emptyn_i <= '1'; -- not empty when high:
      for i in 1 to 8 loop
        read(char_file_read, char_in);
        byte_v := character'pos(char_in);
        report "read char: " & integer'image(byte_v);
        usb_fd_io   <= std_logic_vector(to_unsigned(byte_v, 8));
        wait until falling_edge(usb_slrdn_o);  wait until rising_edge(usb_slrdn_o);
      end loop;
      usb_fd_io <= (others => 'Z');
      usb_emptyn_i <= '0'; -- empty when low:
    
    
      for i in 1 to 8 loop
        wait until rising_edge(usb_slwrn_o);
        char_out := character'val(to_integer(unsigned(usb_fd_io)));
        write(char_file_write, char_out);
        byte_v := character'pos(char_out);
        report "wrote char: " & integer'image(byte_v); 
      end loop;
      while usb_pktendn_o = '1' loop
        wait until rising_edge(clk_sys);
      end loop;

      --  --for i in 0 to 50 loop wait until rising_edge(clk_sys); end loop;    
      file_close(char_file_write);
      file_open(char_file_write, DEVICE_WRITE, write_mode); 
      --  --for i in 0 to 50 loop wait until rising_edge(clk_sys); end loop;    
    end loop;


    --usb_ebcyc_i <= '1';
    --wait until rising_edge(clk_sys);
    --usb_emptyn_i <= '1'; -- not empty when high:
    --while not endfile(char_file) loop
    ----for i in 1 to 8 loop
    --  read(char_file, char_v);
    --  byte_v := character'pos(char_v); 
    --  report "2-read char: " & integer'image(byte_v);
    --  usb_fd_io   <= std_logic_vector(to_unsigned(byte_v, 8));
    --  wait until falling_edge(usb_slrdn_o);  wait until rising_edge(usb_slrdn_o);
    --end loop;
    --usb_ebcyc_i <= '0';
    --usb_emptyn_i <= '0'; -- empty when low:

    --file_close(char_file);
    --file_open(char_file, DEVICE, write_mode);
    --write(char_file, char_v);
    --file_close(char_file);

    ---- I'm the EZ-USB chip and I got some data from the host. I'll de-assert my empty line
    --                               write these words  =>    ********
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0x4e6f11ff
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0x00000086
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0xe80f0101
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0x04060000
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0xB00BBABE
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0x00008001
--pcimem /sys/bus/pci/devices/0000:02:00.0/resource2 0xA0 w 0x00000004

 
    --usb_ebcyc_i <= '1';
    --wait until rising_edge(clk_sys);
    --usb_emptyn_i <= '1'; -- not empty when high: send data 4e6f11ff
    --write_word(x"4e6f11ff", usb_slrdn_o, usb_fd_io); -- eb-header (magic, etc.)
    --usb_emptyn_i <= '0'; -- empty when low
    --for i in 0 to 521 loop wait until rising_edge(clk_sys); end loop;    
    --usb_emptyn_i <= '1'; -- 
    --write_word(x"00000086", usb_slrdn_o, usb_fd_io);
    --usb_emptyn_i <= '0'; -- empty when low
    --for i in 0 to 521 loop wait until rising_edge(clk_sys); end loop;    
    --usb_emptyn_i <= '1'; -- 
    --write_word(x"280f0202", usb_slrdn_o, usb_fd_io); -- record header
    --write_word(x"04060000", usb_slrdn_o, usb_fd_io);
    --write_word(x"B00BBABE", usb_slrdn_o, usb_fd_io);
    --write_word(x"CAFE1234", usb_slrdn_o, usb_fd_io);
    --write_word(x"04060000", usb_slrdn_o, usb_fd_io);
    --write_word(x"00000004", usb_slrdn_o, usb_fd_io);
    --write_word(x"00000004", usb_slrdn_o, usb_fd_io);
    --wait until rising_edge(clk_sys);
    --usb_emptyn_i <= '0'; -- empty when low
    --for i in 0 to 521 loop wait until rising_edge(clk_sys); end loop;    
    --usb_ebcyc_i <= '0';


    while true loop 
      wait until rising_edge(clk_sys);
    end loop;


    file_close(char_file_write);
    file_close(char_file_read);

  end process;


end architecture;



