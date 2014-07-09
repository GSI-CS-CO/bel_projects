-- libraries and packages
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- submodules
library work;
use work.wb_ssd1325_serial_driver_pkg.all;

-- entity
entity ssd1325_serial_driver_testbench is
end ssd1325_serial_driver_testbench;

-- architecture
architecture rtl of ssd1325_serial_driver_testbench is

  -- test signals
  signal s_tb_clk_system     : std_logic;
  signal s_tb_rst_n_system   : std_logic;
  
  signal s_tb_slave_addr_out : std_logic_vector(31 downto 0);
  signal s_tb_slave_data_out : std_logic_vector(31 downto 0);  
  signal s_tb_slave_we_out   : std_logic;
  signal s_tb_slave_cyc_out  : std_logic;
  signal s_tb_slave_stb_out  : std_logic;
  signal s_tb_slave_data_in  : std_logic_vector(31 downto 0);
  signal s_tb_slave_stall_in : std_logic;
  signal s_tb_slave_ack_in   : std_logic;
  signal s_tb_slave_err_in   : std_logic;
  signal s_tb_slave_rty_in   : std_logic;
  signal s_tb_slave_int_in   : std_logic;
  
  signal s_ssd_rst           : std_logic;
  signal s_ssd_dc            : std_logic;
  signal s_ssd_ss            : std_logic;
  signal s_ssd_sclk          : std_logic;
  signal s_ssd_sd            : std_logic;
  signal s_ssd_irq           : std_logic;
  
  -- signals for test cases
  signal s_tb_cyc_counter           : std_logic_vector(31 downto 0);
  signal s_perform_legacy_tests     : std_logic;
  signal s_perform_wb_to_spi_tests  : std_logic;
   
  type t_test_case_signals is record
    s_slave_addr_out    : std_logic_vector(31 downto 0);
    s_slave_data_out    : std_logic_vector(31 downto 0);
    s_slave_we_out      : std_logic;
    s_slave_cyc_out     : std_logic;
    s_slave_stb_out     : std_logic;
  end record;
  
  type   t_test_case_records is array (0 to 1) of t_test_case_signals;
  signal s_test_case_connector : t_test_case_records;
   
  -- test bench settings
  constant c_system_clock_cycle : time := 16 ns;  -- i.e.: 16 => 125Mhz
  constant c_system_reset_delay : time := 100 ns; -- reset duration at test bench start
  
  -- SPI transfer
  subtype	t_spi_byte            is std_logic_vector(7 downto 0) ;
  type    t_spi_byte_array      is array (5 downto 0) of t_spi_byte ;
  signal  s_spi_byte_array      : t_spi_byte_array := (x"00",x"ff",x"aa",x"55",x"12",x"34");
  type    t_spi_single_byte     is array (5 downto 0) of std_logic_vector(7 downto 0) ;
  signal  s_spi_sample_byte     : t_spi_single_byte;
  signal  s_spi_sample_reg      : std_logic_vector(7 downto 0);
  signal  s_reset_sample_logic  : std_logic;
    
