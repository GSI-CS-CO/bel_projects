library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.remote_update_pkg.all;

entity wb_asmi is
  generic ( PAGESIZE : INTEGER );
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
  
  --signal  s_wren          : std_logic;
  signal  s_write         : std_logic;
  signal  s_datain        : std_logic_vector(7 downto 0);
  signal  s_illegal_write : std_logic;
  
  signal  s_sector_erase    : std_logic;
  signal  s_illegal_erase   : std_logic;

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
     shift_bytes  => s_shift_bytes,
     write        => s_write,
     --wren         => s_wren,
     sector_erase   => s_sector_erase,
     illegal_write => s_illegal_write,
     illegal_erase => s_illegal_erase,
     reset        => not rst_n_i,
     busy         => s_busy,
     datain       => s_datain, 
     data_valid   => s_data_valid,
     dataout      => s_dataout,
     rdid_out     => s_rdid_out,
     status_out   => s_status_out);


  with slave_i.sel(3 downto 0) select s_datain <=
                                      slave_i.dat(7 downto 0)   when x"1",
                                      slave_i.dat(15 downto 8)  when x"2",
                                      slave_i.dat(23 downto 16) when x"4",
                                      slave_i.dat(31 downto 24) when x"8",
                                      (others => '0')           when others;
      
  with slave_i.adr(3 downto 0) select slave_o.dat <=
                                      s_status_out & s_status_out & s_status_out & s_status_out  when  x"4",
                                      s_rdid_out & s_rdid_out & s_rdid_out & s_rdid_out          when  x"8",
                                      s_dataout & s_dataout & s_dataout & s_dataout              when  x"0",
                                      (others => '0')                                            when others;

  --s_addr <= slave_i.adr(27 downto 4); -- lower addresses are mapped to special registers

  wb_cycle: process (clk_sys_i, rst_n_i, slave_i)
    variable  s_byte_count : integer range  0 to PAGESIZE;
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
        s_byte_count    :=  0;
        s_shift_bytes   <= '0';
        s_addr          <= (others => '0');
      else
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        s_read_rdid     <= '0';
        s_read_status   <= '0';
        s_read          <= '0';
        s_rden          <= '0';
        s_write         <= '0';
        s_shift_bytes   <= '0';
        s_sector_erase  <= '0';
      
        case wb_state is
          when idle =>
            if slave_i.cyc = '1' and slave_i.stb = '1' then
              -- read status from epcs
              if (slave_i.adr(3 downto 0) = x"4") then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_read_status <= '1';
                end if;
                
              -- read memory capacity id from epcs
              elsif (slave_i.adr(3 downto 0) = x"8") then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_read_rdid <= '1';
                end if;
              
              -- sector erase
              elsif (slave_i.adr(3 downto 0) = x"c") then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '1' then
                  -- shift sector number (0-63) 18 Bits for sector size (2**18)
                  case slave_i.sel is
                    when x"1" =>  s_addr <= slave_i.dat(5 downto 0) & "00" & x"0000";
                    when x"2" =>  s_addr <= slave_i.dat(13 downto 8) & "00" & x"0000";
                    when x"4" =>  s_addr <= slave_i.dat(21 downto 16) & "00" & x"0000";
                    when x"8" =>  s_addr <= slave_i.dat(29 downto 24) & "00" & x"0000";
                    when others => s_addr <= (others => '0');
                  end case;
                  s_sector_erase <= '1';
                end if;
              
              -- write buffer to flash
              elsif (slave_i.adr(3 downto 0) = x"f") then
                if (slave_i.sel(3 downto 0) = x"f") then
                  s_addr <= slave_i.dat(23 downto 0);
                  slave_o.stall <= '1';
                  wb_state <= stall;
                  s_write <= '1';
                  s_byte_count := 0;
                else
                  wb_state <= err;
                end if;
                
              -- access to flash          
              elsif (slave_i.adr(3 downto 0) = x"0") then
                -- read from flash
                if slave_i.we = '0' then
                  s_addr <= slave_i.adr(27 downto 4);
                  s_read <= '1';
                  s_rden <= '1';
                  slave_o.stall <= '1';
                  wb_state <= read_valid;
                -- write to page buffer
                elsif slave_i.we = '1' then
                  s_shift_bytes <= '1';
                  if s_byte_count < PAGESIZE then
                    slave_o.ack <= '1'; -- written to fifo
                    s_byte_count := s_byte_count + 1;
                  else
                    slave_o.err <= '1';
                  end if;
                end if;
                
               else
                  slave_o.err <= '1';
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
            if s_illegal_write = '1' or s_illegal_erase = '1' then
              wb_state <= err;
            else
              wb_state <= busy_wait;
            end if;
          
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