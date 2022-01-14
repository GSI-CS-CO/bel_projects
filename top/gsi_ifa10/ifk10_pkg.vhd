library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

package ifk10_pkg is

COMPONENT IFA8_X
   GENERIC ( TEST : INTEGER := 0 );
   PORT
   (
      A_ME_ESC       :   IN STD_LOGIC;
      A_ME_TD        :   IN STD_LOGIC;
      A_ME_VW        :   IN STD_LOGIC;
      A_CLK_FG       :   IN STD_LOGIC;
      A_ME_DSC       :   IN STD_LOGIC;
      A_ME_SDO       :   IN STD_LOGIC;
      A_ME_CDS       :   IN STD_LOGIC;
      A_MIL1_BOI     :   IN STD_LOGIC;
      A_MIL1_BZI     :   IN STD_LOGIC;
      A_ME_SD        :   IN STD_LOGIC;
      A_Timing       :   IN STD_LOGIC;
      A_ME_nBOO      :   IN STD_LOGIC;
      A_ME_nBZO      :   IN STD_LOGIC;
      A_EXT_CLK      :   IN STD_LOGIC;
      V_Ain          :   IN STD_LOGIC_VECTOR(32 DOWNTO 1);
      V_Bin          :   IN STD_LOGIC_VECTOR(32 DOWNTO 1);
      V_Cin          :   IN STD_LOGIC_VECTOR(32 DOWNTO 1);
      EPIOS_CSn      :   IN STD_LOGIC;
      EPIOS_Adr      :   IN STD_LOGIC_VECTOR(11 DOWNTO 0);
      EPIOS_WR       :   IN STD_LOGIC;
      EPIOS_RD       :   IN STD_LOGIC;
      EPIOS_Frame    :   IN STD_LOGIC;
      EPIOS_CLK      :   IN STD_LOGIC;
      nEmpf_en       :   IN STD_LOGIC;
      EPIOS_Datain   :   IN STD_LOGIC_VECTOR(15 DOWNTO 0);
      clk_24MHz      :   IN STD_LOGIC;
      clk_6MHz       :   IN STD_LOGIC;
      sys_reset      :   IN STD_LOGIC;
      A_SEL_B        :   IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      A_UMIL15V      :   IN STD_LOGIC;
      A_UMIL5V       :   IN STD_LOGIC;
      sysclk         :   IN STD_LOGIC;
      Ena_Every100ns :   IN STD_LOGIC;
      Ena_Every166ns :   IN STD_LOGIC;
      Ena_Every250ns :   IN STD_LOGIC;
      Ena_Every500ns :   IN STD_LOGIC;
      Ena_Every1us   :   IN STD_LOGIC;
      Ena_Every20ms  :   IN STD_LOGIC;
      Ena_every1sec  :   IN  STD_LOGIC;
      Ena_every2_5s  :   IN  STD_LOGIC;
      FS_DATA        :   IN STD_LOGIC;
      CPU_RESET      :   IN STD_LOGIC;
      Manual_RES     :   IN STD_LOGIC;
      clk_50MHz      :   IN STD_LOGIC;
      IN_NINL        :   IN STD_LOGIC;
      IN_NRDY        :   IN STD_LOGIC;
      IN_NDRQ        :   IN STD_LOGIC;
      A_nSEL_6408    :   IN STD_LOGIC;
      A_nOPK_INL     :   OUT STD_LOGIC;
      A_nOPK_DRDY    :   OUT STD_LOGIC;
      A_nOPK_DRQ     :   OUT STD_LOGIC;
      A_TEST_CLK     :   OUT STD_LOGIC;
      A_ME_BZI       :   OUT STD_LOGIC;
      A_ME_BOI       :   OUT STD_LOGIC;
      A_ME_UDI       :   OUT STD_LOGIC;
      A_ME_EE        :   OUT STD_LOGIC;
      A_ME_SS        :   OUT STD_LOGIC;
      A_ME_SDI       :   OUT STD_LOGIC;
      A_MIL1_Out_ENA :   OUT STD_LOGIC;
      A_MIL1_nIN_ENA :   OUT STD_LOGIC;
      A_MIL1_nBZO    :   OUT STD_LOGIC;
      A_MIL1_nBOO    :   OUT STD_LOGIC;
      A_nSWITCH_Ena  :   OUT STD_LOGIC;
      FS_DCLK        :   OUT STD_LOGIC;
      FS_NCS         :   OUT STD_LOGIC;
      FS_ASDI        :   OUT STD_LOGIC;

      NOE_TST        :   OUT STD_LOGIC;
      A_Aout         :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
      A_Bout         :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
      A_Cout         :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
      A_I_Ena        :   OUT STD_LOGIC;
      A_I_Out        :   OUT STD_LOGIC;
      A_K_Ena        :   OUT STD_LOGIC;
      A_K_Out        :   OUT STD_LOGIC;
      A_L_Out        :   OUT STD_LOGIC;
      A_L_Ena        :   OUT STD_LOGIC;
      A_M_Out        :   OUT STD_LOGIC;
      A_M_Ena        :   OUT STD_LOGIC;
      A_N_Out        :   OUT STD_LOGIC;
      A_N_Ena        :   OUT STD_LOGIC;
      EPIOS_Dataout  :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      A_Test         :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      A_nLED         :   OUT STD_LOGIC_VECTOR(27 DOWNTO 0);
      A_E_Out        :   OUT STD_LOGIC;
      A_E_Ena        :   OUT STD_LOGIC;
      A_A_Out        :   OUT STD_LOGIC;
      A_A_Ena        :   OUT STD_LOGIC;
      A_B_Out        :   OUT STD_LOGIC;
      A_B_Ena        :   OUT STD_LOGIC;
      A_C_Out        :   OUT STD_LOGIC;
      A_C_Ena        :   OUT STD_LOGIC;
      A_D_Out        :   OUT STD_LOGIC;
      A_D_Ena        :   OUT STD_LOGIC;
      A_F_Out        :   OUT STD_LOGIC;
      A_F_Ena        :   OUT STD_LOGIC;
      A_G_Out        :   OUT STD_LOGIC;
      A_G_Ena        :   OUT STD_LOGIC;
      A_OUT_en       :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
      B_OUT_en       :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
      C_OUT_en       :   OUT STD_LOGIC_VECTOR(32 DOWNTO 1);

      X_SelDirO      :   OUT STD_LOGIC_VECTOR(12 DOWNTO 1);
      X_EnIO         :   OUT STD_LOGIC_VECTOR(12 DOWNTO 1);

      AP_IO          :   INOUT STD_LOGIC_VECTOR(45 DOWNTO 0);
      I2C_SDA        :   INOUT STD_LOGIC;
      I2C_SCL        :   INOUT STD_LOGIC
   );
