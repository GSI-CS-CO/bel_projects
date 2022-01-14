library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.ifk10_pkg.all;

entity ifactl is
  port (
    clk                : in std_logic;                       -- system clock
    sclr               : in std_logic;                       -- synchronous clear
    ifa_adr            : in std_logic_vector(7 downto 0);    -- IFK address
    fc_str             : in std_logic;                       -- function code strobe
    clr_after_rd       : in std_logic;                       -- function code strobe
    fc                 : in std_logic_vector(7 downto 0);    -- function code
    di                 : in std_logic_vector(15 downto 0);   -- data set value
    diw                : in std_logic_vector(15 downto 0);   -- data actual value
    sts                : in std_logic_vector(7 downto 0);    -- 8 Bit status from VG connector
    inrm               : in std_logic_vector(7 downto 0);    -- 8 Bit interrupt mask
    ifa_ctrl           : in std_logic_vector(7 downto 0);    -- IFA internal control status
  --  ifp_id             : in std_logic_vector(7 downto 0);    -- IF-Piggy identification code
  --  ifp_in             : in std_logic;                       -- IF-Piggy input
    --ifa_epld_vers      : in std_logic_vector(7 downto 0);    -- IFA EPLD version number
    i2c_data           : in std_logic_vector(12 downto 0);   -- data i2c bus
    fwl_data_sts       : in std_logic_vector(15 downto 0);   -- data or status from IFA firmware loader
    me_vw_err          : in std_logic_vector(15 downto 0);   -- error counter ME-VW
    me_data_err        : in std_logic_vector(15 downto 0);   -- error counter ME-Data
    if_mode            : in std_logic_vector(15 downto 0);   -- mode of interface card
    ifa_id             : in std_logic_vector(7 downto 0);

    ifa_sd_mux_sel     : out std_logic_vector(3 downto 0);   -- mux select for status and data of the IFA
    ifa_sd_mux         : out std_logic_vector(15 downto 0);  -- mux output for status and data of the IFA
    vg_data            : out std_logic_vector(15 downto 0);  -- data output in IFA-Mode to VG connector
    send_str           : out std_logic;                      -- send strobe for EE-FlipFlop of the sender 6408
    fc_ext             : out std_logic;                      -- signals external data access
    ifa_fc_int         : out std_logic;                      -- function codes for IFA
    wr_i2c             : out std_logic;                      -- write data of the i2c bus
    wr_irm             : out std_logic;                      -- write interrupt mask
    wr_fwl_data        : out std_logic;                      -- write data to IFA firmware loader
    wr_fwl_ctrl        : out std_logic;                      -- write ctrl to IFA firmware loader
    rd_fwl_data        : out std_logic;                      -- read data from IFA firmware loader
    rd_fwl_sts         : out std_logic;                      -- read status from IFA firmware loader
    rd_me_vw_err       : out std_logic;                      -- error counter for ME-VW-Error
    rd_me_data_err     : out std_logic;                      -- error counter for ME-Data-Error
    wr_clr_me_vw_err   : out std_logic;                      -- clear ME-VR error counter
    wr_clr_me_data_err : out std_logic;                      -- clear ME-Data error counter
    IF_Mode_Reg        : out std_logic_vector(15 downto 0);  -- ifa mode register -- FC=0x60
  --  ifp_led            : out std_logic_vector(15 downto 0);  -- IF-Piggy 'FG 380.751' LED port
  --  ifp_led_out        : out std_logic_vector(15 downto 0);  -- IF-Piggy 'FG 380.751' out/leds
    broad_en           : out std_logic;                      -- flag for broadcast enable
    reset_cmd          : out std_logic;                      -- reset command (fc = 0x01)
    res_vw_err         : out std_logic);                     -- reset VW error
end entity ifactl;

architecture arch of ifactl is

constant c_rd_irm         : std_logic_vector(7 downto 0) := x"c9";
constant c_rd_cntrl       : std_logic_vector(7 downto 0) := x"ca";
constant c_rd_id          : std_logic_vector(7 downto 0) := x"cc";
constant c_rd_vers        : std_logic_vector(7 downto 0) := x"cd";
constant c_rd_buildid     : std_logic_vector(7 downto 0) := x"ce";
constant c_rst_buildid    : std_logic_vector(7 downto 0) := x"cf";
constant c_rd_echo        : std_logic_vector(7 downto 0) := x"89";
--380.122
--constant c_rd_1x2_Data    : std_logic_vector(7 downto 0) := x"90";

