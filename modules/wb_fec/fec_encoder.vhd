library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.fec_pkg.all;

entity fec_encoder is
  generic(
    g_vlan  : boolean := false;
      )
  port(
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    ctrl_reg_i  : in  t_fec_ctrl_reg;
    stat_reg_o  : out t_fec_enc_stat_reg;
    enc_src_o   : out t_wrf_source_out;
    enc_src_i   : in  t_wrf_source_in;
    enc_snk_i   : in  t_wrf_sink_in;
    enc_snk_o   : out t_wrf_sink_out);
end fec_encoder;

architecture rtl of fec_encoder is
  -- 6 bytes mac x 2, 4 bytes vlan, 2 bytes ethertype = 18 bytes, 144 bits
  generate if eth_header_vlan_y then
    constant c_eth_header_l integer := 144;
  end generate;

  generate if not eth_header_vlan_n then
    constant c_eth_header_l integer := 132;
  end generate;

  constant c_fab_width  integer := 16;
  constant c_eth_frame  integer := 1500*8;
  constant c_head_cnt   integer := c_eth_header_l / c_fab_width;

  signal s_eth_payload  std_logic_vector(c_eth_payload -1 downto 0);
  signal s_eth_header   std_logic_vector(c_eth_header_l - 1 downto 0);
  signal s_fec_header   std_logic_vector(c_fec_eth_header_l -1 downto 0);

  ram?

  signal s_new_frame        std_logic;
  signal s_getting_frame    std_logic;
  signal s_encoding         std_logic;
  signal s_start_encd       std_logic;

