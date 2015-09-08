library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.remote_update_pkg.all;

entity wb_asmi is
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;
    
    -- asmi interface, needed for pof check
    asmi_busy       : out std_logic;
    asmi_data_valid : out std_logic;
    asmi_dataout    : out std_logic_vector(7 downto 0);
    asmi_addr_ext   : in std_logic_vector(23 downto 0);
    asmi_rden_ext   : in std_logic;
    asmi_read_ext   : in std_logic;
    -- needed for multiplexing
    asmi_to_ext     : in std_logic

      
  );
end entity;

architecture arch of wb_asmi is
  signal  s_busy          : std_logic;
  signal  s_data_valid    : std_logic;
  signal  s_dataout       : std_logic_vector(7 downto 0);
  signal  s_addr          : std_logic_vector(23 downto 0);
  signal  s_rden          : std_logic;
  signal  s_read          : std_logic;
  signal  s_rdid          : std_logic;
  signal  s_shift_bytes   : std_logic;
  signal  s_read_status	  : std_logic;
  signal  s_data_in       : std_logic_vector(23 downto 0);

  signal  s_rdid_out		  : std_logic_vector(7 downto 0);
  signal  s_status_out		: std_logic_vector(7 downto 0);
  
      
  type    t_wb_cyc  is (idle, stall, busy_wait, read_valid, cycle_end, err); 
  signal  wb_state            : t_wb_cyc;

  signal  s_addr_ext    : std_logic_vector(23 downto 0);
  signal  s_rden_ext    : std_logic;
  signal  s_read_ext    : std_logic;
  
  signal  s_read_rdid     : std_logic;
  signal  s_write_strobe  : std_logic;
  signal  s_read_strobe   : std_logic;
  

begin
  
  with asmi_to_ext select s_addr <=
                          s_addr_ext when '1',
                          s_addr when '0';
                          
  with asmi_to_ext select s_rden <=
                          s_rden_ext when '1',
                          s_rden when '0';
                          
  with asmi_to_ext select s_read <=
                          s_read_ext when '1',
                          s_read when '0';
  
  
  
  asmi: altasmi
    port map (
     addr         => s_addr,
     clkin        => clk_sys_i,
     rden         => s_rden,
     fast_read    => s_read,
     read_rdid	  => s_read_rdid,
     read_status  => s_read_status,
     shift_bytes  => '0',
     reset        => not rst_n_i,
     busy         => s_busy,
     data_valid   => s_data_valid,
     dataout      => s_dataout,
     rdid_out     => s_rdid_out,
     status_out   => s_status_out);


      
  with slave_i.adr(24 downto 0) select slave_o.dat <=
                                      x"000000" & s_rdid_out            when '0' & x"000004",
                                      x"000000" & s_status_out          when '0' & x"000000",
                                      x"000000" & s_dataout             when others;

  s_addr <= slave_i.adr(25 downto 2); -- lower addresses are mapped to special registers

  wb_cycle: process (clk_sys_i, rst_n_i, slave_i)
  begin
    if rising_edge(clk_sys_i) then
      
      if rst_n_i = '0' then
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        s_read_rdid     <= '0';
        s_read_status   <= '0';
      else
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        s_read_rdid     <= '0';
        s_read_status   <= '0';
      
        case wb_state is
          when idle =>
            if slave_i.cyc = '1' and slave_i.stb = '1' then
              -- read status from epcs
              if (slave_i.adr(24 downto 0) = '0' & x"000000") then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_read_status <= '1';
                end if;
                
              -- read memory capacity id from epcs
              elsif (slave_i.adr(24 downto 0) = '0' & x"000004") then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_read_rdid <= '1';
                end if;
                
              -- access to flash          
              elsif (slave_i.adr(24 downto 0) >= '0' & x"000100") then
                if slave_i.we = '0' then
                  s_read <= '1';
                  s_rden <= '1';
                  slave_o.stall <= '1';
                  wb_state <= read_valid;
                end if;
              
              else
              
                wb_state <= err;
              end if;
            end if;
             
          when read_valid =>
            slave_o.stall <= '1';
            if s_data_valid = '1' then
              slave_o.ack <= '1';
              wb_state <= idle;
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
    

   
  end process;
  
  slave_o.int <= '0';
  slave_o.rty <= '0';
  


end architecture;