constant c_rd_if_mode     : std_logic_vector(7 downto 0) := x"97";
constant c_rd_i2c         : std_logic_vector(7 downto 0) := x"98";
constant c_rd_fwl_data    : std_logic_vector(7 downto 0) := x"9c";   -- data for firmware loader of the IFA
constant c_rd_fwl_sts     : std_logic_vector(7 downto 0) := x"9d";   -- status of the IFA firmware loader
constant c_rd_me_vw_err   : std_logic_vector(7 downto 0) := x"d3";   -- error counter ME-VW
constant c_rd_me_data_err : std_logic_vector(7 downto 0) := x"d4";   -- error counter ME-Data
--constant c_rd_ifp_led     : std_logic_vector(7 downto 0) := x"9e"; -- IF-Piggy 'FG 380.751' LED port
--constant c_rd_ifp_io      : std_logic_vector(7 downto 0) := x"9f"; -- IF-Piggy 'FG 380.751' Out/LEDs/In
--constant c_rd_ifp_id      : std_logic_vector(7 downto 0) := x"8e"; -- IF-Piggy-ID

constant c_wr_sw1         : std_logic_vector(7 downto 0) := x"06";
constant c_wr_irm         : std_logic_vector(7 downto 0) := x"12";
constant c_wr_echo        : std_logic_vector(7 downto 0) := x"13";
constant c_wr_if_mode     : std_logic_vector(7 downto 0) := x"60";
constant c_wr_fwl_data    : std_logic_vector(7 downto 0) := x"65"; -- data for IFA firmware loader
constant c_wr_fwl_ctrl    : std_logic_vector(7 downto 0) := x"66"; -- ctrl for IFA firmware loader
constant c_wr_i2c         : std_logic_vector(7 downto 0) := x"61";
constant c_wr_ifp_led     : std_logic_vector(7 downto 0) := x"69"; -- IF-Piggy 'FG 380.751' LED-Port
constant c_wr_ifp_out     : std_logic_vector(7 downto 0) := x"6a"; -- IF-Piggy 'FG 380.751' Out/LED's/In

constant c_reset_cmd      : std_logic_vector(7 downto 0) := x"01";
constant c_res_vw_err     : std_logic_vector(7 downto 0) := x"7d";
constant c_res_broadcast  : std_logic_vector(7 downto 0) := x"7e";
constant c_set_broadcast  : std_logic_vector(7 downto 0) := x"7f";


signal s_fc_int             : std_logic;
signal s_fc_ext             : std_logic;
signal s_rd_irm             : std_logic;
signal s_rd_cntrl           : std_logic;
signal s_rd_id              : std_logic;
signal s_rd_vers            : std_logic;
signal s_rd_buildid         : std_logic;
signal s_rst_buildid        : std_logic;
signal s_rd_sts             : std_logic;
signal s_rd_echo            : std_logic;
signal s_send_str           : std_logic;
signal s_wr_irm             : std_logic;
signal s_res_vw_err         : std_logic;
signal s_reset_cmd          : std_logic;
signal s_rd_i2c             : std_logic;
signal s_wr_i2c             : std_logic;
signal s_wr_fwl_data        : std_logic;
signal s_wr_fwl_ctrl        : std_logic;
signal s_rd_fwl_data        : std_logic;
signal s_rd_fwl_sts         : std_logic;

--signal c_rd_1x2_D            : std_logic;
--signal s_wr_ifp_led         : std_logic;
--signal s_wr_ifp_out         : std_logic;
--signal s_rd_ifp_led         : std_logic;
--signal s_rd_ifp_io          : std_logic;
--signal s_rd_ifp_id          : std_logic;
signal s_rd_if_mode         : std_logic;
signal s_broad_en           : std_logic;
signal s_ed                 : std_logic_vector(15 downto 0);
--signal s_ifp_led            : std_logic_vector(15 downto 0);
--signal s_ifp_out            : std_logic_vector(15 downto 0);
signal s_vg_data            : std_logic_vector(15 downto 0);

signal r_IF_Mode_Reg       : std_logic_vector(15 downto 0);

signal s_rd_me_vw_err       : std_logic;
signal s_rd_me_data_err     : std_logic;
signal s_wr_clr_me_vw_err   : std_logic;
signal s_wr_clr_me_data_err : std_logic;

