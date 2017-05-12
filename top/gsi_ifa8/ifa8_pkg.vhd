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

component io_6408_a is
  port (
      clk        : in std_logic;                      -- System-Clock
      sclr       : in std_logic;                      -- Sychron Clear
      sdo        : in std_logic;                      -- Serial-Data-Out    -+-> (Signale vom '6408' fr zu empfangene Daten)
      dsc        : in std_logic;                      -- Data-Shift-Clock    |
      td         : in std_logic;                      -- Take-Data           |
      cds        : in std_logic;                      -- Command/Data Synch. |
      vw         : in std_logic;                      -- Valid Word         -+
      res_vw_err : in std_logic;                      -- Reset Fehlerspeicher fr VW-Test
      wr_mil     : in std_logic;                      -- Starte Daten Senden
      sts_da     : in std_logic_vector(15 downto 0);  -- Sende-Daten          -+-> (Signale vom/zum '6408' fr zu sendende Daten)
      esc        : in std_logic;                      -- Encoder-Shift-Clock   |
      sd         : in std_logic;                      -- Send-Data            -+
      bzo        : in std_logic;                      -- Bus-Zero-Out         -+-> (Signale vom '6408' fr zu sendende Daten)
      boo        : in std_logic;                      -- Bus-One-Out          -+

      rcv_ready  : out std_logic;                     -- Strobe fr neue gltige Daten
      cmd_rcv    : out std_logic;                     -- CommandwordFunktionscode-Strobe fr Piggy
      mil_rcv_d  : out std_logic_vector(15 downto 0); -- Interne Daten
      sdi        : out std_logic;                     -- Serial-Data-In (serielle Daten zum '6408' fr zu sendende Daten)
      ee         : out std_logic;                     -- Encoder Enable (Startet Sendesequence beim '6408')
      nsend_en   : out std_logic;                     -- Enable Signal zur Sender-Freigabe
      nrcv_en    : out std_logic;
		rcv_err	  : out std_logic);                    -- Enable Signal zur Empfnger-Freigabe
end component io_6408_a;


component mil_en_decoder_ifa is
  port (
    mil_in_pos      : in std_logic;
    mil_in_neg      : in std_logic;
    clr_me_vw_err   : in std_logic;
    clr_me_data_err : in std_logic;
    wr_mil          : in std_logic;
    cmd_trm         : in std_logic;
    mil_trm_d       : in std_logic_vector(15 downto 0);
    clk             : in std_logic;
    manchester_clk  : in std_logic;
    ena_every_500ns : in std_logic;
    sclr            : in std_logic;
    powerup_flag    : in std_logic;
    test            : in std_logic;

    nsel_mil_rcv    : out std_logic;
    cmd_rcv         : out std_logic;
    rcv_error       : out std_logic;
    rcv_rdy       	: out std_logic;
    mil_rcv_d       : out std_logic_vector(15 downto 0);
    
    nmil_out_pos    : out std_logic;
    nmil_out_neg    : out std_logic;
    
    send_data       : out std_logic;
    nsel_mil_drv    : out std_logic;
    mil_rdy_4_wr    : out std_logic;
    
    mil_dec_diag_p  : out std_logic_vector(15 downto 0);
    mil_dec_diag_n  : out std_logic_vector(15 downto 0);
    
    me_dec_err      : out std_logic_vector(15 downto 0);
    me_vw_err       : out std_logic_vector(15 downto 0);
    me_data_err     : out std_logic_vector(15 downto 0);
    me_error_limit  : out std_logic);
end component;
end package ifa8_pkg;
