library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.remote_update_pkg.all;
use work.genram_pkg.all;
use work.aux_functions_pkg.all;
use work.monster_pkg.all;
use work.ep_crc32_pkg.all;

library altera_asmi_parallel_231;
use altera_asmi_parallel_231.asmi10_pkg.all;

entity wb_asmi is
  generic (
    pagesize : integer;
    g_family : string := "none"
  );
  port (
    clk_flash_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out
      
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
  

  signal  s_busy        : std_logic;
  signal  s_data_valid  : std_logic;
  signal  s_dataout     : std_logic_vector(7 downto 0);
  signal  s_addr        : std_logic_vector(31 downto 0);
  signal  s_rden        : std_logic;
  signal  s_read        : std_logic;
  signal  s_rdid        : std_logic;
  signal  s_shift_bytes : std_logic;
  signal  s_read_status : std_logic;
  signal  s_data_in     : std_logic_vector(23 downto 0);

  signal  s_rdid_out      : std_logic_vector(7 downto 0);
  signal  s_status_out    : std_logic_vector(7 downto 0);


  type    t_wb_cyc  is (idle, stall, busy_wait, read_valid, cycle_end, err, write_addr_ready, read_addr_ready, erase_stall);
  signal  wb_state      : t_wb_cyc;

  signal  s_rden_ext    : std_logic;
  signal  s_read_ext    : std_logic;

  signal  s_read_rdid     : std_logic;
  signal  s_write_strobe  : std_logic;
  signal  s_read_strobe   : std_logic;

  signal  s_write         : std_logic;
  signal  s_wren          : std_logic;
  signal  s_datain        : std_logic_vector(7 downto 0);
  signal  s_illegal_write : std_logic;

  signal  s_sector_erase    : std_logic;
  signal  s_bulk_erase      : std_logic;
  signal  s_illegal_erase   : std_logic;
  
  signal  s_read_addr       : std_logic_vector(31 downto 0);
  signal  s_read10_addr     : std_logic_vector(31 downto 0);
  
  signal  illegal_erase     : std_logic;
  signal  illegal_write     : std_logic;
  signal  busy              : std_logic;
  signal  data_valid        : std_logic;
  
  signal read_fifo_in    : std_logic_vector(7 downto 0);
  signal read_fifo_out   : std_logic_vector(7 downto 0);
  signal read_fifo_we    : std_logic;
  signal read_fifo_rd    : std_logic;
  signal read_fifo_empty : std_logic;
  signal read_fifo_full  : std_logic;
  signal fifo_word       : std_logic_vector(31 downto 0);
  signal crc_reg         : std_logic_vector(31 downto 0);
  signal crc_new         : std_logic_vector(31 downto 0);
  signal crc_in          : std_logic_vector(31 downto 0);
  signal s_read_number   : std_logic_vector(31 downto 0) :=  std_logic_vector(to_unsigned(PAGESIZE, 32));
  signal s_first_word    : std_logic;
  signal s_en4b          : std_logic;
  signal s_ex4b          : std_logic;
  signal s_read_dclk     : std_logic;


  constant FLASH_ACCESS : std_logic_vector(7 downto 0) := x"00";
  constant READ_STATUS  : std_logic_vector(7 downto 0) := x"04";
  constant READ_ID      : std_logic_vector(7 downto 0) := x"08";
  constant SECTOR_ERASE : std_logic_vector(7 downto 0) := x"0c";
  constant SET_ADDR     : std_logic_vector(7 downto 0) := x"10";
  constant WRITE_BUFFER : std_logic_vector(7 downto 0) := x"14";
  constant FIFO_READ    : std_logic_vector(7 downto 0) := x"18";
  constant BUSY_CHECK   : std_logic_vector(7 downto 0) := x"1c";
  constant READ_CRC     : std_logic_vector(7 downto 0) := x"20";
  constant READ_NUM     : std_logic_vector(7 downto 0) := x"24";
  constant BULK_ERASE   : std_logic_vector(7 downto 0) := x"28";
  constant SET_4BMODE   : std_logic_vector(7 downto 0) := x"2c";
  constant READ_DCLK    : std_logic_vector(7 downto 0) := x"30";
  constant TIMEOUT      : integer                      := 70;
  constant SECTORSIZE   : integer                      := 16#40000#;

  component crc8_data8 is
    port (
      clock      : in std_logic;
      reset      : in std_logic;
      soc        : in std_logic;
      data       : in std_logic_vector(7 downto 0);
      data_valid : in std_logic;
      eoc        : in std_logic;
      crc        : out std_logic_vector(7 downto 0);
      crc_valid  : out std_logic
    );
  end component;
  -- changes the endianess BIG <-> LITTLE
  function ChangeEndian(vec : std_ulogic_vector) return std_ulogic_vector is
    variable vRet      : std_ulogic_vector(vec'range);
    constant cNumBytes : natural := vec'length / 8;
  begin
    for i in 0 to cNumBytes-1 loop
      for j in 7 downto 0 loop
        vRet(8*i + j) := vec(8*(cNumBytes-1-i) + j);
      end loop;  -- j
    end loop;  -- i

    return vRet;
  end function ChangeEndian;


  function ChangeEndian(vec : std_logic_vector) return std_logic_vector is
  begin
    return std_logic_vector(ChangeEndian(std_ulogic_vector(vec)));
  end function ChangeEndian;

begin
  
    a1: if g_family = "Arria II" generate
      asmi: asmi_arriaII
        port map (
         addr          => s_addr(23 downto 0),
         clkin         => not clk_flash_i,
         rden          => s_rden,
         fast_read     => s_read,
         read_rdid     => s_rdid,
         read_status   => s_read_status,
         shift_bytes   => s_shift_bytes,
         write         => s_write,
         wren          => s_wren,
         sector_erase  => s_sector_erase,
         illegal_write => illegal_write,
         illegal_erase => illegal_erase,
         reset         => not rst_n_i,
         busy          => busy,
         datain        => s_datain,
         data_valid    => data_valid,
         dataout       => s_dataout,
         rdid_out      => s_rdid_out,
         status_out    => s_status_out,
         read_address  => s_read_addr(23 downto 0)
       );
  end generate;

  a5: if g_family(1 to 7) = "Arria V" generate
    asmi_5: asmi5
      port map (
        clkin         => not clk_flash_i,
        fast_read     => s_read,
        rden          => s_rden,
        addr          => s_addr,
        read_status   => s_read_status,
        write         => s_write,
        datain        => s_datain,
        shift_bytes   => s_shift_bytes,
        sector_erase  => s_sector_erase,
        bulk_erase    => s_bulk_erase,
        wren          => s_wren,
        read_rdid     => s_rdid,
        en4b_addr     => s_en4b,
        ex4b_addr     => s_ex4b,
        reset         => not rst_n_i,
        dataout       => s_dataout,
        busy          => busy,
        data_valid    => data_valid,
        status_out    => s_status_out,
        illegal_write => illegal_write,
        illegal_erase => illegal_erase,
        read_address  => s_read10_addr,
        rdid_out      => s_rdid_out,
        read_dummyclk => s_read_dclk
      );
  end generate;

  a2: if g_family(1 to 7) = "Arria 1" generate
    asmi_10: asmi10
      port map (
        clkin         => not clk_flash_i,
        fast_read     => s_read,
        rden          => s_rden,
        addr          => s_addr,
        read_status   => s_read_status,
        write         => s_write,
        datain        => s_datain,
        shift_bytes   => s_shift_bytes,
        sector_erase  => s_sector_erase,
        wren          => s_wren,
        read_rdid     => s_rdid,
        en4b_addr     => s_en4b,
        ex4b_addr     => s_ex4b,
        reset         => not rst_n_i,
        sce           => "000", -- select flash at nCSO[0]
        dataout       => s_dataout,
        busy          => busy,
        data_valid    => data_valid,
        status_out    => s_status_out,
        illegal_write => illegal_write,
        illegal_erase => illegal_erase,
        read_address  => s_read10_addr,
        rdid_out      => s_rdid_out,
        read_dummyclk => s_read_dclk
      );
  end generate;

  -- storage for flash data until read from wishbone interface
  read_fifo: generic_sync_fifo
  generic map (
    g_data_width => 8,
    g_size        => PAGESIZE+1)
  port map (
    rst_n_i => rst_n_i,
    clk_i   => clk_flash_i,
    d_i     => read_fifo_in,
    we_i    => read_fifo_we,
    q_o     => read_fifo_out,
    rd_i    => read_fifo_rd,

    empty_o => read_fifo_empty,
    full_o  => read_fifo_full);

  fifo_rd: process(slave_i.cyc, slave_i.stb, read_fifo_empty, slave_i.adr)
  begin
    if slave_i.cyc = '1' and slave_i.stb = '1' and read_fifo_empty = '0' and slave_i.adr(7 downto 0) = FIFO_READ  and slave_i.we = '0' then
      read_fifo_rd <= '1';
    else
      read_fifo_rd <= '0';
    end if;
  end process;

  crc_arriaII: if g_family = "Arria II" generate
    crc_in <= c_CRC32_INIT_VALUE when (s_first_word = '1' and read_fifo_we = '1') else
              crc_reg;
  end generate crc_arriaII;
  crc_not_arriaII: if g_family /= "Arria II" generate
    crc_in <= c_CRC32_INIT_VALUE when (s_first_word = '1' and data_valid = '1') else
              crc_reg;
  end generate crc_not_arriaII;

  crc_new <= f_update_crc32_d8(crc_in, read_fifo_in);

  crc32: process(clk_flash_i, read_fifo_we, s_first_word, read_fifo_in)
  begin
    if rst_n_i = '0' then
      crc_reg <= (others => '0');
    elsif rising_edge(clk_flash_i) then
      if read_fifo_we = '1' then
        crc_reg <= crc_new;
      end if;
    end if;
  end process;

  input_mux: process(clk_flash_i, slave_i.sel(7 downto 0))
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

  output_mux: process(clk_flash_i, slave_i.adr(7 downto 0))
  begin
    case slave_i.adr(7 downto 0) is
      when READ_STATUS =>
        slave_o.dat <= s_status_out & x"000000";
      when READ_ID =>
        slave_o.dat <= s_rdid_out & x"000000";
      when FLASH_ACCESS =>
        slave_o.dat <= s_dataout & x"000000";
      when FIFO_READ =>
        slave_o.dat <= read_fifo_out & x"000000";
      when BUSY_CHECK =>
        slave_o.dat <= "0000000" & s_busy & "0000000" & s_busy & "0000000" & s_busy & "0000000" & s_busy;
      when SET_ADDR =>
        slave_o.dat <= s_addr;
      when READ_CRC =>
        slave_o.dat <= ChangeEndian(crc_reg);
      when READ_NUM =>
        slave_o.dat <= s_read_number;
      when others =>
        slave_o.dat <= (others => '0');
    end case;
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
    variable s_byte_count : integer range  0 to PAGESIZE;
    variable s_word_count : integer range  0 to SECTORSIZE;
    variable v_read_tmo   : integer range 0 to TIMEOUT;
  begin
    if rising_edge(clk_flash_i) then
      
      if rst_n_i = '0' then
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        s_rdid          <= '0';
        s_read_status   <= '0';
        s_byte_count    :=  0;
        s_word_count    :=  0;
        s_shift_bytes   <= '0';
        s_addr          <= (others => '0');
        v_read_tmo      := 0;
        fifo_word       <= (others => '0');
        s_wren          <= '0';
        s_first_word    <= '0';
        s_read_number   <= std_logic_vector(to_unsigned(PAGESIZE, 32));
        s_read_dclk     <= '0';
        s_en4b          <= '0';
        s_ex4b          <= '0';
      else
        s_write_strobe  <= '0';
        s_read_strobe   <= '0';
        slave_o.ack     <= '0';
        slave_o.stall   <= '0';
        slave_o.err     <= '0';
        s_rdid          <= '0';
        s_read_status   <= '0';
        s_read          <= '0';
        s_rden          <= '0';
        s_write         <= '0';
        s_shift_bytes   <= '0';
        s_sector_erase  <= '0';
        s_bulk_erase    <= '0';
        read_fifo_we    <= '0';
        s_wren          <= '0';
        s_first_word    <= '0';
        s_read_dclk     <= '0';
        s_en4b          <= '0';
        s_ex4b          <= '0';
      
        case wb_state is
          when idle =>
            if slave_i.cyc = '1' and slave_i.stb = '1' then

              -- asmi core is still busy
              if (s_busy = '1' and (slave_i.adr(7 downto 0) /= BUSY_CHECK)) then
                slave_o.err <= '1';

              -- enable 4byte address mode
              elsif (slave_i.adr(7 downto 0) = SET_4BMODE) then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '1' and slave_i.sel = x"f" then
                  if slave_i.dat(0) = '1' then
                    s_en4b <= '1';
                  elsif slave_i.dat(0) = '0' then
                    s_ex4b <= '1';
                  end if;
                  s_wren <= '1';
                end if;

              -- read dummyclock 
              elsif (slave_i.adr(7 downto 0) = READ_DCLK) then
                wb_state <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '1' and slave_i.sel = x"f" then
                  s_read_dclk <= '1';
                end if;
              
              -- read status from epcs
              elsif (slave_i.adr(7 downto 0) = READ_STATUS) then
                wb_state      <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_read_status <= '1';
                end if;

              -- read back the crc value of a page read instruction
              elsif (slave_i.adr(7 downto 0) = READ_CRC) then
                if slave_i.we = '0' and slave_i.sel = x"f" then
                  slave_o.ack <= '1';
                else
                  slave_o.err <= '1';
                end if;

              -- check if asmi core is busy
              elsif (slave_i.adr(7 downto 0) = BUSY_CHECK) then
                if slave_i.we = '0' then
                  slave_o.ack <= '1';
                else
                  slave_o.err <= '1';
                end if;
                
              -- read memory capacity id from epcs
              elsif (slave_i.adr(7 downto 0) = READ_ID) then
                wb_state      <= stall;
                slave_o.stall <= '1';
                if slave_i.we = '0' then
                  s_rdid <= '1';
                end if;
              
              -- sector erase
              elsif (slave_i.adr(7 downto 0) = SECTOR_ERASE) then
                wb_state      <= erase_stall;
                slave_o.stall <= '1';

                if slave_i.we = '1' then
                  if (slave_i.sel(3 downto 0) = x"f") then
                    s_addr <= slave_i.dat(31 downto 0);
                    s_wren         <= '1';
                    s_sector_erase <= '1';
                   end if;
                end if;

              -- bulk erase
              elsif (slave_i.adr(7 downto 0) = BULK_ERASE) then
                wb_state      <= erase_stall;
                slave_o.stall <= '1';

                if slave_i.we = '1' then
                  if (slave_i.sel(3 downto 0) = x"f") then
                    s_addr <= slave_i.dat(31 downto 0);
                    s_wren         <= '1';
                    s_bulk_erase <= '1';
                   end if;
                end if;

              -- read from fifo
              elsif (slave_i.adr(7 downto 0) = FIFO_READ) then
                if slave_i.we = '0' then
                  if (slave_i.sel(3 downto 0) = x"8") then
                    if read_fifo_empty = '0' then
                      slave_o.ack <= '1';
                    else 
                      slave_o.err <= '1';
                    end if;
                  else
                    slave_o.err <= '1';
                  end if;
                else
                  slave_o.err <= '1';
                end if;
              
              -- set addr for read/write
              elsif (slave_i.adr(7 downto 0) = SET_ADDR) then
                if (slave_i.sel(3 downto 0) = x"f") then
                  if slave_i.we = '1' then
                    s_addr <= slave_i.dat(31 downto 0);
                  end if;
                  slave_o.ack <= '1';
                  wb_state <= idle;    
                else
                  wb_state <= err;
                end if;

              -- set number of bytes to read
              elsif (slave_i.adr(7 downto 0) = READ_NUM) then
                if (slave_i.sel(3 downto 0) = x"f") then
                  if slave_i.we = '1' then
                    s_read_number <= slave_i.dat(31 downto 0);
                  end if;
                  slave_o.ack <= '1';
                  wb_state <= idle;
                else
                  wb_state <= err;
                end if;

              -- write buffer to the flash
              elsif (slave_i.adr(7 downto 0) = WRITE_BUFFER) then
                slave_o.stall <= '1';
                if (slave_i.sel(3 downto 0) = x"f") and slave_i.we = '1' then
                  s_addr <= slave_i.dat(31 downto 0);
                  wb_state <= write_addr_ready;    
                  s_byte_count := 0;
                else
                  wb_state <= err;
                end if;
                
              -- access to flash          
              elsif (slave_i.adr(7 downto 0) = FLASH_ACCESS) then
                if slave_i.we = '0' then
                  --slave_o.stall <= '1';
                  -- do not wait for busy going down
                  slave_o.ack <= '1';
                  wb_state <= read_addr_ready;
                -- write to page buffer
                elsif slave_i.we = '1' then
                  s_wren        <= '1';
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
          
          -- start multi byte read
          when read_addr_ready =>
            s_rden        <= '1';
            s_read        <= '1';
            --slave_o.stall <= '1';
            wb_state      <= read_valid;
          
          -- write buffer to flash
          when write_addr_ready =>
            s_wren        <= '1';
            s_write       <= '1';
            slave_o.stall <= '1';
            wb_state      <= stall;
             
          -- multi byte read
          when read_valid =>
            --slave_o.stall <= '1';
            s_rden        <= '1';
            if slave_i.cyc = '1' and slave_i.stb = '1' and slave_i.adr(7 downto 0) = BUSY_CHECK then
              slave_o.ack <= '1';
            -- check if data valid ever comes
            elsif v_read_tmo = TIMEOUT then
              wb_state <= err;
              v_read_tmo := 0;
            -- valid data on the output
            elsif s_data_valid = '1' then
              if s_word_count = 0 then
                s_first_word <= '1';
              end if;
              s_word_count := s_word_count + 1;
              read_fifo_in <= s_dataout;
              read_fifo_we <= '1';
              v_read_tmo := 0;
            -- stop reading after one page
            elsif s_word_count = to_integer(unsigned(s_read_number)) then
              v_read_tmo := 0;
              s_byte_count := 0;
              s_word_count := 0;
              wb_state <= idle;
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

          when erase_stall =>
            slave_o.stall   <= '1';
            if s_illegal_write = '1' or s_illegal_erase = '1' then
              wb_state <= err;
            else
              -- do not wait for busy going down, etherbone will get an timeout error
              wb_state <= cycle_end;
            end if;
          
          when busy_wait =>
            slave_o.stall   <= '1';
            if s_illegal_write = '1' or s_illegal_erase = '1' then
              wb_state <= err;
            elsif s_busy = '0' then
              wb_state <= cycle_end;
            end if;

          when cycle_end =>
            slave_o.ack <= '1';
            wb_state    <= idle;
          
          when err =>
            slave_o.err <= '1';
            wb_state    <= idle;
            
        end case;
        
      end if;
    end if;
    

   
  end process;
  
  slave_o.rty <= '0';
  


end architecture;