begin

  -- instantiate the driver
  wb_ssd1325_serial_driver_test : entity work.wb_ssd1325_serial_driver
  port map (
    clk_sys_i     => s_tb_clk_system,
    rst_n_i       => s_tb_rst_n_system,
    slave_i.adr   => s_tb_slave_addr_out,
    slave_i.dat   => s_tb_slave_data_out, 
    slave_i.we    => s_tb_slave_we_out,
    slave_i.cyc   => s_tb_slave_cyc_out,
    slave_i.stb   => s_tb_slave_stb_out,
    slave_i.sel   => (others => 'Z'),
    slave_o.dat   => s_tb_slave_data_in,
    slave_o.stall => s_tb_slave_stall_in,
    slave_o.ack   => s_tb_slave_ack_in,
    slave_o.err   => s_tb_slave_err_in,
    slave_o.rty   => s_tb_slave_rty_in,
    slave_o.int   => s_tb_slave_int_in,
    ssd_rst_o     => s_ssd_rst,
    ssd_dc_o      => s_ssd_dc,
    ssd_ss_o      => s_ssd_ss,
    ssd_sclk_o    => s_ssd_sclk,
    ssd_data_o    => s_ssd_sd,
    ssd_irq_o     => s_ssd_irq
  );   

  -- system clock
  p_clock : process
  begin
    s_tb_clk_system <= '0';
    wait for (c_system_clock_cycle/2);
    s_tb_clk_system <= '1';
    wait for (c_system_clock_cycle/2); 
  end process;

  -- system reset
  p_reset : process
  begin
    s_tb_rst_n_system <= '0';
    wait for c_system_reset_delay;
    s_tb_rst_n_system <= '1';
    wait;       
  end process;
  
  -- system counter/time
  p_time : process (s_tb_clk_system, s_tb_rst_n_system)
  begin
    if (s_tb_rst_n_system = '0') then
      s_tb_cyc_counter <= (others => '0');
    elsif (rising_edge(s_tb_clk_system)) then
      s_tb_cyc_counter <= std_logic_vector(unsigned(s_tb_cyc_counter)+1);
    end if;
  end process;
  
  -- state machine for test case selection
  p_test_case_selection : process (s_tb_clk_system, s_tb_rst_n_system) 
  begin
    if (s_tb_rst_n_system = '0') then
      s_perform_legacy_tests      <= '0';
      s_perform_wb_to_spi_tests   <= '0';
    elsif (rising_edge(s_tb_clk_system)) then
      if (unsigned(s_tb_cyc_counter) < 8000) then
        s_perform_legacy_tests    <= '1';
        s_perform_wb_to_spi_tests <= '0';
      elsif (unsigned(s_tb_cyc_counter) < 16000) then
        s_perform_legacy_tests    <= '0';
        s_perform_wb_to_spi_tests <= '1';
      else
        report "Simulation completed successfully!" severity failure;
      end if;
    end if;  
  end process;
  
  -- multiplex test case signals
  p_test_case_multiplexor : process (s_tb_rst_n_system, s_perform_legacy_tests, s_perform_wb_to_spi_tests, s_test_case_connector) 
  begin
    if (s_tb_rst_n_system = '0') then
      s_tb_slave_addr_out <= (others => 'Z');
      s_tb_slave_data_out <= (others => 'Z');
      s_tb_slave_we_out   <= '0';
      s_tb_slave_cyc_out  <= '0';
      s_tb_slave_stb_out  <= '0';      
    elsif (s_perform_legacy_tests = '1') then
      s_tb_slave_addr_out <= s_test_case_connector(0).s_slave_addr_out;
      s_tb_slave_data_out <= s_test_case_connector(0).s_slave_data_out;
      s_tb_slave_we_out   <= s_test_case_connector(0).s_slave_we_out;
      s_tb_slave_cyc_out  <= s_test_case_connector(0).s_slave_cyc_out;
      s_tb_slave_stb_out  <= s_test_case_connector(0).s_slave_stb_out;
    elsif (s_perform_wb_to_spi_tests = '1') then
      s_tb_slave_addr_out <= s_test_case_connector(1).s_slave_addr_out;
      s_tb_slave_data_out <= s_test_case_connector(1).s_slave_data_out;
      s_tb_slave_we_out   <= s_test_case_connector(1).s_slave_we_out;
      s_tb_slave_cyc_out  <= s_test_case_connector(1).s_slave_cyc_out;
      s_tb_slave_stb_out  <= s_test_case_connector(1).s_slave_stb_out;
    else
      s_tb_slave_addr_out <= (others => 'Z');
      s_tb_slave_data_out <= (others => 'Z');
      s_tb_slave_we_out   <= '0';
      s_tb_slave_cyc_out  <= '0';
      s_tb_slave_stb_out  <= '0';      
    end if;  
  end process;
  
  ----------------------------------------------------------------------------------------------------------------------
  --  _     _____ ____    _    ______   __  _____ _____ ____ _____    ____    _    ____  _____ ____  
  -- | |   | ____/ ___|  / \  / ___\ \ / / |_   _| ____/ ___|_   _|  / ___|  / \  / ___|| ____/ ___| 
  -- | |   |  _|| |  _  / _ \| |    \ V /    | | |  _| \___ \ | |   | |     / _ \ \___ \|  _| \___ \ 
  -- | |___| |__| |_| |/ ___ \ |___  | |     | | | |___ ___) || |   | |___ / ___ \ ___) | |___ ___) |
  -- |_____|_____\____/_/   \_\____| |_|     |_| |_____|____/ |_|    \____/_/   \_\____/|_____|____/ 
  --                                                                                                    
  ----------------------------------------------------------------------------------------------------------------------
  
  -- legacy test cases for simple stimulation
  p_wb_stimulate_device : process (s_tb_clk_system, s_tb_rst_n_system)
  begin
    if (s_tb_rst_n_system = '0') then
      s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
      s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
      s_test_case_connector(0).s_slave_we_out   <= '0';
      s_test_case_connector(0).s_slave_cyc_out  <= '0';
      s_test_case_connector(0).s_slave_stb_out  <= '0';
    elsif (rising_edge(s_tb_clk_system)) then
      if (s_perform_legacy_tests = '1') then
        case s_tb_cyc_counter is
        
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"0000000a" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000011";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"0000000b" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"0000000c" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;	
            end if;
            
          --------------------------------------------------------------------------------
          -- double pipeline write
          when x"0000000e" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000022";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"0000000f" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000033";
          when x"00000010" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_we_out   <= '0';
              s_test_case_connector(0).s_slave_stb_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
          when x"00000011" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- triple pipeline write
          when x"00000013" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000044";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000014" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000055";
          when x"00000015" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000066";
            if (s_tb_slave_ack_in='1') then
              null;
            else
              report "Missing ack!" severity failure;		
            end if;
          when x"00000016" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
            else
              report "Missing ack!" severity failure;		
            end if;
          when x"00000017" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline read
          when x"00000033" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000034" =>
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000035" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
              if (s_tb_slave_data_in/=x"00000066") then
                report "Expected data does not match!" severity failure;		
              end if;
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"0000014f" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000f0";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000150" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000151" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"0000018f" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000aa";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000190" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000191" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
          
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"000001b0" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"00000055";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"000001b1" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"000001b2" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
          
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"000001b4" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"0000000f";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"000001b5" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"000001b6" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to low
          when x"00000500" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000004";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000501" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000502" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high
          when x"00000540" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"0000000c";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000541" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000542" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
           --------------------------------------------------------------------------------
          -- single pipeline write to set ss to low (reset ctrl bit)
          when x"00000550" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000000";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000551" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000552" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high (without ctrl bit)
          when x"00000560" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000008";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000561" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000562" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"0000058f" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"0000000f";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000590" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000591" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
          
          --------------------------------------------------------------------------------
          -- single pipeline write
          when x"0000068f" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000f0";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00000690" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00000691" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
          
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high (enable irq)
          when x"00001000" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000010";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001001" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001002" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;         
          
          --------------------------------------------------------------------------------
          -- single pipeline write (send data)
          when x"00001010" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000aa";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001011" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001012" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high (disable irq)
          when x"00001150" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000000";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001151" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001152" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;         
          
          --------------------------------------------------------------------------------
          -- single pipeline write (send data)
          when x"00001160" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000aa";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001161" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001162" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high (enable irq)
          when x"00001300" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000010";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001301" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001302" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;         
          
          --------------------------------------------------------------------------------
          -- single pipeline write (send data)
          when x"00001310" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000aa";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001311" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001312" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- single pipeline write to set ss to high (clear irq)
          when x"00001840" =>
            s_test_case_connector(0).s_slave_addr_out <= x"0000000c";
            s_test_case_connector(0).s_slave_data_out <= x"00000030";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001841" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001842" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;   
             
          --------------------------------------------------------------------------------
          -- single pipeline write (send data) irq should be back again after clearing
          when x"00001910" =>
            s_test_case_connector(0).s_slave_addr_out <= x"00000000";
            s_test_case_connector(0).s_slave_data_out <= x"000000aa";
            s_test_case_connector(0).s_slave_we_out   <= '1';
            s_test_case_connector(0).s_slave_cyc_out  <= '1';
            s_test_case_connector(0).s_slave_stb_out  <= '1';
          when x"00001911" =>
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          when x"00001912" =>
            if (s_tb_slave_ack_in='1') then
              s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
              s_test_case_connector(0).s_slave_cyc_out  <= '0';
            else
              report "Missing ack!" severity failure;		
            end if;
            
          --------------------------------------------------------------------------------
          -- reset bus signals
          when others =>
            s_test_case_connector(0).s_slave_addr_out <= (others => 'Z');
            s_test_case_connector(0).s_slave_data_out <= (others => 'Z');
            s_test_case_connector(0).s_slave_we_out   <= '0';
            s_test_case_connector(0).s_slave_cyc_out  <= '0';
            s_test_case_connector(0).s_slave_stb_out  <= '0';
          end case;  

      end if;
    end if;
  end process;
  
  ----------------------------------------------------------------------------------------------------------------------
  -- __        ______ ____  ____  ____ ___   _____ _____ ____ _____    ____    _    ____  _____ ____  
  -- \ \      / / __ )___ \/ ___||  _ \_ _| |_   _| ____/ ___|_   _|  / ___|  / \  / ___|| ____/ ___| 
  --  \ \ /\ / /|  _ \ __) \___ \| |_) | |    | | |  _| \___ \ | |   | |     / _ \ \___ \|  _| \___ \ 
  --   \ V  V / | |_) / __/ ___) |  __/| |    | | | |___ ___) || |   | |___ / ___ \ ___) | |___ ___) |
  --    \_/\_/  |____/_____|____/|_|  |___|   |_| |_____|____/ |_|    \____/_/   \_\____/|_____|____/ 
  --                                                                                                   
  ----------------------------------------------------------------------------------------------------------------------
  
  -- perform wishbone to SPI test cases
  p_perform_wb_to_spi : process (s_tb_clk_system, s_tb_rst_n_system)
    variable v_internal_time : natural := 0;
  begin
    if (s_tb_rst_n_system = '0') then
      s_test_case_connector(1).s_slave_addr_out   <= (others => 'Z');
      s_test_case_connector(1).s_slave_data_out   <= (others => 'Z');
      s_test_case_connector(1).s_slave_we_out     <= '0';
      s_test_case_connector(1).s_slave_cyc_out    <= '0';
      s_test_case_connector(1).s_slave_stb_out    <= '0';
      v_internal_time                             := 0;
    elsif (rising_edge(s_tb_clk_system)) then
      if (s_perform_wb_to_spi_tests = '1') then
        v_internal_time := v_internal_time + 1;
        case v_internal_time is
        
          --------------------------------------------------------------------------------
          -- send all bytes
          when 1 =>
            s_test_case_connector(1).s_slave_addr_out <= x"00000000";
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
            s_test_case_connector(1).s_slave_we_out   <= '1';
            s_test_case_connector(1).s_slave_cyc_out  <= '1';
            s_test_case_connector(1).s_slave_stb_out  <= '1';
            s_reset_sample_logic                      <= '1';
          when 2 =>
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
            s_reset_sample_logic                      <= '0';
          when 3 =>
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
          when 4 =>
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
          when 5 =>
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
          when 6 =>
            s_test_case_connector(1).s_slave_data_out <= x"000000" & s_spi_byte_array(v_internal_time-1);
          when 7 =>
            s_test_case_connector(1).s_slave_addr_out <= (others => 'Z');
            s_test_case_connector(1).s_slave_data_out <= (others => 'Z');
            s_test_case_connector(1).s_slave_we_out   <= '0';
            s_test_case_connector(1).s_slave_stb_out  <= '0';
          when 8 =>
            s_test_case_connector(1).s_slave_cyc_out  <= '0';
            
          --------------------------------------------------------------------------------
          -- reset bus signals
          when others =>
            s_test_case_connector(1).s_slave_addr_out <= (others => 'Z');
            s_test_case_connector(1).s_slave_data_out <= (others => 'Z');
            s_test_case_connector(1).s_slave_we_out   <= '0';
            s_test_case_connector(1).s_slave_cyc_out  <= '0';
            s_test_case_connector(1).s_slave_stb_out  <= '0';
          end case;
            
      end if;
    end if;
  end process;
  
  -- sample spi data
  s_spi_sample_reg <= "0000000" & s_ssd_sd; -- needed for shift operation
  
  p_sample_spi_data : process (s_ssd_sclk, s_reset_sample_logic, s_perform_wb_to_spi_tests)
    variable v_received_bits  : natural := 0;
    variable v_received_bytes : natural := 0;

  begin
    if (s_perform_wb_to_spi_tests = '1') then
      if (s_reset_sample_logic = '1') then
        v_received_bits  := 0;
        v_received_bytes := 0;
        for i in 0 to 5 loop
          s_spi_sample_byte(i) <= (others => '0');
        end loop;
      elsif (rising_edge(s_ssd_sclk)) then 
        if (v_received_bytes /= 0) then
          -- check if received byte matches
          if (s_spi_sample_byte(v_received_bytes-1) /= s_spi_byte_array(v_received_bytes-1)) then
            report integer'image(to_integer(unsigned(s_spi_sample_byte(v_received_bytes-1))));
            report integer'image(to_integer(unsigned(s_spi_byte_array(v_received_bytes-1))));
            report "Expected data does not match!" severity failure;
          else
            report "Expected data matches!" severity note;
          end if;
        end if;
        -- sample bits
        s_spi_sample_byte(v_received_bytes) <= s_spi_sample_byte(v_received_bytes) or std_logic_vector(unsigned(s_spi_sample_reg) sll integer(7-v_received_bits));
        v_received_bits                     := v_received_bits + 1;
        if (v_received_bits = 8) then
          v_received_bits  := 0;
          v_received_bytes := v_received_bytes + 1;
        end if;
      end if;
    end if;   
  end process;
         
end rtl;