END COMPONENT;


COMPONENT Clock_Timing
   PORT
   (
   PLL_reset            : in  STD_LOGIC;
   clk_24MHz_in         : in  STD_LOGIC;
   clk_50MHz_in         : in  STD_LOGIC;
   clk_ext_in           : in  STD_LOGIC;

   sys_clk              : out  STD_LOGIC; --120MHz

   clk_24MHz_out        : out  STD_LOGIC;
   clk_12MHZ_out        : out  STD_LOGIC;
   clk_50MHz_out        : out  STD_LOGIC;
   clk_25MHZ_out        : out  STD_LOGIC;
   clk_6MHz_out         : out  STD_LOGIC; --no PLL
   ext_clk_out          : out  STD_LOGIC; --no PLL

   locked_clk           : out  STD_LOGIC; --1 PLL locked, 0- PLL-rising

   Ena_every_100ns      : out  STD_LOGIC;
   Ena_every_166ns      : out  STD_LOGIC;
   Ena_every_250ns      : out  STD_LOGIC;
   Ena_every_500ns      : out  STD_LOGIC;
   Ena_every_us         : out  STD_LOGIC;
   Ena_every_ms         : out  STD_LOGIC;
   Ena_every_20ms       : out  STD_LOGIC;
   Ena_every_1sec       : out  STD_LOGIC;
   Ena_every_2_5s       : out  STD_LOGIC
   );
END COMPONENT;


component build_id_ram is
  port (
    clk:      in std_logic;
    sclr:     in std_logic;
    str:      in std_logic;

    build_id_out: out std_logic_vector(15 downto 0)
  );
end component;


--vk neues Steuermodul für 6408 encoder
 COMPONENT  A6408_encoder

  Port
  (
   --system controls
    sys_clk          : in  STD_LOGIC;  -- systemclock at least 2x of bin Data clk -=> (approx. >2MHz)
    sys_reset        : in  STD_LOGIC;
    enable           : in  STD_LOGIC;  -- '1' - enable this modul
    ESC              : in  STD_LOGIC;  -- Encoder-Shift-Clock   |

  -- encode data controls
    WR_MIL          : in  STD_LOGIC;  -- start encoding with a LH change here -- Startet ein Mil-Send, muß mindestens 1 Takt (SYS_Clock) aktiv sein.                --
    Sts_Da          : in  std_logic_vector(15 downto 0); --data word  to be transmitted
    SS_sel          : in  STD_LOGIC;  --sync select
    BZO             : in  STD_LOGIC;  -- Bus-Zero-Out        -> (Signale vom '6408' für zu sendende Daten)
    BOO             : in  STD_LOGIC;

---
    -- VHDL encoded signals
    SD               : in  STD_LOGIC;   --send data now to 6408
    SS               : out STD_LOGIC;  --sync select vorest fixed 0 -> send Data Words
    SDI              : OUT STD_LOGIC;  --Serielle Daten zum '6408' für zu sendende Daten)

    EE               : out STD_LOGIC; -- Encoder Enable (Startet Sendesequence beim '6408')
    Send_En          : out STD_LOGIC; -- Enable Signal zur Sender-Freigabe
    BZO_out          : out STD_LOGIC; -- Bus-Zero-Out        -> (Signale vom '6408' für zu sendende Daten)
    BOO_out          : out STD_LOGIC;

    enc_busy         : out STD_LOGIC     -- ongoing Encoding from WR_MIL strobe to nSend_En

  );
