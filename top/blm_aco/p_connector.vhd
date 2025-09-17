LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity p_connector is

port(
  Powerup_Done		  : in std_logic;
  signal_tap_clk_250mhz  : in std_logic;
  A_SEL                  : in std_logic_vector(3 downto 0);
  PIO_SYNC		           : in STD_LOGIC_VECTOR(150 DOWNTO 16); 
  CLK_IO                 : in std_logic;                      -- Clock for user_I/0
  DIOB_Config1           : in std_logic_vector(15 downto 0);
  AW_Output_Reg          : in t_IO_Reg_1_to_7_Array;          -- Output-Register to the Piggys
  UIO_SYNC		           : in STD_LOGIC_VECTOR(15 DOWNTO 0); 
  hp_la_o                : in std_logic_vector(15 downto 0);
  local_clk_is_running   : in std_logic; 
  clk_blink              : in std_logic;
  s_nLED_Sel             : in std_logic;   -- LED = Sel
  s_nLED_Dtack           : in std_logic;   -- LED = Dtack
  s_nLED_inR             : in std_logic;   -- LED = interrupt
  s_nLED_User1_o         : in std_logic;   -- LED3 = User 1
  s_nLED_User2_o         : in  std_logic;  -- LED2 = User 2
  s_nLED_User3_o         : in std_logic;   -- LED1 = User 3
  Tag_Sts                : in std_logic_vector(15 downto 0);  -- Tag-Status
  Timing_Pattern_LA      : in std_logic_vector(31 downto 0);  --  latched timing pattern from SCU_Bus for external user functions
  Tag_Aktiv              : in std_logic_vector( 7 downto 0);  -- Flag: Bit7 = Tag7 (active) --- Bit0 = Tag0 (active)       
  IOBP_LED_ID_Bus_o      : in std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
  IOBP_ID                : in t_id_array;                     -- IDs of the "Slave-Boards"
  IOBP_LED_En            : in std_logic;                      -- Output-Enable für LED- ID-Bus 
  IOBP_STR_rot_o         : in std_logic_vector(12 downto 1);  -- LED-Str Red for Slave 12-1
  IOBP_STR_gruen_o       : in std_logic_vector(12 downto 1);  -- LED-Str Green for Slave 12-1
  IOBP_STR_ID_o          : in std_logic_vector(12 downto 1);  -- ID-Str Green for Slave 12-1
  IOBP_Output            : in std_logic_vector(5 downto 0);   -- Outputs "Slave-Boards 1-12" 
  IOBP_Input             : in t_IOBP_array;                   -- Inputs "Slave-Boards 1-12"
  Deb66_out              : in std_logic_vector(65 downto 0);
  AW_IOBP_Input_Reg      : in t_IO_Reg_1_to_7_Array;  -- Input-Register of the Piggy's
  PIO_ENA_SLOT_1         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_2         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_3         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_4         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_5         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_6         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_7         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_8         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_9         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_10         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_11         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_12         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_1         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_3          : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_2          : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_4         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_5       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_6        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_7       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_8        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_9      : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_10        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_11       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_12       : in std_logic_vector(5 downto 0);

  --------------------------------------------------------------------------------------
  A_TA                   : out std_logic_vector(15 downto 0); -- test port a
  IOBP_LED_ID_Bus_i      : out  std_logic_vector(7 downto 0); 
  PIO_OUT                : out   STD_LOGIC_VECTOR(150 DOWNTO 16); 
  PIO_ENA                : out   STD_LOGIC_VECTOR(150 DOWNTO 16);   
  UIO_OUT                : out   STD_LOGIC_VECTOR(15 DOWNTO 0);
  UIO_ENA                : out   STD_LOGIC_VECTOR(15 DOWNTO 0);
  AW_ID                  : out   std_logic_vector(7 downto 0);
  AWIn_Deb_Time          : out   integer range 0 to 7;           -- Debounce-Time 2 High "AWIn_Deb_Time", value from DIOB-Config 1
  Min_AWIn_Deb_Time      : out   integer range 0 to 7;           -- Minimal Debounce-Time 2 High"Min_AWIn_Deb_Time" 
  Diob_Status1           : out   std_logic_vector(15 downto 0);
  DIOB_Status2           : out   std_logic_vector(15 downto 0);
  IOBP_Id_Reg1           : out std_logic_vector(15 downto 0);
  IOBP_Id_Reg2           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg3           : out std_logic_vector(15 downto 0);
  IOBP_Id_Reg4           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg5           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg6           : out    std_logic_vector(15 downto 0);
  IOBP_Id_Reg7           : out   std_logic_vector(15 downto 0);
  IOBP_Id_Reg8           : out    std_logic_vector(15 downto 0);
  Deb66_in               : out     std_logic_vector(65 downto 0);
  Syn66                  : out        std_logic_vector(65 downto 0);
  AW_Input_Reg           : out   t_IO_Reg_1_to_8_Array;
  A_Tclk                 : out std_logic;
  extension_cid_group    : out  integer range 0 to 16#FFFF#;
  extension_cid_system   : out integer range 0 to 16#FFFF#;
  Max_AWOut_Reg_Nr       : out integer range 0 to 7;
  Max_AWIn_Reg_Nr        : out  integer range 0 to 7;
  Debounce_cnt           : out  integer range 0 to 16383;
  s_nLED_User1_i         : out std_logic;  -- LED3 = User 1
  s_nLED_User2_i         : out std_logic;  -- LED2 = User 2
  s_nLED_User3_i         : out std_logic;
  --IOBP_Output_Readback   : out t_IO_Reg_0_to_7_Array;
  --IOBP_Output_Readback   : out std_logic_vector(15 downto 0);
  Deb_Sync66             : out std_logic_vector(65 downto 0)
  );
  end p_connector;
  
  architecture rtl of p_connector is

    CONSTANT  CLK_sys_in_ps:  INTEGER  := (1000000000 / (125000000 / 1000));  --must actually be half-clk

   TYPE      t_Integer_Array  is array (0 to 7) of integer range 0 to 16383;

    constant  Wert_2_Hoch_n:   t_Integer_Array := (001000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   1uS
    002000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   2uS
    004000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   4uS
    008000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   8uS
    016000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  16uS
    032000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  32uS
    064000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  64uS
    128000 * 1000 / CLK_sys_in_ps);  -- Number of clocks for the Debounce Time of 128uS
  
  BEGIN

  --############################# Set Defaults ######################################

 --   PIO_OUT(150 downto 16)      <=  (OTHERS => '0');   -- setze alle Outputs auf '0';
  --  PIO_ENA(150 downto 16)      <=  (OTHERS => '0');   -- Disable alle Outputs;

    --UIO_OUT(15 downto 0)        <=  (OTHERS => '0');   -- setze alle Outputs auf '0';
    --UIO_ENA(15 downto 0)        <=  (OTHERS => '0');   -- Disable alle Outputs;
    --AW_ID(7 downto 0)           <=  x"FF";    -- Anwender-Karten ID
    -- AWIn_Deb_Time               <= 0;    -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time", Wert aus DIOB-Config 1
 -- Min_AWIn_Deb_Time           <= 0;    -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    Diob_Status1(15 downto 6)   <= (OTHERS => '0');      -- Reserve
    Diob_Status1(5  downto 0)   <= Tag_Sts(5 downto 0);  -- Tag-Ctrl Status
    Diob_Status2(15 downto 8)   <= (OTHERS => '0');      -- Reserve
    Diob_Status2( 7 downto 0)   <= Tag_Aktiv;            -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)


    --IOBP_LED_ID_Bus_i           <=             (OTHERS => '1');     -- Data_Output "Slave-Karte 1-12"

    --Deb66_in                    <= (OTHERS => '0');
   -- Syn66                       <= (OTHERS => '0');

    --#################################################################################
    --###                                                                           ###
    --###                    IO-Stecker-Test mit "BrückenStecker                    ###
    --###                                                                           ###
    --#################################################################################

    PROCESS ( Powerup_Done, AW_ID,signal_tap_clk_250mhz, A_SEL,
            PIO_SYNC, PIO_ENA,  PIO_OUT,  CLK_IO,
            AWIn_Deb_Time, Min_AWIn_Deb_Time, 
            AW_Input_Reg,
            DIOB_Config1,  
            AW_Output_Reg,
            UIO_SYNC, UIO_ENA,  UIO_OUT,
            hp_la_o, local_clk_is_running, clk_blink,
            s_nLED_Sel, s_nLED_Dtack, s_nLED_inR, s_nLED_User1_o, s_nLED_User2_o, s_nLED_User3_o,
            Timing_Pattern_LA,      
            IOBP_LED_ID_Bus_i, IOBP_LED_ID_Bus_o, IOBP_ID, IOBP_LED_En, IOBP_STR_rot_o, IOBP_STR_gruen_o, IOBP_STR_ID_o,
            IOBP_Id_Reg1, IOBP_Id_Reg2, IOBP_Id_Reg3, IOBP_Id_Reg4, IOBP_Id_Reg5, IOBP_Id_Reg6,           
            IOBP_Output, IOBP_Input, Deb66_out, Deb66_in, Syn66, AW_IOBP_Input_Reg, 
            PIO_ENA_SLOT_1,PIO_ENA_SLOT_2,PIO_ENA_SLOT_3, PIO_ENA_SLOT_4,PIO_ENA_SLOT_5,PIO_ENA_SLOT_6,
            PIO_ENA_SLOT_7,PIO_ENA_SLOT_8,PIO_ENA_SLOT_9,PIO_ENA_SLOT_10,PIO_ENA_SLOT_11,PIO_ENA_SLOT_12
            )
    begin

    IF  DIOB_Config1(15) = '1'  THEN   -- Config-Reg Bit15 = 1  --> Testmode

    --- Test der PIO-Pins ---

     AW_Input_Reg(1)(15 downto 0)  <=  ( CLK_IO,        PIO_SYNC(16),  PIO_SYNC(17),  PIO_SYNC(18),
                                         PIO_SYNC(19),  PIO_SYNC(20),  PIO_SYNC(21),  PIO_SYNC(22),
                                         PIO_SYNC(23),  PIO_SYNC(24),  PIO_SYNC(25),  PIO_SYNC(26),
                                         PIO_SYNC(27),  PIO_SYNC(28),  PIO_SYNC(29),  PIO_SYNC(30) );

          ( PIO_OUT(61),  PIO_OUT(62),  PIO_OUT(59),  PIO_OUT(60),
            PIO_OUT(57),  PIO_OUT(58),  PIO_OUT(55),  PIO_OUT(56),
            PIO_OUT(53),  PIO_OUT(54),  PIO_OUT(51),  PIO_OUT(52),
            PIO_OUT(49),  PIO_OUT(50),  PIO_OUT(47),  PIO_OUT(48)  )  <=  AW_Output_Reg(1)(15 downto 0) ;

            PIO_ENA(62 downto 47)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(2)(15 downto 0)  <=  ( PIO_SYNC(31),  PIO_SYNC(32),  PIO_SYNC(33),  PIO_SYNC(34),
                                          PIO_SYNC(35),  PIO_SYNC(36),  PIO_SYNC(37),  PIO_SYNC(38),
                                          PIO_SYNC(39),  PIO_SYNC(40),  PIO_SYNC(41),  PIO_SYNC(42),
                                          PIO_SYNC(43),  PIO_SYNC(44),  PIO_SYNC(45),  PIO_SYNC(46) );

          ( PIO_OUT(77),  PIO_OUT(78),  PIO_OUT(75),  PIO_OUT(76),
            PIO_OUT(73),  PIO_OUT(74),  PIO_OUT(71),  PIO_OUT(72),
            PIO_OUT(69),  PIO_OUT(70),  PIO_OUT(67),  PIO_OUT(68),
            PIO_OUT(65),  PIO_OUT(66),  PIO_OUT(63),  PIO_OUT(64)   )   <=  AW_Output_Reg(2)(15 downto 0) ;

            PIO_ENA(78 downto 63)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(3)(15 downto 0)  <=  ( PIO_SYNC(79),  PIO_SYNC(80),  PIO_SYNC(81),  PIO_SYNC(82),
                                          PIO_SYNC(83),  PIO_SYNC(84),  PIO_SYNC(85),  PIO_SYNC(86),
                                          PIO_SYNC(87),  PIO_SYNC(88),  PIO_SYNC(89),  PIO_SYNC(90),
                                          PIO_SYNC(91),  PIO_SYNC(92),  PIO_SYNC(93),  PIO_SYNC(94) );

          ( PIO_OUT(125), PIO_OUT(126), PIO_OUT(123), PIO_OUT(124),
            PIO_OUT(121), PIO_OUT(122), PIO_OUT(119), PIO_OUT(120),
            PIO_OUT(117), PIO_OUT(118), PIO_OUT(115), PIO_OUT(116),
            PIO_OUT(113), PIO_OUT(114), PIO_OUT(111), PIO_OUT(112)  )   <=  AW_Output_Reg(3)(15 downto 0) ;

            PIO_ENA(126 downto 111)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(4)(15 downto 0)  <=  ( PIO_SYNC(95),  PIO_SYNC(96),  PIO_SYNC(97),  PIO_SYNC(98),
                                          PIO_SYNC(99),  PIO_SYNC(100), PIO_SYNC(101), PIO_SYNC(102),
                                          PIO_SYNC(103), PIO_SYNC(104), PIO_SYNC(105), PIO_SYNC(106),
                                          PIO_SYNC(107), PIO_SYNC(108), PIO_SYNC(109), PIO_SYNC(110) );

          ( PIO_OUT(141), PIO_OUT(142), PIO_OUT(139), PIO_OUT(140),
            PIO_OUT(137), PIO_OUT(138), PIO_OUT(135), PIO_OUT(136),
            PIO_OUT(133), PIO_OUT(134), PIO_OUT(131), PIO_OUT(132),
            PIO_OUT(129), PIO_OUT(130), PIO_OUT(127), PIO_OUT(128)  )   <=  AW_Output_Reg(4)(15 downto 0) ;

            PIO_ENA(142 downto 127)                                     <= (others => '1'); -- Output-Enable


    AW_Input_Reg(5)(15 downto 4)  <=   AW_Output_Reg(5)(15 downto 4); --+   Input [15..4] = Copy der Output-Bits, da Testprog. nur 16 Bit Vergleich.
    AW_Input_Reg(5)(3  downto 0)  <=  (PIO_SYNC(143), PIO_SYNC(144), PIO_SYNC(149), PIO_SYNC(150));

   --  Beim Test, sind die Pins vom AW_Output_Reg(5)(3 downto 0) mit AW_Input_Reg(5)(3 downto 0) extern verbunden.

           (PIO_OUT(147), PIO_OUT(148), PIO_OUT(145), PIO_OUT(146))     <=  AW_Output_Reg(5)(3 downto 0) ;
            PIO_ENA(148 downto 145)                                     <= (others => '1'); -- Output-Enable


    --- Test der User-Pins zur VG-Leiste und HPLA1 (HP-Logicanalysator) ---


    UIO_ENA(15 downto 0)          <= (OTHERS => '0');           -- UIO = Input;
    AW_Input_Reg(6)(15 downto 0)  <=  UIO_SYNC(15 downto 0);    -- User-Pins zur VG-Leiste als Input


    A_TA(15 downto 0)             <= AW_Output_Reg(6)(15 downto 0);  -- HPLA1 (HP-Logicanalysator) als Output


    --- Test Codierschalter ---

    AW_Input_Reg(7)(15 downto 4)  <=  (OTHERS => '0');         -- setze alle unbenutzten Bit's = 0
    AW_Input_Reg(7)(3 downto 0)   <=  not A_SEL(3 downto 0);   -- Lese Codierschalter (neg. Logic)


  else

    --#################################################################################
    --#################################################################################
    --###                                                                           ###
    --###                         Stecker Anwender I/O                              ###
    --###                                                                           ###
    --#################################################################################
    --#################################################################################


    --input: Anwender_ID ---
      AW_ID(7 downto 0)         <=  PIO_SYNC(150 downto 143);
