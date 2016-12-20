library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

package ifa8_pkg is

component sweep_cntrl is
  generic (
    dw:       integer;
    f_in_khz: integer
  );
  port (
    clk:      in std_logic;
    freq_en:  in std_logic;
    reset:    in std_logic;
    
    ena_soft_trig:  in std_logic;
    ld_delta:       in std_logic;
    ld_delay:       in std_logic;
    ld_flattop_int: in std_logic;
    set_flattop:    in std_logic;
    stop_in:        in std_logic;
    hw_trig:        in std_logic;
    sw_trig:        in std_logic;
    ramp_fin:       in std_logic;
    delta:          in unsigned(dw-1 downto 0);
    d_in:           in unsigned(dw-1 downto 0);
    
    wr_delta:       out std_logic;
    s_stop_delta:   out std_logic;
    wr_ft_int:      out std_logic;
    wr_flattop:     out std_logic;
    idle:           out std_logic;
    w_start:        out std_logic;
    work:           out std_logic;
    stop:           out std_logic;
    stop_exec:      out std_logic;
    to_err:         out std_logic;
    seq_err:        out std_logic;
    trigger:        out std_logic;
    init_hw:        out std_logic
    
  );
end component;

component build_id_ram is
  port ( 
    clk:      in std_logic;
    sclr:     in std_logic;
    str:      in std_logic;
    
    build_id_out: out std_logic_vector(15 downto 0)
  );
end component;

end package ifa8_pkg;