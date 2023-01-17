-- Synopsys
-- Small testbench to take this i2c master into operation and observe Wishbone wrapper handling
--
-- Prescaler:
-- Value = (wb_clk_frequency)/(5*desired_frequency)
-- Reset = 0xffff

-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;
use work.wb_i2c_wrapper_pkg.all;

-- Entity (empty)
entity i2c_testbench is
  generic (g_interfaces : integer := 5);
  port (last_byte  : out   std_logic_vector(7 downto 0);
        slave_ack  : out   std_logic_vector(g_interfaces-1 downto 0);
        scl_pad_io : inout std_logic_vector(g_interfaces-1 downto 0);
        sda_pad_io : inout std_logic_vector(g_interfaces-1 downto 0));
end;

-- Architecture
architecture rtl of i2c_testbench is

  -- Testbench settings
  constant c_interfaces  : integer := g_interfaces;
  constant c_reset_time  : time    := 200 ns;
  constant c_clock_cycle : time    := 16 ns;
  constant c_gen_xwb     : boolean := false;
  constant c_loop_in_out : boolean := true;

 -- I2C master registers
  constant c_reg_prer_lo : std_logic_vector(31 downto 0) := x"00000000"; -- RW clock prescale low-byte
  constant c_reg_prer_hi : std_logic_vector(31 downto 0) := x"00000001"; -- RW clock prescale high-byte
  constant c_reg_ctr     : std_logic_vector(31 downto 0) := x"00000002"; -- RW control register
  constant c_reg_txr     : std_logic_vector(31 downto 0) := x"00000003"; -- #W transmit register
  constant c_reg_rxr     : std_logic_vector(31 downto 0) := x"00000003"; -- R# receive register
  constant c_reg_cr      : std_logic_vector(31 downto 0) := x"00000004"; -- #W command register
  constant c_reg_sr      : std_logic_vector(31 downto 0) := x"00000004"; -- R# status register

  -- I2c master register bits
  constant c_reg_ctr_core_enable_bit            : std_logic_vector(31 downto 0) := x"00000080";
  constant c_reg_ctr_interrupt_enable_bit       : std_logic_vector(31 downto 0) := x"00000040";
  constant c_reg_txr_next_byte_to_transmit_mask : std_logic_vector(31 downto 0) := x"000000fe";
  constant c_reg_txr_reading_from_slave_bit     : std_logic_vector(31 downto 0) := x"00000001";
  constant c_reg_txr_writing_fo_slave_bit       : std_logic_vector(31 downto 0) := x"00000000";
  constant c_reg_rxr_last_byte_received_mask    : std_logic_vector(31 downto 0) := x"000000ff";
  constant c_reg_cr_sta                         : std_logic_vector(31 downto 0) := x"00000080";
  constant c_reg_cr_sto                         : std_logic_vector(31 downto 0) := x"00000040";
  constant c_reg_cr_rd                          : std_logic_vector(31 downto 0) := x"00000020";
  constant c_reg_cr_wr                          : std_logic_vector(31 downto 0) := x"00000010";
  constant c_reg_cr_ack                         : std_logic_vector(31 downto 0) := x"00000008";
  constant c_reg_cr_iack                        : std_logic_vector(31 downto 0) := x"00000001";
  constant c_reg_sr_rack                        : std_logic_vector(31 downto 0) := x"00000080";
  constant c_reg_sr_bus_busy                    : std_logic_vector(31 downto 0) := x"00000040";
  constant c_reg_sr_al                          : std_logic_vector(31 downto 0) := x"00000020";
  constant c_reg_sr_tip                         : std_logic_vector(31 downto 0) := x"00000002";
  constant c_reg_sr_if                          : std_logic_vector(31 downto 0) := x"00000001";

  -- Other constants
  constant c_reg_all_zero                : std_logic_vector(31 downto 0) := x"00000000";
  constant c_cyc_on                      : std_logic := '1';
  constant c_cyc_off                     : std_logic := '0';
  constant c_str_on                      : std_logic := '1';
  constant c_str_off                     : std_logic := '0';
  constant c_we_on                       : std_logic := '1';
  constant c_we_off                      : std_logic := '0';
  constant c_i2c_write                   : std_logic_vector(31 downto 0) := x"00000000";
  constant c_i2c_read                    : std_logic_vector(31 downto 0) := x"00000001";
  constant c_prescaler_low               : std_logic_vector(31 downto 0) := x"000000ff";
  constant c_prescaler_high              : std_logic_vector(31 downto 0) := x"00000002";

  -- Basic device signals
  signal s_clk   : std_logic := '0';
  signal s_rst_n : std_logic := '0';
  signal s_rst   : std_logic := '0';

  -- Wishbone connections
  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;
  signal s_wb_desc_out  : t_wishbone_device_descriptor;

  -- Interrupt
  signal s_int : std_logic;

  -- I2C pads
  signal s_scl_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_padoen_out : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_padoen_out : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_pad_io     : std_logic_vector(g_interfaces-1 downto 0);
  signal s_sda_pad_io     : std_logic_vector(g_interfaces-1 downto 0);

  -- Testbench logic
  signal s_sequence_cnt   : std_logic_vector(31 downto 0);
  signal s_ack            : std_logic;

  -- I2C slave
  signal s_scl_latch      : std_logic;
  signal s_sda_latch      : std_logic;
  signal s_received_bit   : std_logic;
  signal s_received_byte  : std_logic_vector(7 downto 0);
  signal s_slave_sda_out  : std_logic_vector(c_interfaces-1 downto 0);
  signal s_slave_idle     : std_logic;

  -- Functions
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic;
                   adr : t_wishbone_address; dat : t_wishbone_data) return t_wishbone_slave_in is
  variable v_setup : t_wishbone_slave_in;
  begin
    v_setup.cyc := cyc;
    v_setup.stb := stb;
    v_setup.we  := we;
    v_setup.adr := adr;
    v_setup.dat := dat;
    v_setup.sel := (others => '0'); -- Don't care
    return v_setup;
  end function wb_stim;

  -- Procedures
  -- Procedure wb_expect -> Check WB slave answer
  procedure wb_expect(msg : string; dat_from_slave : t_wishbone_data; compare_value : t_wishbone_data) is
  begin
    if (to_integer(unsigned(dat_from_slave)) = to_integer(unsigned(compare_value))) then
      report "Test passed: " & msg;
    else
      report "Test errored: " & msg;
      report "-> Info:  Answer from slave:          " & integer'image(to_integer(unsigned(dat_from_slave)));
      report "-> Error: Expected answer from slave: " & integer'image(to_integer(unsigned(compare_value)));
    end if;
  end procedure wb_expect;

    type slave_states is (idle, start, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8, stop, err);
    signal s_slave_state : slave_states := idle;