end COMPONENT ;


COMPONENT  A6408_decoder
  Port
  (
  --system controls
    sys_clk    : in  STD_LOGIC;  -- systemclock
    sys_reset  : in  STD_LOGIC;
    enable     : in  STD_LOGIC;  -- '1' - enable this modul

    -- decode data controls
    DSC        : in  STD_LOGIC;  -- Decoder-Shift-Clock
    SDO        : in  STD_LOGIC;  -- serielle Daten input
    TD         : in  STD_LOGIC;  -- Take-Data -Frame für datenbits vom MIL-Baustein
   CDS        : in  STD_LOGIC;   -- Data/Command Worttyp vom MIL-Baustein
    VW         : in  STD_LOGIC;  -- Valid Word
    Res_VW_Err : in  STD_LOGIC;  -- Reset Fehlerspeicher für VW-Test

---
    -- decoded signals
    rcv_ready  : out std_logic; -- Strobe für neue gültige Daten
    cds_out    : out std_logic; -- Data/Command Wort empfangen 0-Data 1- command
    mil_rcv_d  : out std_logic_vector(15 downto 0); -- Interne Daten
    nrcv_en    : out std_logic;
    rcv_err    : out std_logic;                    -- Enable Signal zur Empfänger-Freigabe

    dec_busy   : out STD_LOGIC    -- ongoing encoding, active from WR_MIL signal strobe to Send_En

  );

  end COMPONENT ;



COMPONENT mil_en_decoder_ifa
   PORT
   (
      Ena_Every_500ns   :   IN STD_LOGIC;
      Mil_TRM_D         :   IN STD_LOGIC_VECTOR(15 DOWNTO 0);
      Mil_in_Pos        :   IN STD_LOGIC;
      Mil_in_Neg        :   IN STD_LOGIC;
      CMD_TRM           :   IN STD_LOGIC;
      Wr_Mil            :   IN STD_LOGIC;
      sys_clk           :   IN STD_LOGIC;
      Test              :   IN STD_LOGIC;
      sys_reset         :   IN STD_LOGIC;
      Manchester_clk    :   IN STD_LOGIC;
      CLR_ME_VW_Err     :   IN STD_LOGIC;
      CLR_ME_Data_Err   :   IN STD_LOGIC;
      PowerUP_Flag      :   IN STD_LOGIC;
      Mil_RCV_D         :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      nMil_Out_Neg      :   OUT STD_LOGIC;
      nMil_Out_Pos      :   OUT STD_LOGIC;
      RCV_ERROR         :   OUT STD_LOGIC;
      RCV_Rdy           :   OUT STD_LOGIC;
      CMD_Rcv           :   OUT STD_LOGIC;
      Mil_Rdy_4_WR      :   OUT STD_LOGIC;
      nSel_Mil_DRV      :   OUT STD_LOGIC;
      Send_Data         :   OUT STD_LOGIC;
      nSEL_MIL_RCV      :   OUT STD_LOGIC;
      Mil_DEC_Diag_p    :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      Mil_DEC_Diag_n    :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      ME_VW_Err         :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      ME_Data_Err       :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      ME_Error_Limit    :   OUT STD_LOGIC;
      ME_DEC_Err        :   OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
   );
END COMPONENT;


component ifactl
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


---------------------------------------------------------------
--Debouncer - s. Panchow
COMPONENT Debounce
   GENERIC ( cnt : INTEGER :=4 );
   PORT
   (
      sig      :   IN STD_LOGIC;
      sel      :   IN STD_LOGIC;
      cnt_en   :   IN STD_LOGIC;
      clk      :   IN STD_LOGIC;
      res      :   IN STD_LOGIC;
      sig_deb  :   OUT STD_LOGIC
   );
END COMPONENT;
---------------------------------------------------------------


