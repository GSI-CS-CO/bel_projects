library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.remote_update_pkg.all;

entity wb_asmi is
  generic ( PAGESIZE : INTEGER );
  port (
    clk_flash_i : in std_logic;
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

  component altera_spi is
    generic(
      g_family     : string  := "none";
      g_port_width : natural := 1);
    port(
      dclk_i : in  std_logic;
      ncs_i  : in  std_logic;
      oe_i   : in  std_logic_vector(0 downto 0);
      asdo_i : in  std_logic_vector(0 downto 0);
      data_o : out std_logic_vector(0 downto 0));
  end component;
  
  signal flash_ncs  : std_logic;
  signal flash_dclk : std_logic;
  signal flash_oe   : std_logic_vector(0 downto 0);
  signal flash_asdo : std_logic_vector(0 downto 0);
  signal flash_data : std_logic_vector(0 downto 0);
  

  signal  s_busy          : std_logic;
  signal  s_data_valid    : std_logic;
  signal  s_dataout       : std_logic_vector(7 downto 0);
  signal  s_addr          : std_logic_vector(23 downto 0);
  signal  s_asmi_addr     : std_logic_vector(23 downto 0);
  signal  s_rden          : std_logic;
  signal  s_asmi_rden     : std_logic;
  signal  s_read          : std_logic;
  signal  s_asmi_read     : std_logic;
  signal  s_rdid          : std_logic;
  signal  s_shift_bytes   : std_logic;
  signal  s_read_status   : std_logic;
  signal  s_data_in       : std_logic_vector(23 downto 0);

  signal  s_rdid_out      : std_logic_vector(7 downto 0);
  signal  s_status_out    : std_logic_vector(7 downto 0);


  type    t_wb_cyc  is (idle, stall, busy_wait, read_valid, cycle_end, err, write_addr_ready, read_addr_ready);
  signal  wb_state            : t_wb_cyc;

  signal  s_addr_ext    : std_logic_vector(23 downto 0);
  signal  s_rden_ext    : std_logic;
  signal  s_read_ext    : std_logic;

  signal  s_read_rdid     : std_logic;
  signal  s_write_strobe  : std_logic;
  signal  s_read_strobe   : std_logic;

  signal  s_write         : std_logic;
  signal  s_datain        : std_logic_vector(7 downto 0);
  signal  s_illegal_write : std_logic;

  signal  s_sector_erase    : std_logic;
  signal  s_illegal_erase   : std_logic;
  
  signal  s_read_addr       : std_logic_vector(23 downto 0);
  
  signal  illegal_erase     : std_logic;
  signal  illegal_write     : std_logic;
  signal  busy              : std_logic;
  signal  data_valid        : std_logic;

begin
  
  asmi_addr_mux: process (clk_flash_i, asmi_to_ext)
  begin
    if rising_edge(clk_flash_i) then
      if asmi_to_ext = '1' then
        s_asmi_addr <= asmi_addr_ext;
      else
        s_asmi_addr <= s_addr;
      end if;
    end if;
  end process;

  asmi_rden_mux: process (clk_flash_i, asmi_to_ext)
  begin
    if rising_edge(clk_flash_i) then
      if asmi_to_ext = '1' then
        s_asmi_rden <= asmi_rden_ext;
      else
        s_asmi_rden <= s_rden;
      end if;
    end if;
  end process;

  asmi_read_mux: process (clk_flash_i, asmi_to_ext)
  begin
    if rising_edge(clk_flash_i) then
      if asmi_to_ext = '1' then
        s_asmi_read <= asmi_read_ext;
      else
        s_asmi_read <= s_read;
      end if;
    end if;
  end process;
 
  
  spi : altera_spi
    generic map(
      g_family     => "Arria II GX",
      g_port_width => 1)
    port map(
      dclk_i => flash_dclk,
      ncs_i  => flash_ncs,
      oe_i   => flash_oe,
      asdo_i => flash_asdo,
      data_o => flash_data);
  
  asmi: altasmi
    port map (
     addr         => s_asmi_addr,
     clkin        => not clk_flash_i,
     rden         => s_asmi_rden,
     fast_read    => s_asmi_read,
     read_rdid	  => s_read_rdid,
     read_status  => s_read_status,
     shift_bytes  => s_shift_bytes,
     write        => s_write,
     sector_erase   => s_sector_erase,
     illegal_write => illegal_write,
     illegal_erase => illegal_erase,
     reset        => not rst_n_i,
     busy         => busy,
     datain       => s_datain, 
     data_valid   => data_valid,
     dataout      => s_dataout,
     rdid_out     => s_rdid_out,
     status_out   => s_status_out,
     read_address => s_read_addr,
     asmi_dataout => flash_data,
     asmi_sdoin   => flash_asdo,
     asmi_dataoe  => flash_oe,
     asmi_dclk    => flash_dclk,
     asmi_scein   => flash_ncs
     
     
     );
  input_mux: process(clk_flash_i, slave_i.sel(3 downto 0))
  begin
    if rising_edge(clk_flash_i) then
      case slave_i.sel(3 downto 0) is
        when x"1" =>
          s_datain <= slave_i.dat(7 downto 0);
        when x"2" =>
          s_datain <= slave_i.dat(15 downto 8);
        when x"4" =>
          s_datain <= slave_i.dat(23 downto 16);
        when x"8" =>
          s_datain <= slave_i.dat(31 downto 24);
        when others =>
          s_datain <= (others => '0');
      end case;
    end if;
  end process;

  output_mux: process(clk_flash_i, slave_i.adr(3 downto 0))
  begin
    if rising_edge(clk_flash_i) then
      case slave_i.adr(3 downto 0) is
        when x"4" =>
          slave_o.dat <= s_status_out & s_status_out & s_status_out & s_status_out;
        when x"8" =>
          slave_o.dat <= s_rdid_out & s_rdid_out & s_rdid_out & s_rdid_out;
        when x"0" =>
          slave_o.dat <= s_dataout & s_dataout & s_dataout & s_dataout;
        when others =>
          slave_o.dat <= (others => '0');
      end case;
    end if;
  end process;

  
  reg_flash_signals: process(clk_flash_i)
  begin
    if rising_edge(clk_flash_i) then
      s_data_valid      <= data_valid;
      s_busy            <= busy;
      s_illegal_write   <= illegal_write;
      s_illegal_erase   <= illegal_erase;
    end if;
   
  
  end process;
  

  wb_cycle: process (clk_flash_i, rst_n_i, slave_i)
    variable  s_byte_count : integer range  0 to PAGESIZE;
    variable  v_read_tmo : integer range 0 to 70;
  begin
    if rising_edge(clk_flash_i) then
      
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
        v_read_tmo      := 0;
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
                  if (slave_i.sel(3 downto 0) = x"f") then
                    s_addr <= slave_i.dat(23 downto 0);
                    s_sector_erase <= '1';
                   end if;
                end if;
              
              -- set addr for write
              elsif (slave_i.adr(3 downto 0) = x"f") then
                if (slave_i.sel(3 downto 0) = x"f") then
                  s_addr <= slave_i.dat(23 downto 0);
                  slave_o.stall <= '1';
                  wb_state <= write_addr_ready;    
                  s_byte_count := 0;
                else
                  wb_state <= err;
                end if;
                
              -- access to flash          
              elsif (slave_i.adr(3 downto 0) = x"0") then
                -- set addr for read
                if slave_i.we = '0' then
                  s_addr <= slave_i.adr(27 downto 4);
                  slave_o.stall <= '1';
                  wb_state <= read_addr_ready;
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
          
          -- start read
          when read_addr_ready =>
            s_read <= '1';
            s_rden <= '1';
            slave_o.stall <= '1';
            wb_state <= read_valid;
          
          -- write buffer to flash
          when write_addr_ready =>
            s_write <= '1';
            slave_o.stall <= '1';
            wb_state <= stall;
             
          when read_valid =>
            slave_o.stall <= '1';
            -- check if data valid ever comes
            if v_read_tmo = 70 then
              wb_state <= err;
              v_read_tmo := 0;
            elsif s_data_valid = '1' then
              slave_o.ack <= '1';
              wb_state <= idle;
              v_read_tmo := 0;
            else
              v_read_tmo := v_read_tmo + 1;
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