IOBP_Id_Reg7(15 downto 8) <= (others => '0');
IOBP_Id_Reg7(7 downto 0)  <= AW_ID(7 downto 0);

    --  --- Output: Anwender-LED's ---

    PIO_OUT(17) <= s_nLED_Sel;                          -- LED7 = sel Board
    PIO_OUT(19) <= s_nLED_Dtack;                        -- LED6 = Dtack
    PIO_OUT(21) <= s_nLED_inR;                          -- LED5 = interrupt
    PIO_OUT(23) <= not Powerup_Done or clk_blink;       -- LED4 = Powerup
    PIO_OUT(25) <= s_nLED_User1_o;                      -- LED3 = User 1
    PIO_OUT(27) <= s_nLED_User2_o;                      -- LED2 = User 2
    PIO_OUT(29) <= s_nLED_User3_o;                      -- LED1 = User 3
    PIO_OUT(31) <= local_clk_is_running and clk_blink;  -- LED0 (User-4) = int. Clock

   (PIO_ENA(17), PIO_ENA(19), PIO_ENA(21), PIO_ENA(23),
    PIO_ENA(25), PIO_ENA(27), PIO_ENA(29), PIO_ENA(31) )  <=  std_logic_vector'("11111111"); --  Output-Enable
PIO_ENA(150 downto 143) <= std_logic_vector'("00000000");

    A_TA(15 downto 0) <= hp_la_o(15 downto 0); ----------------- Output für HP-Logic-Analysator

    A_Tclk   <= signal_tap_clk_250mhz;  -- Clock  für HP-Logic-Analysator

    UIO_OUT(0)  <= '0';
    UIO_ENA(0)  <= '1';       -- Output-Enable für Interlock-Bit
    AW_Input_Reg(7)   <=  Timing_Pattern_LA(31 downto 16);  -- H-Word vom Timing_Pattern
    AW_Input_Reg(8)   <=  Timing_Pattern_LA(15 downto 0);   -- L-Word vom Timing_Pattern


 if AW_ID(7 downto 0) = "00010011" then --  c_AW_INLB12S1.ID 

