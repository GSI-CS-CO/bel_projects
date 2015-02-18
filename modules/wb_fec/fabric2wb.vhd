library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.genram_pkg.all;

entity fabric2wb is
  generic(
    g_size_ram  : integer := 3200);

  port(
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    dec_snk_i       : in  t_wrf_sink_in;
    dec_snk_o       : out t_wrf_sink_out;
    irq_fwb_o       : out std_logic;    
    wb_ram_master_i : in  t_wishbone_master_in;
    wb_ram_master_o : out t_wishbone_master_out;
    wb_fwb_slave_i  : in  t_wishbone_slave_in;
    wb_fwb_slave_o  : out t_wishbone_slave_out);
end fabric2wb;

architecture rtl of fabric2wb is
  constant c_num_fec_blocks: integer := 4;
  constant c_num_frames_ram: integer := 4;
  constant c_words_frame   : integer := (g_size_ram / c_num_frames_ram);
  constant c_words_frame_wb: integer := (g_size_ram / c_num_frames_ram)/2;
  constant c_sel           : std_logic_vector(3 downto 0) := ("0011");
  signal s_new_frame_d0    : std_logic;
  signal s_cnt_words       : unsigned(f_log2_size(c_words_frame)-1 downto 0);
  signal s_cnt_words_last  : unsigned(f_log2_size(c_words_frame)-1 downto 0);
  signal s_cnt_ts          : unsigned(f_log2_size(c_words_frame)-1 downto 0);
  signal s_cnt_words_ram   : unsigned(f_log2_size(g_size_ram)-1 downto 0);
  --signal s_hot_slot        : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
  signal s_hot_slot        : integer range 0 to c_num_frames_ram-1;
  signal s_cnt_frames_ts   : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
  signal s_cnt_overrun     : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
  signal s_freed_slot      : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
  --signal s_r_slot          : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
  signal s_r_slot          : integer  range 0 to c_num_frames_ram-1;
  signal s_write_slot      : unsigned(c_num_frames_ram-1 downto 0);
  signal s_rst_words_ram   : std_logic;
  signal s_rst_overrun     : std_logic;
  signal s_read_slot       : std_logic;