--HEX-Switch für Kartentyp_Auswahl
constant C_HW_SW_IFA230    : STD_LOGIC_VECTOR(3 downto 0) := B"0000";--0x0
constant C_HW_SW_IFAIOMODE : STD_LOGIC_VECTOR(3 downto 0) := B"0001";--0x1 IFA ,Enable IO-BUS; au spiggy Jumper
constant C_HW_SW_IFA112    : STD_LOGIC_VECTOR(3 downto 0) := B"0010";--0x2
constant C_HW_SW_IFA122    : STD_LOGIC_VECTOR(3 downto 0) := B"0011";--0x3
constant C_HW_SW_IFA203    : STD_LOGIC_VECTOR(3 downto 0) := B"0100";--0x4
constant C_HW_SW_IFA203B   : STD_LOGIC_VECTOR(3 downto 0) := B"0101";--0x5
constant C_HW_SW_IFA211    : STD_LOGIC_VECTOR(3 downto 0) := B"0110";--0x6 --für 1:1 Kompatibilität, wenn Steuersoftware nicht auf 230 umgestellt wird
constant C_HW_SW_IFA221    : STD_LOGIC_VECTOR(3 downto 0) := B"0111";--0x7 --für 1:1 Kompatibilität, wenn Steuersoftware nicht auf 230 umgestellt wird

constant C_HW_SW_TSTZ      : STD_LOGIC_VECTOR(3 downto 0) := B"1110";--0xE
constant C_HW_SW_TST       : STD_LOGIC_VECTOR(3 downto 0) := B"1111";--0xF


--für IOBUF-Auswahl
constant C_HW_NO_MODE      : STD_LOGIC_VECTOR(3 downto 0) := B"0000";  -- Keine zulässige HW ausgewählt
constant C_HW_MB_MODE      : STD_LOGIC_VECTOR(3 downto 0) := B"0001";
constant C_HW_SCU_MODE     : STD_LOGIC_VECTOR(3 downto 0) := B"0010";
constant C_HW_IFA_MODE     : STD_LOGIC_VECTOR(3 downto 0) := B"0100";  -- wird bei switch =0,1,4,5,6 ausgewählt
constant C_HW_112_MODE     : STD_LOGIC_VECTOR(3 downto 0) := B"0101";  -- wird bei switch =2 ausgewählt
constant C_HW_122_MODE     : STD_LOGIC_VECTOR(3 downto 0) := B"0011";  -- wird bei switch =3 ausgewählt
constant C_HW_TEST_MODEZ   : STD_LOGIC_VECTOR(3 downto 0) := B"0110";  -- 0xE  -- save Mode alle externen IO-Devices werden abgeschaltet
constant C_HW_TEST_MODE    : STD_LOGIC_VECTOR(3 downto 0) := B"0111";  -- 0xF


--Für Funktionsmodul-Auswahl
--IF_Mode Register
CONSTANT C_IFA_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0000";
CONSTANT C_FG_MODE         : STD_LOGIC_VECTOR(15 downto 0) := x"0001";
CONSTANT C_MB_MODE         : STD_LOGIC_VECTOR(15 downto 0) := x"0002";
CONSTANT C_SWEEP_MODE      : STD_LOGIC_VECTOR(15 downto 0) := x"0004";
CONSTANT C_IO_BUS_MODE     : STD_LOGIC_VECTOR(15 downto 0) := x"0008";
CONSTANT C_FG_DDS_MODE     : STD_LOGIC_VECTOR(15 downto 0) := x"0010";
CONSTANT C_SCU_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0020";
CONSTANT C_SCU_ACU_MODE    : STD_LOGIC_VECTOR(15 downto 0) := x"0040";

CONSTANT C_112_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0080";
CONSTANT C_122_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0081";
CONSTANT C_203_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0082";
CONSTANT C_203B_MODE       : STD_LOGIC_VECTOR(15 downto 0) := x"0083";

CONSTANT C_211_MODE        : STD_LOGIC_VECTOR(15 downto 0) := x"0800";



-- IDs für das MIL-Register
CONSTANT IFA_ID_230        : STD_LOGIC_VECTOR(7 downto 0) := x"F9";
CONSTANT IFA_ID_221        : STD_LOGIC_VECTOR(7 downto 0) := x"FA";
CONSTANT IFA_ID_211        : STD_LOGIC_VECTOR(7 downto 0) := x"FB";
CONSTANT IFA_ID_203        : STD_LOGIC_VECTOR(7 downto 0) := x"FC";
CONSTANT IFA_ID_122        : STD_LOGIC_VECTOR(7 downto 0) := x"FF";
CONSTANT IFA_ID_112        : STD_LOGIC_VECTOR(7 downto 0) := x"FF";

CONSTANT IFA_ID_TEST       : STD_LOGIC_VECTOR(7 downto 0) := x"01";
CONSTANT IFA_ID_TEST2      : STD_LOGIC_VECTOR(7 downto 0) := x"02";
CONSTANT IFA_ID_NO         : STD_LOGIC_VECTOR(7 downto 0) := x"00";        --dummy



-----------------
constant VERS_IFA          : STD_LOGIC_VECTOR(7 downto 0) := X"20";


end package ifk10_pkg;