signal build_id_out         : std_logic_vector(15 downto 0);


begin

IF_Mode_Reg<= r_IF_Mode_Reg;

--set sub-Modes by Mil-Command
setIFMode: process(clk,sclr,FC_Str,DI,FC)

 begin
  if rising_edge(clk) then
    if sclr='1'then
      r_IF_Mode_Reg <= ( others =>'0');
    else
      if (C_WR_IF_Mode = fc) and  (fc_str='1') then
         r_IF_Mode_Reg <= DI;  --set sub mode
      end if;
   end if;
  end if;

end process;


  fc_int: process (clk, sclr)
  begin
    if sclr = '1' then
      s_fc_int <= '0';
    elsif rising_edge(clk) then
      s_fc_int <= '0';
      case (fc) is

        when c_rd_irm =>
          s_fc_int <= '1';
        when c_rd_cntrl =>
          s_fc_int <= '1';
        when c_rd_id =>
          s_fc_int <= '1';
        when c_rd_vers =>
          s_fc_int <= '1';
        when c_rd_buildid =>
          s_fc_int <= '1';
        when c_rst_buildid =>
          s_fc_int <= '1';
        when c_rd_echo =>
          s_fc_int <= '1';
        when c_rd_if_mode =>
          s_fc_int <= '1';
        when c_rd_i2c =>
          s_fc_int <= '1';
        when c_rd_fwl_data =>
          s_fc_int <= '1';
        when c_rd_fwl_sts =>
          s_fc_int <= '1';
        when c_rd_me_vw_err =>
          s_fc_int <= '1';
        when c_rd_me_data_err =>
          s_fc_int <= '1';
      --  when c_rd_ifp_led =>
       --   s_fc_int <= '1';
     --   when c_rd_ifp_io =>
      --    s_fc_int <= '1';
      --  when c_rd_ifp_id =>
      --    s_fc_int <= '1';
        when c_wr_irm =>
          s_fc_int <= '1';
        when c_wr_echo =>
          s_fc_int <= '1';
        when c_wr_if_mode =>
          s_fc_int <= '1';
        when c_wr_i2c =>
          s_fc_int <= '1';
        when c_wr_fwl_data =>
          s_fc_int <= '1';
        when c_wr_fwl_ctrl =>
          s_fc_int <= '1';
        when c_wr_ifp_led =>
          s_fc_int <= '1';
        when c_wr_ifp_out =>
          s_fc_int <= '1';
        when c_res_vw_err =>
          s_fc_int <= '1';
        when c_res_broadcast =>
          s_fc_int <= '1';
        when c_set_broadcast =>
          s_fc_int <= '1';
        when others =>
          s_fc_int <= '0';
      end case;
    end if;
  end process;

  fc_external: process (clk, sclr)
  begin
    if sclr = '1' then
      s_fc_ext <= '0';
    elsif rising_edge(clk) then
      s_fc_ext <= '0';
      if s_fc_int = '0' and fc_str = '1' then
        s_fc_ext <= '1';
      end if;
    end if;
  end process;

  rd_status: process (clk, sclr)
  begin
    if sclr = '1' then
      s_rd_irm      <= '0';
      s_rd_cntrl    <= '0';
      s_rd_id       <= '0';
      s_rd_vers     <= '0';
      s_rd_buildid  <= '0';
      s_rst_buildid <= '0';
      s_rd_sts      <= '0';
      s_rd_echo     <= '0';
      s_rd_if_mode  <= '0';

    elsif rising_edge(clk) then
      s_rd_irm      <= '0';
      s_rd_cntrl    <= '0';
      s_rd_id       <= '0';
      s_rd_vers     <= '0';
      s_rd_buildid  <= '0';
      s_rst_buildid <= '0';
      s_rd_sts      <= '0';
      s_rd_echo     <= '0';
      s_rd_if_mode  <= '0';

      if fc_str = '1' then
        case (fc) is
          when c_rd_irm =>
            s_rd_irm <= '1';

          when c_rd_cntrl =>
            s_rd_cntrl <= '1';

          when c_rd_id =>
            s_rd_id <= '1';

          when c_rd_vers =>
            s_rd_vers <= '1';

          when c_rd_buildid =>
            s_rd_buildid <= '1';

          when c_rst_buildid =>
            s_rst_buildid <= '1';

         when c_rd_if_mode =>
            s_rd_if_mode <= '1';

          -- c0 - c8 , status read from vg backplane connector
          when x"c0" =>
            s_rd_sts <= '1';
          when x"c1" =>
            s_rd_sts <= '1';
          when x"c2" =>
            s_rd_sts <= '1';
          when x"c3" =>
            s_rd_sts <= '1';
          when x"c4" =>
            s_rd_sts <= '1';
          when x"c5" =>
            s_rd_sts <= '1';
          when x"c6" =>
            s_rd_sts <= '1';
          when x"c7" =>
            s_rd_sts <= '1';
          when x"c8" =>
            s_rd_sts <= '1';



          when c_rd_echo =>
            s_rd_echo <= '1';


          when others =>

        end case;
      end if;
    end if;
  end process;

  send_strobe: process (clk, sclr)
  begin
    if sclr = '1' then
      s_send_str <= '0';
    elsif rising_edge(clk) then
      s_send_str <= '0';
      if fc(7) = '1' and fc_str = '1' then
        s_send_str <= '1';
      end if;
    end if;
  end process;


  -- decode fc for writing, i2c and firmware loader
  decode_fc: process (clk, sclr)
  begin
    if sclr = '1' then
      s_wr_irm      <= '0';
      s_res_vw_err  <= '0';
      s_reset_cmd   <= '0';
      s_rd_i2c      <= '0';
      s_wr_i2c      <= '0';
      s_wr_fwl_ctrl <= '0';
      s_wr_fwl_data <= '0';
      s_rd_fwl_sts  <= '0';
      s_rd_fwl_data <= '0';
    elsif rising_edge(clk) then
      s_wr_irm      <= '0';
      s_res_vw_err  <= '0';
      s_reset_cmd   <= '0';
      s_rd_i2c      <= '0';
      s_wr_i2c      <= '0';
      s_wr_fwl_ctrl <= '0';
      s_wr_fwl_data <= '0';
      s_rd_fwl_sts  <= '0';
      s_rd_fwl_data <= '0';

      if fc_str = '1' then
        case (fc) is
          when c_wr_irm =>
            s_wr_irm  <= '1';

          when c_res_vw_err =>
            s_res_vw_err <= '1';

          when c_reset_cmd =>
            s_reset_cmd <= '1';

          when c_rd_i2c =>
            s_rd_i2c <= '1';

          when c_wr_i2c =>
            s_wr_i2c <= '1';

          when c_wr_fwl_data =>
            s_wr_fwl_data <= '1';

          when c_wr_fwl_ctrl =>
            s_wr_fwl_ctrl <= '1';

          when c_rd_fwl_data =>
            s_rd_fwl_data <= '1';

          when c_rd_fwl_sts =>
            s_rd_fwl_sts <= '1';
          when others =>

        end case;
      end if;
    end if;
  end process;


  me_err_cnt: process (clk, sclr)
  begin
    if sclr = '1' then
      s_rd_me_vw_err        <= '0';
      s_rd_me_data_err      <= '0';
      s_wr_clr_me_vw_err    <= '0';
      s_wr_clr_me_data_err  <= '0';
    elsif rising_edge(clk) then
      if fc_str = '1' then
        case (fc) is
          when c_rd_me_vw_err =>
            s_rd_me_vw_err <= '1';

          when c_rd_me_data_err =>
            s_rd_me_data_err <= '1';

          when others =>
        end case;
      end if;
      if clr_after_rd = '1' then
        case (fc) is
          when c_rd_me_vw_err =>
            s_wr_clr_me_vw_err <= '1';

          when c_rd_me_data_err =>
            s_wr_clr_me_data_err <= '1';
          when others =>
        end case;
      end if;
    end if;
  end process;


  decode_piggy_mode: process (clk, sclr)
  begin
    if sclr = '1' then
    --  s_wr_ifp_led <= '0';
  --    s_wr_ifp_out <= '0';
    --  s_rd_ifp_led <= '0';
  --    s_rd_ifp_io  <= '0';
    --  s_rd_ifp_id  <= '0';
   --   s_rd_if_mode <= '0';
    elsif rising_edge(clk) then
 --     s_wr_ifp_led <= '0';
  --    s_wr_ifp_out <= '0';
 --     s_rd_ifp_led <= '0';
     -- s_rd_ifp_io  <= '0';
