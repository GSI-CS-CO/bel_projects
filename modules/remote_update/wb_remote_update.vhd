library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.remote_update_pkg.all;

entity wb_remote_update is
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out
      
  );
end entity;

architecture arch of wb_remote_update is
  signal  s_asmi_busy         : std_logic;
  signal  s_asmi_data_valid   : std_logic;
  signal  s_asmi_dataout      : std_logic_vector(7 downto 0);
  signal  s_asmi_addr         : std_logic_vector(23 downto 0);
  signal  s_asmi_rden         : std_logic;
  signal  s_asmi_read         : std_logic;
  signal  s_read_strobe       : std_logic;
  signal  s_write_strobe      : std_logic;
  signal  s_data_in           : std_logic_vector(23 downto 0);
  signal  s_busy              : std_logic;
  signal  s_data_out          : std_logic_vector(23 downto 0);
  signal  s_pof_error         : std_logic;
      
  type    t_wb_cyc  is (idle, stall, busy_wait, cycle_end, err); 
  signal  wb_state            : t_wb_cyc;
  signal  r_pof_error         : std_logic_vector(0 downto 0);
  signal  r_config            : std_logic_vector(1 downto 0);
  
  

begin
  
  asmi: altasmi
    port map (
     addr        => s_asmi_addr,
     clkin       => clk_sys_i,
     rden        => s_asmi_rden,
     read        => s_asmi_read,
     reset       => not rst_n_i,
     busy        => s_asmi_busy,
     data_valid  => s_asmi_data_valid,
     dataout     => s_asmi_dataout);

  aru:  remote_update
    port map (
      asmi_busy         => s_asmi_busy,
      asmi_data_valid   => s_asmi_data_valid,
      asmi_dataout      => s_asmi_dataout,
      clock             => clk_sys_i,
      data_in           => slave_i.dat(23 downto 0),
      param             => slave_i.adr(4 downto 2),
      read_param        => s_read_strobe,
      reconfig          => r_config(0),
      reset             => not rst_n_i,
      reset_timer       => r_config(1),
      write_param       => s_write_strobe,
      asmi_addr         => s_asmi_addr,
      asmi_rden         => s_asmi_rden,
      asmi_read         => s_asmi_read,
      busy              => s_busy,
      data_out          => s_data_out,
      pof_error         => s_pof_error);


  slave_o.dat <=  x"00" & s_data_out  when (slave_i.adr(5 downto 0) <= '1' & x"c") else
                  x"0000000" & "000" & r_pof_error when (slave_i.adr(5 downto 0) = "10" & x"4") else
                  (others => '0');
      

  wb_cycle: process (clk_sys_i, rst_n_i, slave_i, s_pof_error)
  begin
    if rising_edge(clk_sys_i) then
      -- It is vitally important that for each occurance of
      --   (cyc and stb and not stall) there is (ack or rty or err)
      --   sometime later on the bus.
      --
      
      if rst_n_i = '0' then
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        r_config        <= (others => '0');
        r_pof_error     <= (others => '0');
      else
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
      
        case wb_state is
          when idle =>
            if slave_i.cyc = '1' and slave_i.stb = '1' then
              -- register of altera remote update
              if (slave_i.adr(5 downto 0) <= "01" & x"c") then
                wb_state        <= stall;
                if slave_i.we = '1' then
                  s_write_strobe  <= '1';
                else
                  s_read_strobe <= '1';
                end if;
                slave_o.stall <= '1';
              -- trigger reconfiguration (0) or watchdog timer reset (1)
              elsif (slave_i.adr(5 downto 0) = "10" & x"0") then
                if (slave_i.sel(0) = '1' and slave_i.we = '1') then
                  r_config <= slave_i.dat(1 downto 0);
                  slave_o.ack <= '1';
                  wb_state <= idle;
                end if;
              elsif (slave_i.adr(5 downto 0) = "10" & x"4") then
                if slave_i.we = '0' then
                  slave_o.ack <= '1';
                  wb_state <= idle;
                end if;
              else
                wb_state <= err;
              end if;
            end if;
              
          when stall =>
            slave_o.stall   <= '1';
            wb_state        <= busy_wait;
          
          when busy_wait =>
            slave_o.stall   <= '1';
            if s_busy = '0' then
              wb_state <= cycle_end;
            end if;

          when cycle_end =>
            slave_o.ack <= '1';
            wb_state <= idle;
          
          when err =>
            slave_o.err <= '1';
            wb_state <= idle;
            
        end case;
        
      end if;
    end if;
    
    -- store information if the flash image is bad
    if s_pof_error = '1' then
      r_pof_error(0) <= '1';
    end if;
   
  end process;
  
  slave_o.int <= '0';
  slave_o.rty <= '0';
  


end architecture;