--########################################################################################

extension_cid_group  <= 19; --"00010011"; -- c_AW_INLB12S1.CID; 

extension_cid_system <= 55; --c_cid_system;       -- extension card: CSCOHW

Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

--############################# Set Debounce- oder Syn-Time ######################################

  AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1

  IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN Debounce_cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
                                         ELSE Debounce_cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
  END IF;

--################################### Set LED's ########################################

s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei --
s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei --
s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei --

--========================== Output Register 1 ======================================

PIO_OUT(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA)
PIO_ENA(86)   <=  '1';                -- Output Enable
---------------------------------------------------------------------------------------------------------------------------------------

--========================== Output Register 2 ======================================

PIO_OUT(88)   <=  '0';  ---------------- Output_Enable OEn2 (nach init vom ALTERA)
PIO_ENA(88)   <=  '1';                -- Output Enable
---------------------------------------------------------------------------------------------------------------------------------------

--                    ID-Input-Register für die IO-Module Nr. 1+12

IOBP_Id_Reg6(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
IOBP_Id_Reg6( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
IOBP_Id_Reg5(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
IOBP_Id_Reg5( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
IOBP_Id_Reg4(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
IOBP_Id_Reg4( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
IOBP_Id_Reg3(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
IOBP_Id_Reg3( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
IOBP_Id_Reg2(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
IOBP_Id_Reg2( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
IOBP_Id_Reg1(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
IOBP_Id_Reg1( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1

-----------------------------------------------------------------------------------------------------------------------------------------
------------------------- general LED Assigments - intermediate backplane ---------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------

(PIO_OUT(114), PIO_OUT(50), PIO_OUT(132), PIO_OUT(32), PIO_OUT(135), PIO_OUT(33),
PIO_OUT(117), PIO_OUT(51), PIO_OUT(99),  PIO_OUT(83), PIO_OUT(106), PIO_OUT(66))  <=  IOBP_STR_rot_o;   -- LED-Strobe Rot  für Slave 12-1
(PIO_ENA(114), PIO_ENA(50), PIO_ENA(132), PIO_ENA(32), PIO_ENA(135), PIO_ENA(33),
PIO_ENA(117), PIO_ENA(51), PIO_ENA(99),  PIO_ENA(83), PIO_ENA(106), PIO_ENA(66))  <=  std_logic_vector'("111111111111");   -- Output Enable

(PIO_OUT(116), PIO_OUT(34), PIO_OUT(134), PIO_OUT(16), PIO_OUT(133), PIO_OUT(49),
PIO_OUT(115), PIO_OUT(67), PIO_OUT(97),  PIO_OUT(81), PIO_OUT(104), PIO_OUT(64))  <=  IOBP_STR_gruen_o; -- LED-Strobe Grün für Slave 12-1
(PIO_ENA(116), PIO_ENA(34), PIO_ENA(134), PIO_ENA(16), PIO_ENA(133), PIO_ENA(49),
PIO_ENA(115), PIO_ENA(67), PIO_ENA(97),  PIO_ENA(81), PIO_ENA(104), PIO_ENA(64))  <=  std_logic_vector'("111111111111");   -- Output Enable

(PIO_OUT(118), PIO_OUT(36), PIO_OUT(136), PIO_OUT(18), PIO_OUT(131), PIO_OUT(47),
PIO_OUT(113), PIO_OUT(65), PIO_OUT(95),  PIO_OUT(85), PIO_OUT(90),  PIO_OUT(68))  <=  not IOBP_STR_ID_o;    -- ID-Strobe für Slave 12-1 (Enable ist L-Aktiv)
(PIO_ENA(118), PIO_ENA(36), PIO_ENA(136), PIO_ENA(18), PIO_ENA(131), PIO_ENA(47),
PIO_ENA(113), PIO_ENA(65), PIO_ENA(95),  PIO_ENA(85), PIO_ENA(90),  PIO_ENA(68))  <=  std_logic_vector'("111111111111");   -- Output Enable

-------------------- Input/Output vom LED_ID_Bus der Zwischenbackplane  ------------
IOBP_LED_ID_Bus_i <= (PIO_Sync(70), PIO_Sync(72), PIO_Sync(74), PIO_Sync(76), PIO_Sync(78), PIO_Sync(80), PIO_Sync(82), PIO_Sync(84));   ------------------------- Input  LED_ID_Bus
                 (PIO_OUT(70),  PIO_OUT(72),  PIO_OUT(74),  PIO_OUT(76),  PIO_OUT(78),  PIO_OUT(80),  PIO_OUT(82),  PIO_OUT(84))   <=  IOBP_LED_ID_Bus_o;   -- Output LED_ID_Bus


-------------------- Tri-State Steuerung vom LED_ID_Bus der Zwischenbackplane  ------------
IF IOBP_LED_En = '1' THEN ---------------- LED write Loop
(PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("11111111");  -- Output Enable
ELSE --------------------------------------ID read Loop
(PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("00000000");  -- Output Disable
END IF;

-----------------------------------------------------------------------------------------------------------------------------------------
( PIO_ENA(56),  PIO_ENA(62),  PIO_ENA(54),  PIO_ENA(60),  PIO_ENA(52),  PIO_ENA(58)) <= PIO_ENA_SLOT_1;
( PIO_ENA(96),  PIO_ENA(102), PIO_ENA(94),  PIO_ENA(100), PIO_ENA(92),  PIO_ENA(98)) <= PIO_ENA_SLOT_2;
( PIO_ENA(73),  PIO_ENA(79),  PIO_ENA(71),  PIO_ENA(77),  PIO_ENA(69),  PIO_ENA(75)) <= PIO_ENA_SLOT_3;
( PIO_ENA(101), PIO_ENA(93),  PIO_ENA(103), PIO_ENA(91),  PIO_ENA(105), PIO_ENA(89)) <= PIO_ENA_SLOT_4;
( PIO_ENA(53),  PIO_ENA(63),  PIO_ENA(55),  PIO_ENA(61),  PIO_ENA(57),  PIO_ENA(59)) <= PIO_ENA_SLOT_5;
( PIO_ENA(119), PIO_ENA(111), PIO_ENA(121), PIO_ENA(109), PIO_ENA(123), PIO_ENA(107))<= PIO_ENA_SLOT_6;
( PIO_ENA(35),  PIO_ENA(45),  PIO_ENA(37),  PIO_ENA(43),  PIO_ENA(39),  PIO_ENA(41)) <= PIO_ENA_SLOT_7;
( PIO_ENA(137), PIO_ENA(129), PIO_ENA(139), PIO_ENA(127), PIO_ENA(141), PIO_ENA(125))<= PIO_ENA_SLOT_8;
( PIO_ENA(30),  PIO_ENA(20),  PIO_ENA(28),  PIO_ENA(22),  PIO_ENA(26),  PIO_ENA(24)) <= PIO_ENA_SLOT_9;
( PIO_ENA(130), PIO_ENA(138), PIO_ENA(128), PIO_ENA(140), PIO_ENA(126), PIO_ENA(142))<= PIO_ENA_SLOT_10;
( PIO_ENA(48),  PIO_ENA(38),  PIO_ENA(46),  PIO_ENA(40),  PIO_ENA(44),  PIO_ENA(42)) <= PIO_ENA_SLOT_11;
( PIO_ENA(112), PIO_ENA(120), PIO_ENA(110), PIO_ENA(122), PIO_ENA(108), PIO_ENA(124))<= PIO_ENA_SLOT_12;

( PIO_OUT(56),  PIO_OUT(62),  PIO_OUT(54),  PIO_OUT(60),  PIO_OUT(52),  PIO_OUT(58)) <= PIO_OUT_SLOT_1;
( PIO_OUT(96),  PIO_OUT(102), PIO_OUT(94), PIO_OUT(100),  PIO_OUT(92),  PIO_OUT(98)) <= PIO_OUT_SLOT_2;
( PIO_OUT(73),  PIO_OUT(79),  PIO_OUT(71),  PIO_OUT(77),  PIO_OUT(69),  PIO_OUT(75)) <= PIO_OUT_SLOT_3;
( PIO_OUT(101), PIO_OUT(93),  PIO_OUT(103), PIO_OUT(91),  PIO_OUT(105), PIO_OUT(89)) <= PIO_OUT_SLOT_4;
( PIO_OUT(53),  PIO_OUT(63),  PIO_OUT(55),  PIO_OUT(61),  PIO_OUT(57),  PIO_OUT(59)) <= PIO_OUT_SLOT_5;
( PIO_OUT(119), PIO_OUT(111), PIO_OUT(121), PIO_OUT(109), PIO_OUT(123), PIO_OUT(107))<= PIO_OUT_SLOT_6;
( PIO_OUT(35),  PIO_OUT(45),  PIO_OUT(37),  PIO_OUT(43),  PIO_OUT(39),  PIO_OUT(41)) <= PIO_OUT_SLOT_7;
( PIO_OUT(137), PIO_OUT(129), PIO_OUT(139), PIO_OUT(127), PIO_OUT(141), PIO_OUT(125))<= PIO_OUT_SLOT_8;
( PIO_OUT(30),  PIO_OUT(20),  PIO_OUT(28),  PIO_OUT(22),  PIO_OUT(26),  PIO_OUT(24)) <= PIO_OUT_SLOT_9;
( PIO_OUT(130), PIO_OUT(138), PIO_OUT(128), PIO_OUT(140), PIO_OUT(126), PIO_OUT(142))<= PIO_OUT_SLOT_10;
( PIO_OUT(48),  PIO_OUT(38),  PIO_OUT(46),  PIO_OUT(40),  PIO_OUT(44),  PIO_OUT(42)) <= PIO_OUT_SLOT_11;
( PIO_OUT(112), PIO_OUT(120), PIO_OUT(110), PIO_OUT(122), PIO_OUT(108), PIO_OUT(124))<= PIO_OUT_SLOT_12;

for i in 1 to 6 loop
AW_Input_Reg(i)<= AW_IOBP_Input_Reg(i);
end loop;

---output readback
--IOBP_Output_Readback <= "0000000000" & IOBP_Output;
--IOBP_Output_Readback(0) <= "0000000000" & IOBP_Output;
--IOBP_Output_Readback(1) <= (OTHERS => '0');
--IOBP_Output_Readback(2) <= (OTHERS => '0');
--IOBP_Output_Readback(3) <= (OTHERS => '0');
--IOBP_Output_Readback(4) <= (OTHERS => '0');
--IOBP_Output_Readback(5) <= (OTHERS => '0');
--IOBP_Output_Readback(6) <= (OTHERS => '0');
--IOBP_Output_Readback(7) <= (OTHERS => '0');

--################################ Debounce oder Sync Input's  ##################################

--  Deb66_in = H-Aktiv             IOBP_Input = L-Aktiv
--        |                                |
Deb66_in( 5 DOWNTO  0)   <=  not IOBP_Input( 1);  -- Input-Daten
Deb66_in(11 DOWNTO  6)   <=  not IOBP_Input( 2);
Deb66_in(17 DOWNTO 12)   <=  not IOBP_Input( 3);
Deb66_in(23 DOWNTO 18)   <=  not IOBP_Input( 4);
Deb66_in(29 DOWNTO 24)   <=  not IOBP_Input( 5);
Deb66_in(35 DOWNTO 30)   <=  not IOBP_Input( 6);
Deb66_in(41 DOWNTO 36)   <=  not IOBP_Input( 7);
Deb66_in(47 DOWNTO 42)   <=  not IOBP_Input( 8);
Deb66_in(53 DOWNTO 48)   <=  not IOBP_Input( 9);
Deb66_in(59 DOWNTO 54)   <=   not IOBP_Input( 10);
Deb66_in(65 DOWNTO 60)   <=   not IOBP_Input( 11);

--  Syn66 = H-Aktiv             IOBP_Input = L-Aktiv
--                                      |
Syn66 ( 5 DOWNTO  0)   <=  not IOBP_Input( 1);  -- Input-Daten
Syn66(11 DOWNTO  6)   <=  not IOBP_Input( 2);
Syn66(17 DOWNTO 12)   <=  not IOBP_Input( 3);
Syn66(23 DOWNTO 18)   <=  not IOBP_Input( 4);
Syn66(29 DOWNTO 24)   <=  not IOBP_Input( 5);
Syn66(35 DOWNTO 30)   <=  not IOBP_Input( 6);
Syn66(41 DOWNTO 36)   <=  not IOBP_Input( 7);
Syn66(47 DOWNTO 42)   <=  not IOBP_Input( 8);
Syn66(53 DOWNTO 48)   <=  not IOBP_Input( 9);
Syn66(59 DOWNTO 54)   <=  not IOBP_Input( 10);
Syn66(65 DOWNTO 60)   <=  not IOBP_Input( 11);

--IF  (Diob_Config1(11) = '1')  THEN Deb_Sync66 <=  Syn66;         -- Dobounce = Abgeschaltet ==> nur Synchronisation
                         --ELSE 
                         Deb_Sync66 <=  Deb66_out;     -- Debounce und Synchronisation
--END IF;



else
   
    extension_cid_system <=  0;  -- extension card: cid_system
    extension_cid_group  <=  0;  -- extension card: cid_group

    Max_AWOut_Reg_Nr     <=  0;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <=  0;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <=  0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    s_nLED_User1_i       <= '0';        -- LED3 = User 1, -- frei --
    s_nLED_User2_i       <= '0';        -- LED3 = User 2, -- frei --
    s_nLED_User3_i       <= '0';        -- LED3 = User 3, -- frei --

  -- Output: Anwender-LED's ---

    PIO_OUT(17)   <=  clk_blink; -- LED7
    PIO_OUT(19)   <=  clk_blink; -- LED6
    PIO_OUT(21)   <=  clk_blink; -- LED5
    PIO_OUT(23)   <=  clk_blink; -- LED4
    PIO_OUT(25)   <=  clk_blink; -- LED3
    PIO_OUT(27)   <=  clk_blink; -- LED2
    PIO_OUT(29)   <=  clk_blink; -- LED1
    PIO_OUT(31)   <=  clk_blink; -- LED0

   (PIO_ENA(17), PIO_ENA(19), PIO_ENA(21), PIO_ENA(23),
    PIO_ENA(25), PIO_ENA(27), PIO_ENA(29), PIO_ENA(31) )  <=  std_logic_vector'("11111111"); -- Output Enable

  END if;

  END IF;
  end process;

end architecture;