begin

  stat_reg_o <= c_stat_enc_reg_default  

  -- fabric interface
  if dec_snk_i.cyc = '1' and dec_snk_i.stb = '1' and dec_snk_i.adr = c_WRF_STATUS then
    s_new_frame <= '1';
  else 
    s_new_frame <= '0';
  end if;

  if dec_snk_i.cyc = '1' and dec_snk_i.stb = '1' and dec_snk_i.adr = c_WRF_DATA then
    s_getting_frame <= '1';
  else
    s_getting_frame <= '0';
  end if;
  
  -- outputs
  enc_snk_o.rty   <= '0';
  enc_snk_o.err   <= '0';
  enc_snk_o.ack   <= enc_snk_i.cyc and enc_snk_i.stb;
  enc_snk_o.stall <= enc_src_i.stall when s_encoding = '0' and s_fec_header_en = '1' else
                     '1'             when s_encoding = '0' and s_fec_header_en = '0' else
                     '1'             when s_encoding = '1';

  enc_src_o.adr <= c_WRF_STATUS when s_fec_streamer = ETH_HEADER else --wrong!
                   c_WRF_DATA;

  enc_src_o.cyc <= '1' when s_fec_streamer /= IDLE else
                   '0' when s_fec_streamer  = IDLE;

  enc_src_o.stb <= '1' when s_fec_streamer /= IDLE  else
                   '0' when s_fec_streamer  = IDLE;

  enc_src_o.sel <= c_sel;
  enc_src_o.we  <= '1';

  --enc_src_i.ack

  -- creates the fec header
  FEC_HEADER : fec_header_generator
  port map (      
      clk_i       => clk_i,
      rst_n_i     => rst_n_i,
      fec_header  => s_fec_header,
      ready       => s_fec_header_en,
      ctrl_reg    => ctrl_reg_i);

  -- If systematic the original packet is also sent
  -- if not systematic, only encoded info is sent
  FEC_SYSTEMATIC : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_start_encod     <= '0';
        s_fec_systematic  <= IDLE;
      else
        case s_fec_systematic is 
          when IDLE     =>
            if s_new_frame = '1' and s_systematic = '1' then
              s_fec_systematic <= TX_FRAME;
              s_start_encod    <= '0';
            if s_new_frame = '1' and s_systematic = '0' then
              s_start_encod    <= '1';
              s_fec_systematic <= IDLE;
            else
              s_start_encod    <= '0';
              s_fec_systematic <= IDLE;
            end;
          when TX_FRAME =>
            if s_getting_frame = '1' then
              s_start_encod    <= '0';
            elsif s_getting_frame = '0' then
              s_fec_systematic <= IDLE;
              s_start_encod    <= '1';
            end;
          when others   =>
        end case;
      end if;
    end if;
  end process;

  -- it controls the states of the FSM and the rate of the code
  FEC_FSM : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_frame_ptr     <= 0;
        s_fec_streamer  <= IDLE;
        s_rate_cntr     <= 0;
      else

        if s_fec_streamer /= IDLE or s_fec_streamer /= FEC_HEADER then
          if enc_src_i.stall = '0' and (s_encoding = '1' or s_getting_frame = '1') then
            s_frame_ptr   <= s_frame_ptr + 1;
          else
            s_frame_ptr   <= s_frame_ptr;
          end if;
        else
            s_frame_ptr   <= 0;
        end if;

        case s_fec_streamer is
          when IDLE         =>
            if s_new_frame = '1' and s_encoding = '0' then
              s_fec_streamer  <= ETH_HEADER;
              s_encoding      <= '0';
            elsif s_encoding = '1' then
              s_fec_streamer  <= ETH_HEADER;              
              if s_fec_rate = s_rate_ctnr then            
                s_rate_cntr   <= 0;
                s_encoding    <= '0';
              else
                s_encoding    <= '1';
                s_rate_cntr   <= s_rate_cntr + 1;
              end if;
            elsif s_start_encod = '1' and s_encoding = '0' then
              s_fec_streamer  <= ETH_HEADER;
              s_encoding      <= '1';
              s_rate_cntr     <= s_rate_cntr + 1;
            else
              s_fec_streamer  <= IDLE;
              s_encoding      <= '0';
            end if;
          when ETH_HEADER   =>            
            if s_frame_ptr = ETH_HEADER_CNTR then 
                s_fec_streamer  <= FEC_HEADER;
            end if;
          when FEC_HEADER   =>            
            if s_frame_ptr = FEC_HEADER_CNTR then          
              if s_encoding = '1' then
                s_fec_streamer  <= ETH_PAYLOAD;
              else
                s_fec_streamer  <= FEC_PAYLOAD;
              end if;
            end if;
          when ETH_PAYLOAD  =>
            if s_frame_ptr = ETH_PAYLOAD_CNTR then
             s_fec_streamer  <= IDLE;
            end if;
          when FEC_PAYLOAD  =>
            if s_frame_ptr = FEC_PAYLOAD_CNTR then
             s_fec_streamer  <= IDLE;
            end if;
          when others       =>
        end case;
      end if;
    end if;
  end process;


  FEC_STREAMING : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_eth_header    <= (others => '0');
      else              
        case s_fec_streamer is
          when IDLE         =>
          when ETH_HEADER   =>
            if s_rate_cntr = 0 then -- first frame sent
              enc_src_o.dat <= enc_snk_i.dat;
              s_eth_header  <= s_eth_header(c_head_cnt-16 downto 0) & enc_snk_i.dat;
            else
              s_eth_header  <= s_eth_header(c_head_cnt-16 downto 0) & s_eth_header(c_head_cnt downto c_head_cnt - 16);
              enc_src_o.dat <= s_eth_header(c_head_cnt downto c_head_cnt - 16);
            end if;
          when FEC_HEADER   =>
            enc_src_o.dat   <= s_fec_header(moving range);
          when ETH_PAYLOAD  =>            
            s_eth_payload   <= s_eth_payload(c_eth_frame-16 downto 0) & enc_snk_i.dat;
            enc_src_o.dat   <= enc_snk_i.dat;
          when FEC_PAYLOAD  =>
            enc_src_o.dat   <= s_eth_payload * matrix;
          when others       =>
        end case;
      end if;
    end if;
  end process;  
end rtl;
