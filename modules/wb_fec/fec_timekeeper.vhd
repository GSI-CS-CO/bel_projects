library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
  
library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

use work.fec_pkg.all;

entity fec_timekeeper is
  port(
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    ctrl_reg_i  : in  t_fec_ctrl_reg;
    lat_stat_o  : out t_fec_latency_stat_reg;
    init_i	    : in  std_logic;
    halt_i	    : in  std_logic);
end fec_timekeeper;

architecture rtl of fec_timekeeper is

  signal s_init_time        : unsigned(27 downto 0);
  signal s_halt_time        : unsigned(27 downto 0);
  signal s_fec_latency_max  : unsigned(27 downto 0);
  signal s_fec_latency_min  : unsigned(27 downto 0);

  type t_recording is (INIT, DELAY, IDLE);
  signal s_recording : t_recording := IDLE;

begin 

  counter : process(clk_i)
    variable v_fec_latency    : unsigned(27 downto 0);
    variable v_num_lat_sample : unsigned(31 downto 0);
    variable v_lat_acc        : unsigned(31 downto 0);
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_recording <= IDLE;
        v_fec_latency     := (others => '0');
        v_num_lat_sample  := (others => '0');
        v_num_lat_sample  := (others => '0');  
        s_init_time       <= (others => '0');
        s_halt_time       <= (others => '0');
        s_fec_latency_max <= (others => '0');
        s_fec_latency_min <= (others => '0');

      else

      case s_recording is
        when IDLE =>
          if init_i = '1' then
            s_recording <= INIT;
          end if;
        when INIT =>
          s_init_time <= unsigned(ctrl_reg_i.time_code.cycles);
          s_recording <= DELAY;
        when DELAY =>          
          if halt_i = '1' then
            s_recording <= IDLE;
            -- Latency
            v_fec_latency := unsigned(ctrl_reg_i.time_code.cycles) - s_init_time;
            lat_stat_o.fec_lat <= std_logic_vector(resize(v_fec_latency,32));
            -- Max and Min values
            if v_fec_latency > s_fec_latency_max then
              lat_stat_o.fec_lat_max <= std_logic_vector(resize(v_fec_latency,32));
              s_fec_latency_max <= v_fec_latency;
            elsif (v_fec_latency < s_fec_latency_min) and (v_fec_latency > 0) then
              lat_stat_o.fec_lat_min <= std_logic_vector(resize(v_fec_latency,32));
              s_fec_latency_min <= v_fec_latency;
            end if;
            -- Counter of samples
            v_num_lat_sample := v_num_lat_sample + 1;
            lat_stat_o.fec_num_lat <= std_logic_vector(v_num_lat_sample);
            -- Sum up values, must be reseted when read
            v_lat_acc :=  v_lat_acc + v_fec_latency;
            lat_stat_o.fec_lat_acc <=  std_logic_vector(v_lat_acc);
          end if;
        end case;
      end if;
    end if;
  end process;
end rtl; 