--s_rd_ifp_id  <= '0';
   --   s_rd_if_mode <= '0';

      if fc_str = '1' then
        case (fc) is
       --   when c_wr_ifp_led =>
       --     s_wr_ifp_led <= '1';

     --     when c_wr_ifp_out =>
     --       s_wr_ifp_out <= '1';

      --    when c_rd_ifp_led =>
       --     s_rd_ifp_led <= '1';


     --     when c_rd_ifp_io =>
    --        s_rd_ifp_io <= '1';

    --      when c_rd_ifp_id =>
     --       s_rd_ifp_id <= '1';

    --      when c_rd_if_mode =>
        --    s_rd_if_mode <= '1';
          when others =>
        end case;
      end if;

    end if;
  end process;


  brdcst_flag: process (clk, sclr)
  begin
    if sclr = '1' then
      s_broad_en <= '0';
    elsif rising_edge(clk) then
      if fc_str = '1' and fc = c_res_broadcast then
        s_broad_en <= '1';
      end if;
    end if;
  end process;


  echo_reg: process (clk, sclr)
  begin
    if sclr = '1' then
      s_ed <= (others => '0');
   --   s_ifp_led <= (others => '0');
  --    s_ifp_out <= (others => '0');
    elsif rising_edge(clk) then
      if fc_str = '1' then
        case (fc) is
          when c_wr_echo =>
            s_ed <= di;

       --   when c_wr_ifp_led =>
       --     s_ifp_led <= di;

        --  when c_wr_ifp_out =>
        --    s_ifp_out <= di;
          when others =>
        end case;
      end if;
    end if;
  end process;

  vg_data_reg: process (clk, sclr)
  begin
    if sclr = '1' then
      s_vg_data <= (others => '0');
    elsif rising_edge(clk) then
      s_vg_data <= di;
    end if;
  end process;

  buildid: build_id_ram
  port map (
    clk           => clk,
    sclr          => s_rst_buildid,
    str           => s_rd_buildid,
    build_id_out  => build_id_out);


  ifa_sd_mux <= sts & ifa_adr                   when s_rd_sts         = '1' else
                inrm & x"00"                    when s_rd_irm         = '1' else
                ifa_ctrl & x"00"                when s_rd_cntrl       = '1' else
                ifa_id & x"00"                  when s_rd_id          = '1' else
                VERS_IFA & x"00"                when s_rd_vers        = '1' else
                s_ed                            when s_rd_echo        = '1' else
                '0' & '0' & '0' & i2c_data      when s_rd_i2c         = '1' else
                if_mode                         when s_rd_if_mode     = '1' else
                fwl_data_sts                    when s_rd_fwl_data    = '1' or s_rd_fwl_sts = '1' else
               -- s_ifp_led                       when s_rd_ifp_led     = '1' else
               -- ifp_in & s_ifp_out(14 downto 0) when s_rd_ifp_io      = '1' else
              --  x"00" & ifp_id                  when s_rd_ifp_id      = '1' else
                me_vw_err                       when s_rd_me_vw_err   = '1' else
                me_data_err                     when s_rd_me_data_err = '1' else
                build_id_out                    when s_rd_buildid     = '1' else
                diw;

fc_ext      <= s_fc_ext;
ifa_fc_int  <= s_fc_int;

send_str    <= s_send_str;
res_vw_err  <= s_res_vw_err;
reset_cmd   <= s_reset_cmd;
broad_en    <= not s_broad_en;
wr_i2c      <= s_wr_i2c;
-- write interrupt mask
wr_irm      <= s_wr_irm;

-- firmware loader
wr_fwl_data <= s_wr_fwl_data;
wr_fwl_ctrl <= s_wr_fwl_ctrl;

rd_fwl_data <= s_rd_fwl_data;
rd_fwl_sts  <= s_rd_fwl_sts;

ifa_sd_mux_sel  <= x"0";
vg_data         <= s_vg_data;
--ifp_led         <= s_ifp_led;
--ifp_led_out     <= s_ifp_out;

rd_me_vw_err        <= s_rd_me_vw_err;
rd_me_data_err      <= s_rd_me_data_err;
wr_clr_me_vw_err    <= s_wr_clr_me_vw_err;
wr_clr_me_data_err  <= s_wr_clr_me_data_err;


end architecture arch;
