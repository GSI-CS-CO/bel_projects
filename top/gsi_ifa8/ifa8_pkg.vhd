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

component ifactl is
  generic (
    ifa_id : in std_logic_vector(7 downto 0) := x"00");
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
    ifp_id             : in std_logic_vector(7 downto 0);    -- IF-Piggy identification code
    ifp_in             : in std_logic;                       -- IF-Piggy input
    ifa_epld_vers      : in std_logic_vector(7 downto 0);    -- IFA EPLD version number
    i2c_data           : in std_logic_vector(12 downto 0);   -- data i2c bus
    fwl_data_sts       : in std_logic_vector(15 downto 0);   -- data or status from IFA firmware loader
    me_vw_err          : in std_logic_vector(15 downto 0);   -- error counter ME-VW
    me_data_err        : in std_logic_vector(15 downto 0);   -- error counter ME-Data
    if_mode            : in std_logic_vector(15 downto 0);   -- mode of interface card
  
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
    ifp_led            : out std_logic_vector(15 downto 0);  -- IF-Piggy 'FG 380.751' LED port
    ifp_led_out        : out std_logic_vector(15 downto 0);  -- IF-Piggy 'FG 380.751' out/leds
    broad_en           : out std_logic;                      -- flag for broadcast enable
    reset_cmd          : out std_logic;                      -- reset command (fc = 0x01)
    res_vw_err         : out std_logic);                     -- reset VW error
end component ifactl;
	
component i2c
  generic (divisor : integer);
  port(
    sysclk      : in std_logic;
    clk_en      : in std_logic;
    nreset      : in std_logic;
    ack_tx      : in std_logic;
    cmd_stop    : in std_logic;
    cmd_start   : in std_logic;
    cmd_send    : in std_logic;
    cmd_receive : in std_logic;
    execute     : in std_logic;
    sda         : inout std_logic;
    scl         : inout std_logic;
    din         : in std_logic_vector(7 downto 0);
    ack_rx      : out std_logic;
    status      : out std_logic;
    dvalid      : out std_logic;
    denable     : out std_logic;
    busy        : out std_logic;
    dout        : out std_logic_vector(7 downto 0));
end component;

component irq_mask is
  port (
    intl_d       : in std_logic;
    drdy_d       : in std_logic;
    drq_d        : in std_logic;
    pures_d      : in std_logic;
  
    n_intl_si    : in std_logic;
    n_drdy_si    : in std_logic;
    n_drq_si     : in std_logic;

    clk          : in std_logic;
    wr_irm       : in std_logic;
    sclr         : in std_logic;
    
    intl_q       : buffer std_logic;
    drdy_q       : buffer std_logic;
    drq_q        : buffer std_logic;
    n_intl_out   : out std_logic;
    n_drdy_out   : out std_logic;
    n_drq_out    : out std_logic;
    powerup_flag : out std_logic;
    n_opt_inl    : out std_logic;
    n_opt_drdy   : out std_logic;
    n_opt_drq    : out std_logic);
end component irq_mask;

end package ifa8_pkg;