begin

  -- this wb slave doesn't supoort them
  dec_snk_o.rty <= '0';
  dec_snk_o.err <= '0';
  
  -- master2fabric
  dec_snk_o.ack   <= wb_ram_master_i.ack;
  dec_snk_o.stall <= wb_ram_master_i.stall;

  -- fabric2master
  wb_ram_master_o.dat <= x"0000" & dec_snk_i.dat;

  wb_ram_master_o.sel <= c_sel;

  wb_ram_master_o.cyc <= dec_snk_i.cyc when dec_snk_i.adr = c_WRF_DATA or 
                                            dec_snk_i.adr = c_WRF_OOB else
                          '0';
  wb_ram_master_o.stb <= dec_snk_i.stb when dec_snk_i.adr = c_WRF_DATA or
                                            dec_snk_i.adr = c_WRF_OOB else
                          '0';
  wb_ram_master_o.we  <= dec_snk_i.we  when dec_snk_i.adr = c_WRF_DATA or
                                            dec_snk_i.adr = c_WRF_OOB else
                          '0';
  wb_ram_master_o.adr <=  std_logic_vector(to_unsigned(c_words_frame * s_hot_slot + 
                                           to_integer(s_cnt_words), c_wishbone_address_width))
                          when dec_snk_i.adr = c_WRF_DATA else
                          std_logic_vector(to_unsigned(c_words_frame * s_hot_slot + 
                                           to_integer(s_cnt_ts), c_wishbone_address_width))
                          when dec_snk_i.adr = c_WRF_OOB  else
                          (others => '0');
  
  cnt_proc : process(clk_i)
    variable v_new_frame      : std_logic;
    variable v_getting_frame  : std_logic;
    variable v_getting_ts     : std_logic;
    variable v_check_overrun  : std_logic;
    --variable v_w_slot         : unsigned(f_log2_size(c_num_frames_ram)-1 downto 0);
    variable v_w_slot         : integer range 0 to c_num_frames_ram-1;

  begin
    
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_cnt_words       <= (others => '0');
        s_cnt_ts          <= (others => '0');
        s_cnt_words_last  <= (others => '0');
        s_new_frame_d0    <= '0';
        s_cnt_overrun     <= (others => '0');
        s_cnt_words_ram   <= (others => '0');
        --s_hot_slot        <= (others => '0');
        s_hot_slot        <= 0;
        s_cnt_frames_ts   <= (others => '0');
        s_write_slot      <= (others => '0'); 
        s_freed_slot      <= (others => '0'); 
      else
        if dec_snk_i.adr = "00" then
          s_new_frame_d0 <= dec_snk_i.stb;
        else
          s_new_frame_d0 <= '0';
        end if;        

        if s_new_frame_d0 = '1' and dec_snk_i.stb = '0' and dec_snk_i.adr = c_WRF_DATA then
           v_new_frame := '1';
        else
           v_new_frame := '0';
        end if;
         
        if dec_snk_i.cyc = '1' and dec_snk_i.stb = '1' and dec_snk_i.adr = c_WRF_DATA then
           v_getting_frame := '1';
        else
           v_getting_frame := '0';
        end if;
        
        if dec_snk_i.cyc = '1' and dec_snk_i.stb = '1' and dec_snk_i.adr = c_WRF_OOB then
           v_getting_ts := '1';
        else
           v_getting_ts := '0';
        end if;

        -- write slot in circular buffer
        if v_new_frame = '1' then
           if (s_hot_slot + 1) < c_num_frames_ram then
              v_w_slot := s_hot_slot + 1;
          else
              v_w_slot := 0;
           end if;
           s_hot_slot    <= v_w_slot;
        else
           s_hot_slot    <= s_hot_slot;
        end if;
        
        --  slot in circular buffer
        if v_new_frame = '1' and s_read_slot = '0'  then --write
           s_write_slot(v_w_slot) <= '1';

           if s_write_slot(v_w_slot) = '1' then
              v_check_overrun := '1';
           end if;
        elsif v_new_frame = '1'and s_read_slot = '1'  then --read/write
           s_write_slot(v_w_slot) <= '1';

           if v_w_slot /= s_r_slot then -- if just finish to read!
             s_write_slot(s_r_slot) <= '0';
           end if;
        elsif s_read_slot = '1'  then     --read
             s_write_slot(s_r_slot) <= '0';
        end if;

        -- check overrun circular buffer
        if v_check_overrun = '1'  then
           s_cnt_overrun <= s_cnt_overrun + 1;
        else
           s_cnt_overrun <= s_cnt_overrun;
        end if;

        -- counts the number of words in memory for a frame
        if v_getting_frame = '1' and wb_ram_master_i.stall = '0' then
          s_cnt_words       <= s_cnt_words + 1;
          s_cnt_words_last  <= s_cnt_words_last;
        elsif v_getting_frame = '1' and wb_ram_master_i.stall = '1' then
          s_cnt_words       <= s_cnt_words;
          s_cnt_words_last  <= s_cnt_words_last;
        else
          s_cnt_words     <= (others => '0');
          if s_cnt_words /= 0 then
            s_cnt_words_last  <= s_cnt_words;
          end if;
        end if;
        
        -- copy the Rx TS at the end of the frame
        if v_getting_ts = '1' and wb_ram_master_i.stall = '0' then
          s_cnt_ts  <= s_cnt_ts + 1;
        elsif v_getting_ts = '1' and wb_ram_master_i.stall = '1' then
          s_cnt_ts  <= s_cnt_ts;
        else
          if v_getting_frame = '1' then
            if s_cnt_words(0) = '1' then -- looking for the next even address
              s_cnt_ts  <= s_cnt_words;
            else
              s_cnt_ts  <= s_cnt_words + 1;
            end if;
            -- s_cnt_frames_ts  <= s_hot_slot;
          end if;
        end if;

      end if;
    end if;
  end process;
  
  wb_slave_if : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        wb_fwb_slave_o.ack  <= '0';
        wb_fwb_slave_o.dat  <= (others=>'0');
        wb_fwb_slave_o.stall<= '0';
        s_rst_words_ram     <= '0';
        s_read_slot         <= '0';
        s_rst_overrun       <= '0';
        s_r_slot            <= 0;
      else
        wb_fwb_slave_o.ack <= wb_fwb_slave_i.cyc and wb_fwb_slave_i.stb;

        if wb_fwb_slave_i.cyc = '1' and wb_fwb_slave_i.stb = '1' then
          case wb_fwb_slave_i.adr(5 downto 2) is
            when  "0000"  => -- slots/frames in ram   0x0
              wb_fwb_slave_o.dat  <= std_logic_vector(to_unsigned(c_num_frames_ram, c_wishbone_data_width));
              s_rst_words_ram     <= '1';
              s_read_slot         <= '0';
              s_rst_overrun       <= '0';
            when  "0001"  => -- slot was read and the slot can be freed
              if wb_fwb_slave_i.we = '1' then
                s_read_slot       <= '1';
                s_r_slot          <= to_integer(unsigned(wb_fwb_slave_i.dat(f_log2_size(c_num_frames_ram)-1 downto 0)));
              else              
                wb_fwb_slave_o.dat  <= std_logic_vector(resize(s_write_slot, c_wishbone_data_width));
                s_rst_words_ram     <= '0';
                s_rst_overrun       <= '0';
              end if;
            when  "0010"  => -- 16 bit words per frame  0x8
              wb_fwb_slave_o.dat  <= std_logic_vector(resize(s_cnt_words_last, c_wishbone_data_width));
              s_rst_words_ram     <= '0';
              s_read_slot         <= '0';
              s_rst_overrun       <= '0';
            when  "0011"  => -- Overflow in ram         0xC
              wb_fwb_slave_o.dat  <= std_logic_vector(resize(s_cnt_overrun, c_wishbone_data_width));
              s_rst_words_ram     <= '0';
              s_read_slot         <= '0';
              s_rst_overrun       <= '1';
            when  "0100"  => -- words per frame wb bus  0x10
              wb_fwb_slave_o.dat  <= std_logic_vector(to_unsigned(c_words_frame_wb, c_wishbone_data_width));
              s_rst_words_ram     <= '0';
              s_read_slot         <= '0';
              s_rst_overrun       <= '0';
            when others   => --                         0x14
              wb_fwb_slave_o.dat  <= x"DEADDA7A";
              s_rst_words_ram     <= '0';
              s_read_slot         <= '0';
              s_rst_overrun       <= '0';
          end case;
        else
          s_rst_words_ram   <= '0';              
          s_read_slot       <= '0';
          s_rst_overrun     <= '0';
        end if;
      end if;
    end if;
  end process;
end rtl;
