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
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;
    
    -- asmi interface, needed for pof check
    asmi_busy       : in std_logic;
    asmi_data_valid : in std_logic;
    asmi_dataout    : in std_logic_vector(7 downto 0);
    asmi_addr       : out std_logic_vector(23 downto 0);
    asmi_rden       : out std_logic;
    asmi_read       : out std_logic;
    asmi_to_aru     : out std_logic
    
  );
end entity;

architecture arch of wb_remote_update is
  signal  s_read_strobe       : std_logic;
  signal  s_write_strobe      : std_logic;
  signal  s_read_status		    : std_logic;
  signal  s_data_in           : std_logic_vector(23 downto 0);
  signal  s_busy              : std_logic;
  signal  s_data_out          : std_logic_vector(23 downto 0);
  signal  s_pof_error         : std_logic;
  
      
  type    t_wb_cyc  is (idle, stall, busy_wait, cycle_end, err); 
  signal  wb_state            : t_wb_cyc;
  signal  r_pof_error         : std_logic_vector(0 downto 0);
  signal  r_config            : std_logic_vector(1 downto 0);
  signal  s_asmi_to_aru       : std_logic;

begin


  aru:  remote_update
    port map (
     -- asmi_busy         => asmi_busy,
      --asmi_data_valid   => asmi_data_valid,
      --asmi_dataout      => asmi_dataout,
      clock             => clk_sys_i,
      data_in           => slave_i.dat(23 downto 0),
      param             => slave_i.adr(4 downto 2),
      read_param        => s_read_strobe,
      reconfig          => r_config(0),
      reset             => not rst_n_i,
      reset_timer       => r_config(1),
      write_param       => s_write_strobe,
      --asmi_addr         => asmi_addr,
      --asmi_rden         => asmi_rden,
      --asmi_read         => asmi_read,
      busy              => s_busy,
      data_out          => s_data_out);
      --pof_error         => s_pof_error);

      
  with slave_i.adr(5 downto 0) select slave_o.dat <=
                                      x"00" & s_data_out                when "01" & x"c",
                                      x"0000000" & "000" & r_pof_error  when "10" & x"4",
                                      (others => '0')                   when others;


  wb_cycle: process (clk_sys_i, rst_n_i, slave_i, s_pof_error)
  begin
    if rising_edge(clk_sys_i) then
      
      if rst_n_i = '0' then
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        r_config        <= (others => '0');
        r_pof_error     <= (others => '0');
        s_asmi_to_aru   <= '0';
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
                
              -- trigger reconfiguration (0) or watchdog timer reset (1)
              elsif (slave_i.adr(5 downto 0) = "10" & x"0") then
                if (slave_i.sel(0) = '1' and slave_i.we = '1') then
                  r_config <= slave_i.dat(1 downto 0);
                  slave_o.ack <= '1';
                  s_asmi_to_aru <= '1';
                end if;
                wb_state <= idle;
                
              -- read/write pof error register
              elsif (slave_i.adr(5 downto 0) = "10" & x"4") then
                if (slave_i.sel(0) = '1' and slave_i.we = '1') then
                  r_pof_error <= slave_i.dat(0 downto 0);
                  if slave_i.dat(0 downto 0) = "0" then
                    s_asmi_to_aru <= '0';
                  end if;
                end if;
                slave_o.ack <= '1';
                wb_state <= idle;
                
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
  
  asmi_to_aru <= s_asmi_to_aru;


end architecture;