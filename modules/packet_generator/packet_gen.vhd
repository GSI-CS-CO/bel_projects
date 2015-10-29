library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.packet_pkg.all;
use work.endpoint_pkg.all;

entity packet_gen is
port (
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    ctrl_reg_i      : in  t_pg_ctrl_reg;
    stat_reg_o      : out t_pg_stat_reg;
    pg_timestamps_i : in  t_txtsu_timestamp;
    pg_tm_tai_i     : in  std_logic_vector(39 downto 0);
    pg_tm_cycle_i   : in  std_logic_vector(27 downto 0);
    pg_src_i        : in  t_wrf_source_in;
    pg_src_o        : out t_wrf_source_out);

end packet_gen;    

architecture rtl of packet_gen is

  ------------ fsm start/stop and frame gen
  signal s_pg_fsm         : t_pg_fsm := IDLE;
  signal s_frame_fsm      : t_frame_fsm := INIT_HDR;
  -- packet gen reg
  signal s_pg_state       : t_pg_state := c_pg_state_default;
  -- wb reg
  signal s_ctrl_reg       : t_pg_ctrl_reg := c_pg_ctrl_default;
  signal s_stat_reg       : t_pg_stat_reg := c_pg_stat_default;

  signal s_frame_gen         : integer := 0;
  signal s_pay_load_reg      : t_wrf_bus := (others => '0');
  signal s_hdr_reg           : t_eth_hdr := (others => '0');
  signal s_rate_con          : integer := 0;
  signal s_rate_dis          : integer := 62500000;
  signal s_hdr_cntr          : integer := 0;
  signal s_load_cntr         : integer := 0;   
  signal s_rate_max          : integer := 0;
  signal s_load_max          : integer := 0;
  signal s_rate_wb           : std_logic := '1';
  signal s_old_rate          : integer := 0;
  signal s_random_seq        : integer := 0; 
  signal s_random_ethtype    : integer := 0; 
  signal s_random_rate       : integer := 0; 
  signal s_random_payload    : integer := 0; 
  signal s_ether_hdr         : t_eth_frame_header;
  signal s_random_seq_o      : std_logic_vector(31 downto 0);
  signal s_random_ethtype_o  : std_logic_vector(2 downto 0);
  signal s_random_rate_o     : std_logic_vector(25 downto 0);
  signal s_random_payload_o  : std_logic_vector(10 downto 0);
  signal s_first             : integer := 0;
  signal s_pkg_cntr          : integer := 0; 
  signal s_con_count         : integer := 0; 
  signal s_dis_count         : integer := 0; 
  signal s_con_time          : integer := 0; 
  signal s_dis_time          : integer := 0; 
  signal s_oob_cnt           : integer := 0;
  signal s_timestamps        : std_logic_vector(31 downto 0);
  signal s_tm_tai            : std_logic_vector(39 downto 0);

  signal s_tm_cycle_i        : std_logic_vector(27 downto 0);
  signal s_tm_tai_i          : std_logic_vector(39 downto 0);

  signal s_rising_stb        : std_logic := '0';

  signal s_count_num         : integer;
  signal s_count_num_i       : std_logic_vector(31 downto 0);
  signal s_payload_length_i  : std_logic_vector(31 downto 0);

  signal s_gen_con_packet    : std_logic;
  signal s_gen_dis_packet    : std_logic;

  begin
  -- default mac add  
  s_ether_hdr.eth_src_addr <= x"ffffffffffff";

  -- Start/Stop fsm Packet Generator
  pg_fsm : process(clk_i)
    begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_pg_fsm <= IDLE;
        s_pg_state.gen_con_packet <= '0';
        s_pg_state.gen_dis_packet <= '0';
        s_pg_state.halt       <= '0';
      else
        case s_pg_fsm is
          when IDLE =>
            -- package generator in continuous pattern  1 1 1 1 1 1
            if( s_ctrl_reg.en_pg = '1'and s_ctrl_reg.mode = "00") then 
              s_pg_fsm <= CONTINUOUS;
            else
              -- package generator in discrete pattern 111    111    111
              if( s_ctrl_reg.en_pg = '1'and s_ctrl_reg.mode = "01") then 
                s_pg_fsm <= DISCRETE;
              else
                s_pg_fsm <= IDLE;
              end if;
            end if;
            s_pg_state.gen_con_packet <= '0';
            s_pg_state.gen_dis_packet <= '0';
            s_pg_state.halt           <= '0';
          when CONTINUOUS =>
            s_pg_state.new_start <= '0';
            -- stop packet generator
            if( s_ctrl_reg.en_pg = '0') then 
              s_pg_fsm <= CON_HALTING;                     
            else
              -- switch to discrete mode
              if( s_ctrl_reg.mode = "01") then
                -- gaurantee a whole packet is transfered before switch to discrete mode
                if(s_pg_state.cyc_ended = '1') then
                  s_pg_state.new_start <= '1';
                  s_pg_fsm <= DISCRETE;
                else
                  s_pg_fsm <= CONTINUOUS;
                end if;
              else
              -- continue continuous mode
                s_pg_fsm <= CONTINUOUS;
              end if;
            s_pg_state.gen_con_packet <= '1';
            s_pg_state.gen_dis_packet <= '0';
            s_pg_state.halt           <= '0';
            end if;
          when DISCRETE =>
          -- stop packet generator
            s_pg_state.new_start <= '0';
            if( s_ctrl_reg.en_pg = '0') then 
              s_pg_fsm <= DIS_HALTING;                     
            else
              -- switch to continuous mode
              if( s_ctrl_reg.mode = "00") then 
              -- gaurantee a whole packet is transfered before switch to continuous mode
                if(s_pg_state.cyc_ended = '1') then
                  s_pg_state.new_start <= '1';
                  s_pg_fsm <= CONTINUOUS;
                else
                  s_pg_fsm <= DISCRETE;
                end if;
              else
                -- continue continuous mode
                s_pg_fsm <= DISCRETE;
              end if;
              s_pg_state.gen_con_packet <= '0';
              s_pg_state.gen_dis_packet <= '1';
              s_pg_state.halt           <= '0';
            end if;
          when CON_HALTING =>
            -- gaurantee a whole packet is transfered before stop
            if(s_pg_state.cyc_ended = '1') then
              s_pg_state.new_start <= '1';
              s_pg_fsm <= IDLE;
              s_pg_state.gen_con_packet <= '0';
            else
              s_pg_fsm <= CON_HALTING;
              s_pg_state.gen_con_packet <= '1';
            end if; 
            s_pg_state.halt <= '1';
          when DIS_HALTING =>
            if(s_pg_state.cyc_ended = '1') then
              s_pg_state.new_start <= '1';
              s_pg_fsm <= IDLE;
              s_pg_state.gen_dis_packet <= '0';
            else
              s_pg_fsm <= DIS_HALTING;
              s_pg_state.gen_dis_packet <= '1';
            end if; 
            s_pg_state.halt <= '1';
          end case;
        end if;
      end if;   
  end process;



  -- Frame Generation
  frame_gen : process(clk_i)
    variable v_payload_length  : integer;
  begin
    if rising_edge(clk_i) then       
      if rst_n_i = '0' then
        s_frame_fsm          <= INIT_HDR;
        s_hdr_reg            <= (others => '0');
        s_pay_load_reg       <= (others => '0');
        s_pg_state.cyc_ended    <= '1';
        s_hdr_cntr              <= 0;
        s_load_cntr             <= 0;
        s_rate_con              <= 0;
        s_rate_dis              <= 0;
        s_pkg_cntr         	    <= 0;
      else
        --rising edge detection for the timestamp stb
        s_rising_stb <= pg_timestamps_i.stb;
        if (s_rising_stb = '0' and pg_timestamps_i.stb = '1') then
          s_timestamps <= pg_timestamps_i.tsval;
          s_tm_tai     <= pg_tm_tai_i;
        end if;

        s_gen_con_packet <= s_pg_state.gen_con_packet;
        s_gen_dis_packet <= s_pg_state.gen_dis_packet;

        if (s_gen_con_packet = '0' and s_pg_state.gen_con_packet = '1') or 
          (s_gen_dis_packet = '0' and s_pg_state.gen_dis_packet = '1') then
          s_count_num     <= 0;
        elsif s_frame_fsm = INIT_HDR and (s_pg_state.gen_con_packet = '1' or  s_pg_state.gen_dis_packet = '1' ) then
          s_count_num     <= s_count_num + 1;
          s_count_num_i   <=std_logic_vector(to_unsigned(s_count_num,32));
        end if;


        if s_rate_wb = '1' and (s_pg_state.gen_con_packet = '1' or  s_pg_state.gen_dis_packet = '1' ) then
          s_rate_max <= to_integer (unsigned(s_ctrl_reg.rate));
        end if;

        if s_frame_fsm = INIT_HDR and (s_pg_state.gen_con_packet = '1' or s_pg_state.gen_dis_packet = '1' ) then
          s_frame_fsm      	         <= ETH_HDR;
          s_rate_wb                  <= '0';
          s_pg_state.cyc_ended       <= '0';
          s_ether_hdr. eth_des_addr  <= s_ctrl_reg.eth_hdr.eth_des_addr;
          s_ether_hdr. eth_etherType <= s_ctrl_reg.eth_hdr.eth_etherType;
          s_hdr_reg                  <= f_eth_hdr(s_ether_hdr);
          s_load_max                 <= to_integer(unsigned(s_ctrl_reg.payload));
          -- get time when Ether Header leave packet generator
          s_tm_tai_i                 <= pg_tm_tai_i;
          s_tm_cycle_i               <= pg_tm_cycle_i;
        end if;

        if s_frame_fsm = ETH_HDR and (s_pg_state.gen_con_packet = '1' or
          s_pg_state.gen_dis_packet = '1' ) then
          --transfer the ether header 6+6+2 bytes
          if s_hdr_cntr = c_hdr_l   then
            s_frame_fsm          <= PAY_LOAD;
            s_hdr_cntr           <= 0;                           
          else
            s_frame_fsm          <= ETH_HDR;
            --if pg_src_i.stall /= '1' then
            if pg_src_i.ack = '1' then
              s_hdr_reg          <= s_hdr_reg(s_hdr_reg'left -16 downto 0) & x"0000";
              s_hdr_cntr         <= s_hdr_cntr + 1;

            end if;
          end if;
        end if;             

        if s_frame_fsm = PAY_LOAD and (s_pg_state.gen_con_packet = '1' or s_pg_state.gen_dis_packet = '1' ) then

        --transfer the payload, payload length is decided by s_load_max
          if s_load_max = s_load_cntr then
            s_frame_fsm       <= OOB;
            s_oob_cnt         <= 0;								
            s_pay_load_reg    <= (others => '0');
            s_load_cntr       <= 0;
          else
            s_frame_fsm       <= PAY_LOAD;
            s_payload_length_i<=std_logic_vector(to_unsigned(s_load_max*2,32));
            CASE s_load_cntr IS
              when 0 => s_pay_load_reg      <= x"BEEF";
              when 1 => s_pay_load_reg      <= x"BEEF";		
              --produce UDP 17 0x11 for t  est
              when 2 => s_pay_load_reg      <= x"BEEF";
              when 3 => s_pay_load_reg      <= x"00" & s_ctrl_reg.udp;
              -- time when the packet goe  s through the WR core 
              when 4 => s_pay_load_reg      <= s_timestamps(31 downto 16);
              when 5 => s_pay_load_reg      <= s_timestamps(15 downto 0);
              when 6 => s_pay_load_reg      <=  x"00" & s_tm_tai(39
                downto 32);
              when 7 => s_pay_load_reg      <= s_tm_tai(31
                downto 16);
              when 8 => s_pay_load_reg      <= s_tm_tai(15
                downto 0);
              --time when the last packet   leaves the packet generator 
              when 10 => s_pay_load_reg     <= x"0" & s_tm_cycle_i(27
                downto 16);
              when 11 => s_pay_load_reg     <= s_tm_cycle_i(15
                downto 0);
              when 12 => s_pay_load_reg     <= x"00" &
              s_tm_tai_i(39 downto 32);
              when 13 => s_pay_load_reg     <= s_tm_tai_i(31
                downto 16);
              when 14 => s_pay_load_reg     <= s_tm_tai_i(15
                downto 0);
              when 16 => s_pay_load_reg     <= x"DEAD";
              when 17 => s_pay_load_reg     <= x"BEEF";
              --packet counter
              when 18 => s_pay_load_reg     <= s_count_num_i(31
                downto 16);
              when 19 => s_pay_load_reg     <= s_count_num_i(15
                downto 0);
              -- packet payload length
              when 20 => s_pay_load_reg     <= s_payload_length_i(31
                downto 16);
              when 21 => s_pay_load_reg     <= s_payload_length_i(15
                downto 0);
              when others => s_pay_load_reg <= (others => '0');
            END CASE;

            if pg_src_i.stall /= '1' then
              s_load_cntr      <= s_load_cntr + 1;
            end if;
          end if;
        end if;

        if s_frame_fsm = OOB and (s_pg_state.gen_con_packet = '1' or s_pg_state.gen_dis_packet = '1' ) then
          if s_oob_cnt = 1 then
            s_frame_fsm       <= IDLE;
          else
            s_oob_cnt <= s_oob_cnt + 1;
          end if;
        end if;

        if s_frame_fsm = IDLE and (s_pg_state.gen_con_packet = '1' or s_pg_state.gen_dis_packet = '1' ) then
          s_pay_load_reg       <= (others => '0');
          s_hdr_reg            <= (others => '0');
          s_pg_state.cyc_ended <= '1';
          --gaurantee change pattern 
          if s_pg_state.new_start = '1' then
            s_frame_fsm        <= INIT_HDR;
            s_rate_wb          <= '1';
          end if;
        end if;

        if s_pg_state.gen_con_packet = '1' then --continuous mode begin
          s_old_rate <= 0;
          --get the rate from wb

          if s_rate_max /= s_rate_con then
            case s_frame_fsm is
              when INIT_HDR =>

              when ETH_HDR =>

              when PAY_LOAD =>

              when OOB =>

              when IDLE    =>
                -- Clock cycle of IDLE=s_rate_max-1[Init header]-(6+6+2)/2[Ether header]-s_load_max/2[payload] 
                s_frame_fsm    <= IDLE;
            end case;
          s_rate_con           <= s_rate_con + 1;   
          else
            -- go back to INIT_HDR after one cycle= rate_max * 16ns
            s_rate_con          <= 0;  
            s_frame_fsm         <= INIT_HDR;
            s_rate_wb           <= '1';
          end if;
        else
          s_rate_con            <= 0;
        end if;--continuous mode end

        -- packet generator works on the discrete mode
        if s_pg_state.gen_dis_packet = '1' then --discrete mode begin
          if s_rate_wb = '1' then
            --get the rate from wb 
            s_rate_max <= to_integer (unsigned(s_ctrl_reg.rate));
          end if;

          if s_rate_dis /= 62500000 then
            case s_frame_fsm is
              when INIT_HDR =>
                s_pkg_cntr    <= s_pkg_cntr +1;
              when ETH_HDR =>
              when PAY_LOAD =>
              when OOB =>
              when IDLE    =>
              --judge whether the number of packet reaches to the rate requirement
              if (s_pkg_cntr * s_rate_max >= 62500000 ) then
              s_frame_fsm     <= IDLE;
              s_rate_wb       <= '1';
              else
              s_frame_fsm     <= INIT_HDR ;
              end if;
            end case;
            if s_rate_max /= s_old_rate then
              s_rate_dis <= 62500000;
            else
              s_rate_dis <= s_rate_dis + 1;   
            end if;
          else
            s_old_rate          <= s_rate_max;
            s_rate_dis        	<= 0;--configure rate
            s_pkg_cntr    			<= 0;
            s_frame_fsm   			<= INIT_HDR;
          end if;
        else
          s_rate_dis       		<= 0;
          s_pkg_cntr          <= 0;
        end if;--discrete mode end

        if s_pg_state.gen_dis_packet = '0' and s_pg_state.gen_con_packet = '0' then
          s_frame_fsm       <= INIT_HDR;
        end if;
      end if;
    end if;
  end process;

  ----- Fabric Interface
  -- Mux between header and payload
  with s_frame_fsm select

    pg_src_o.dat   <= s_pay_load_reg                      when PAY_LOAD,
    s_hdr_reg(s_hdr_reg'left downto s_hdr_reg'left - 15)  when ETH_HDR,
    c_WRF_OOB_TYPE_TX & x"aaa"                            when OOB,
    (others => '0')                                       when others;


    pg_src_o.cyc   <= '1' when ( s_frame_fsm = ETH_HDR or s_frame_fsm = PAY_LOAD 
      or s_frame_fsm = OOB ) else '0';
    pg_src_o.stb   <= '1' when ( s_frame_fsm = ETH_HDR or s_frame_fsm = PAY_LOAD 
      or s_frame_fsm = OOB ) else '0';

  with s_frame_fsm select

    pg_src_o.adr    <= c_WRF_DATA       when PAY_LOAD,
    c_WRF_OOB        when OOB,
    (others => '0')  when others;

    pg_src_o.we      <= '1';
    pg_src_o.sel     <= "11";

  -- WB Register Ctrl/Stat
  ctrl_stat_reg :  process(clk_i)
    begin
      if rising_edge(clk_i) then
        if rst_n_i = '0' then
          s_ctrl_reg  <= c_pg_ctrl_default;
          s_frame_gen <= 0;
        else
          s_ctrl_reg            <= ctrl_reg_i;
          stat_reg_o.frame_gen  <= std_logic_vector(to_unsigned(s_frame_gen,32));
          stat_reg_o.count      <= s_count_num_i;
        end if;
      end if;
    end process;

  end rtl;