begin

  -- Interfaces
  interfaces : for i in 0 to (c_interfaces-1) generate
    -- Control IO pad
    s_scl_pad_io(i) <= s_scl_pad_out(i) when (s_scl_padoen_out(i) = '0') else '1';
    s_sda_pad_io(i) <= s_sda_pad_out(i) when (s_sda_padoen_out(i) = '0') else s_slave_sda_out(i);
    scl_pad_io(i)   <= s_scl_pad_io(i);
    sda_pad_io(i)   <= s_sda_pad_io(i);
    -- Handle inputs
    loop_in_out_yes: if c_loop_in_out generate
      s_scl_pad_in(i) <= scl_pad_io(i);
      s_sda_pad_in(i) <= sda_pad_io(i);
    end generate;
    loop_in_out_no: if not(c_loop_in_out) generate
      s_scl_pad_in(i) <= 'Z';
      s_sda_pad_in(i) <= 'Z';
    end generate;
  end generate;

  -- Clock generator
  p_clock : process
  begin
    s_clk <= '0';
    wait for c_clock_cycle/2;
    s_clk <= '1' and s_rst_n;
    wait for c_clock_cycle/2;
  end process;

  -- Reset controller
  p_reset : process
  begin
    wait for c_reset_time;
    s_rst_n <= '1';
  end process;
  s_rst <= not(s_rst_n);

  -- I2C master (XWB Wrapper)
  xwb_wrapper_yes : if c_gen_xwb generate
    u_i2c_dut : xwb_i2c_master
      generic map (
        g_interface_mode      => CLASSIC,
        g_address_granularity => WORD,
        g_num_interfaces      => c_interfaces)
      port map (
        clk_sys_i    => s_clk,
        rst_n_i      => s_rst_n,
        slave_i      => s_wb_slave_in,
        slave_o      => s_wb_slave_out,
        desc_o       => s_wb_desc_out,
        int_o        => s_int,
        scl_pad_i    => s_scl_pad_in,
        scl_pad_o    => s_scl_pad_out,
        scl_padoen_o => s_scl_padoen_out,
        sda_pad_i    => s_sda_pad_in,
        sda_pad_o    => s_sda_pad_out,
        sda_padoen_o => s_sda_padoen_out);
  end generate;

  -- I2C master (without wrapper)
  xwb_wrapper_no : if not(c_gen_xwb) generate
    u_i2c_dut : i2c_master_top
      generic map (
        ARST_LVL         => '0',
        g_num_interfaces => c_interfaces)
      port map (
        wb_clk_i     => s_clk,
        wb_rst_i     => s_rst,
        arst_i       => '1',
        wb_adr_i     => s_wb_slave_in.adr(2 downto 0),
        wb_dat_i     => s_wb_slave_in.dat(7 downto 0),
        wb_dat_o     => s_wb_slave_out.dat(7 downto 0),
        wb_we_i      => s_wb_slave_in.we,
        wb_stb_i     => s_wb_slave_in.stb,
        wb_cyc_i     => s_wb_slave_in.cyc,
        wb_ack_o     => s_wb_slave_out.ack,
        inta_o       => s_int,
        scl_pad_i    => s_scl_pad_in,
        scl_pad_o    => s_scl_pad_out,
        scl_padoen_o => s_scl_padoen_out,
        sda_pad_i    => s_sda_pad_in,
        sda_pad_o    => s_sda_pad_out,
        sda_padoen_o => s_sda_padoen_out);

      s_wb_slave_out.dat(31 downto 8) <= (others => '0');
  end generate;

  -- Short ack
  s_ack <= s_wb_slave_out.ack;

  -- I2C slave at postion 0
  p_slave : process (s_clk, s_rst_n) is
  begin
    if s_rst_n = '0' then
      s_slave_state      <= idle;
      s_scl_latch        <= '0';
      s_sda_latch        <= '0';
      s_received_byte    <= (others => '0');
      s_slave_sda_out(0) <= '1';
      s_slave_idle       <= '0';
    elsif rising_edge(s_clk) then
      s_scl_latch <= s_scl_pad_in(0);
      s_sda_latch <= s_sda_pad_in(0);

      case (s_slave_state) is
        when idle =>
          s_slave_idle  <= '0';
          if ((s_scl_pad_in(0) = '1') and (s_sda_pad_in(0) = '0') and s_sda_latch = '1') then
            s_slave_state <= start;
            report "Slave: Start condition detected!";
          end if;

        when start => -- ToDo: Implement a proper counter and merge states "bit1" to "bit8" into one state
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit1;
            s_received_byte(7) <= s_received_bit;
            report "Slave: Bit1 detected: " & std_logic'image(s_received_bit);
          end if;

        when bit1 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit2;
            s_received_byte(6) <= s_received_bit;
            report "Slave: Bit2 detected: " & std_logic'image(s_received_bit);
          end if;

        when bit2 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit3;
            s_received_byte(5) <= s_received_bit;
            report "Slave: Bit3 detected: " & std_logic'image(s_received_bit);
          end if;

          when bit3 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit4;
            s_received_byte(4) <= s_received_bit;
            report "Slave: Bit4 detected: " & std_logic'image(s_received_bit);
          end if;

          when bit4 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit5;
            s_received_byte(3) <= s_received_bit;
            report "Slave: Bit5 detected: " & std_logic'image(s_received_bit);
          end if;

          when bit5 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit6;
            s_received_byte(2) <= s_received_bit;
            report "Slave: Bit6 detected: " & std_logic'image(s_received_bit);
          end if;

          when bit6 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit7;
            s_received_byte(1) <= s_received_bit;
            report "Slave: Bit7 detected: " & std_logic'image(s_received_bit);
          end if;

          when bit7 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= bit8;
            s_received_byte(0) <= s_received_bit;
            report "Slave: Bit8 detected: " & std_logic'image(s_received_bit);
            if s_received_bit = '0' then
              report "Slave: Write command detected!";
            else
              report "Slave: Read command detected!";
            end if;
            s_slave_sda_out(0) <= '0';
          end if;

          when bit8 =>
          if ((s_scl_pad_in(0) = '1') and (s_scl_latch = '0')) then
            s_slave_state      <= idle;
            s_slave_idle       <= '1';
            report "Slave: Byte received (dec): " & integer'image(to_integer(unsigned(s_received_byte)));
            s_slave_sda_out(0) <= '1';
          end if;

        when others =>

          null;
      end case;
    end if;
  end process;
  s_received_bit <= s_sda_pad_in(0);
  last_byte      <= s_received_byte;
  slave_ack      <= s_slave_sda_out;

  -- Empty slave ports
  s_slave_sda_out(c_interfaces-1 downto 1) <= (others =>'1');

  -- Testbench controller
  p_tb_ctl : process(s_clk, s_rst_n) is
  begin
    if s_rst_n = '0' then
      s_sequence_cnt <= (others => '0');
    elsif rising_edge(s_clk) then
      s_sequence_cnt <= std_logic_vector(unsigned(s_sequence_cnt) + 1);
    end if;
  end process;

  -- Wishbone controller
  p_wishbone_stim : process
  begin
    -- Reset
    s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    wait until rising_edge(s_rst_n);
    wait until rising_edge(s_clk);
    -- Check if core is disabled (read configuration)
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk); wb_expect("Core disabled?", s_wb_slave_out.dat, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Set up prescaler to 100kHz [hi=00 lo=d7]
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_prer_lo,  c_prescaler_low);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_prer_lo,  c_prescaler_low);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_prer_hi,  c_prescaler_high);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_prer_hi,  c_prescaler_high);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Read back prescaler settings
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_prer_lo,  c_prescaler_low);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_prer_lo,  c_prescaler_low);
    wait until rising_edge(s_ack); wb_expect("Prescaler low set?", s_wb_slave_out.dat, c_prescaler_low);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_prer_hi,  c_prescaler_high);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_prer_hi,  c_prescaler_high);
    wait until rising_edge(s_ack); wb_expect("Prescaler high set?", s_wb_slave_out.dat, c_prescaler_high);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Enable core and interrupts
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_ctr, c_reg_ctr_core_enable_bit or c_reg_ctr_interrupt_enable_bit);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_ctr, c_reg_ctr_core_enable_bit or c_reg_ctr_interrupt_enable_bit);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Check if core and interrupts are enabled now
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_ack); wb_expect("Core enabled?", s_wb_slave_out.dat, c_reg_ctr_core_enable_bit or c_reg_ctr_interrupt_enable_bit);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Idle
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    wait until rising_edge(s_clk);
    -- Address and write bit (programming example page => slave address = 0x51<<1 = 0xa2)
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_txr, x"000000a2");
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_txr, x"000000a2");
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_txr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Set start and write bits
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sta or c_reg_cr_wr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sta or c_reg_cr_wr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_cr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Wait until Slave received address (and r/w bit)
    wait until rising_edge(s_slave_idle);
    wait for c_clock_cycle*10000;
    -- Read RxACK bit
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_ack); wb_expect("RxACK and IF flags set (and TIP off)?", s_wb_slave_out.dat, c_reg_sr_rack or c_reg_sr_if);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_clk);
    -- Write data byte (0xac)
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_txr, x"000000ac");
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_txr, x"000000ac");
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_txr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Set start and write bits
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sta or c_reg_cr_wr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sta or c_reg_cr_wr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_cr, c_reg_all_zero);
    wait until rising_edge(s_clk);
    -- Wait until Slave received address (and r/w bit)
    wait until rising_edge(s_slave_idle);
    wait for c_clock_cycle*10000;
    -- Read RxACK bit
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_ack); wb_expect("RxACK and IF flags set (and TIP off)?", s_wb_slave_out.dat, c_reg_sr_rack or c_reg_sr_if);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_sr, c_reg_sr);
    wait until rising_edge(s_clk);
    -- Set stop bit
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sto);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_cr, c_reg_cr_sto);
    wait until rising_edge(s_clk); s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_cr, c_reg_all_zero);
    -- Finish simulation
    wait for c_clock_cycle*10000;
    -- Using STOP_TIME from settings.sh here
    --report "Simulation done!" severity failure;
  end process;

end;
