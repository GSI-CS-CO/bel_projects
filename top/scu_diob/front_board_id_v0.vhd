-- Identification of the front boards inserted in the slots of the Intermediate backplane 
--Author: Antonietta Russo <a.russo@gsi.de>

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.scu_diob_pkg.all;


entity front_board_id is 

Port ( clk : in STD_LOGIC;
       nReset : in STD_LOGIC;
       Deb_Sync : in STD_LOGIC_VECTOR(65 downto 0);
       Deb_out   :in STD_LOGIC_VECTOR(65 downto 0);

       IOBP_Masken_Reg1 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg2 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg3 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg4 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg5 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg6 : in STD_LOGIC_VECTOR(15 downto 0);
       PIO_SYNC         : in STD_LOGIC_VECTOR(142 DOWNTO 20);
       IOBP_ID          : in t_id_array;
       INTL_Output      : in std_logic_vector(5 downto 0);
       AW_Output_Reg    : in std_logic_vector(5 downto 0);
       nBLM_out_ena      : in std_logic;
       AW_IOBP_Input_Reg     : out t_IO_Reg_1_to_7_Array;
       IOBP_Output     : out std_logic_vector (5 downto 0);     
       IOBP_Input     : out t_IOBP_array;
       IOBP_Aktiv_LED_i   : out t_led_array;
       OUT_SLOT         : out std_logic_vector(5 downto 0);
       ENA_SLOT         : out std_logic_vector(5 downto 0);
       IOBP_Sel_LED     : out t_led_array
);
end front_board_id ;

architecture Arch_front_board_id of front_board_id is
  --  type   IOBP_slot_state_t is   (IOBP_slot_idle, IOBP_slot1, IOBP_slot2,IOBP_slot3,IOBP_slot4,IOBP_slot5,IOBP_slot6,IOBP_slot7,IOBP_slot8,IOBP_slot9,IOBP_slot10,IOBP_slot11,IOBP_slot12);
  --  signal IOBP_slot_state:   IOBP_slot_state_t:= IOBP_slot_idle;
    type   t_reg_array         is array (1 to 12) of std_logic_vector(7 downto 0);
    signal conf_reg     :  t_reg_array;
    Signal IOBP_Out     :  std_logic_vector(5 downto 0);

begin
         
   
  ID_Front_Board_proc: process (clk, nReset)

  begin

      if (not  nReset= '1')    then
          for i in 1 to 12 loop
              conf_reg(i)<= (others => '0' );
          end loop;
          OUT_SLOT <= (others => '0' );
          ENA_SLOT <= (others => '0' );
          -- IOBP_slot_state <= IOBP_slot_idle;

      elsif (clk'EVENT AND clk = '1') then

      --    case IOBP_slot_state is

         --     when IOBP_slot_idle	=>
         --                                     IOBP_slot_state <= IOBP_slot1;

        --      when IOBP_slot1=>			    
                                            conf_reg(1)<= IOBP_ID(1);
                                            case conf_reg(1) is
                                                  when "00000011"  => -- 6 LEMO Input Modul in slot 1
                                                      AW_IOBP_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(1)  <=  Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(1)  <= ( PIO_SYNC(56),  PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                                                      IOBP_Sel_LED(1)   <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1
                                                      
                                                  when "00000100" => -- 6 LWL  Input Modul in slot 1
                                                      AW_IOBP_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(1)  <=  Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(1)  <= ( PIO_SYNC(56),  PIO_SYNC(60),  PIO_SYNC(62),  PIO_SYNC(52),  PIO_SYNC(54),  PIO_SYNC(58));
                                                      IOBP_Sel_LED(1)   <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1

                                                  when others     =>  NULL;
                                            end case;

        --                                      IOBP_slot_state <= IOBP_slot2;

        --      when IOBP_slot2=>			    
                                            conf_reg(2)<= IOBP_ID(2);
                                            case conf_reg(2) is
                                                  when "00000011"  => -- 6 LEMO Input Modul in slot 2
                                                      AW_IOBP_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(2)  <=   Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(2)  <=( PIO_SYNC(96),  PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                                                      IOBP_Sel_LED(2)   <=  not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2
                                                      
                                                  when "00000100" => -- 6 LWL Input Modul in slot 2
                                                      AW_IOBP_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(2)  <=   Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(2)  <=( PIO_SYNC(96),  PIO_SYNC(100), PIO_SYNC(102), PIO_SYNC(92),  PIO_SYNC(94),  PIO_SYNC(98));
                                                      IOBP_Sel_LED(2)   <=  not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2

                                                  when others     =>  NULL;
                                              end case;

--                                              IOBP_slot_state <= IOBP_slot3;

--              when IOBP_slot3=>			    
                                            conf_reg(3)<= IOBP_ID(3);
                                            case conf_reg(3) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 3
                                                      AW_IOBP_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(3)  <=   Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(3)  <=( PIO_SYNC(73),  PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));
                                                      IOBP_Sel_LED(3)   <=  not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3

                                                when  "00000100" => -- 6 LWL Input Modul in slot 3
                                                      AW_IOBP_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(3)  <=   Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(3)  <=( PIO_SYNC(73),  PIO_SYNC(77),  PIO_SYNC(79),  PIO_SYNC(69),  PIO_SYNC(71),  PIO_SYNC(75));
                                                      IOBP_Sel_LED(3)   <=  not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3
      
                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot4;

--               when IOBP_slot4=>			    
                                            conf_reg(4)<= IOBP_ID(4);
                                            case conf_reg(4) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 4
                                                      AW_IOBP_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(4)  <=   Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's
                                                      IOBP_Input(4)  <= ( PIO_SYNC(101), PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                                                      IOBP_Sel_LED(4)   <=  not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4

                                                when "00000100" => -- 6 LWL Input Modul in slot 4
                                                      AW_IOBP_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(4)  <=   Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's
                                                      IOBP_Input(4)  <= ( PIO_SYNC(101), PIO_SYNC(91), PIO_SYNC(93), PIO_SYNC(105), PIO_SYNC(103), PIO_SYNC(89));
                                                      IOBP_Sel_LED(4)   <=  not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4
                 
                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot5;

--              when IOBP_slot5=>			    
                                            conf_reg(5)<= IOBP_ID(5);
                                            case conf_reg(5) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 5
                                                      AW_IOBP_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(5)  <=   Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(5)  <= ( PIO_SYNC(53),  PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                                                      IOBP_Sel_LED(5)   <=  not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5

                                                when "00000100" => -- 6 LWL Input Modul in slot 5
                                                      AW_IOBP_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(5)  <=   Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's
                                                      IOBP_Input(5)  <= ( PIO_SYNC(53),  PIO_SYNC(61),  PIO_SYNC(63),  PIO_SYNC(57),  PIO_SYNC(55),  PIO_SYNC(59));
                                                      IOBP_Sel_LED(5)   <=  not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5

                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot6;

--              when IOBP_slot6=>			    
                                            conf_reg(6)<= IOBP_ID(6);
                                            case conf_reg(6) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 6
                                                      AW_IOBP_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(6)  <=    Deb_out(35 DOWNTO 30);
                                                      IOBP_Input(6)  <= ( PIO_SYNC(119), PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));
                                                      IOBP_Sel_LED(6)   <=  not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6

                                                when "00000100" => -- 6 LWL Input Modul in slot 6
                                                      AW_IOBP_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(6)  <=    Deb_out(35 DOWNTO 30);
                                                      IOBP_Input(6)  <= ( PIO_SYNC(119), PIO_SYNC(109), PIO_SYNC(111), PIO_SYNC(123), PIO_SYNC(121), PIO_SYNC(107));
                                                      IOBP_Sel_LED(6)   <=  not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6

                                                when others     =>  NULL;
                                            end case;

--                                             IOBP_slot_state <= IOBP_slot7;

--              when IOBP_slot7=>			    
                                            conf_reg(7)<= IOBP_ID(7);
                                            case conf_reg(7) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 7
                                                      AW_IOBP_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(7)  <=    Deb_out(41 DOWNTO 36);
                                                      IOBP_Input(7)  <= ( PIO_SYNC(35),  PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));
                                                      IOBP_Sel_LED(7)   <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7

                                                when  "00000100" => -- 6 LWL Input Modul in slot 7
                                                      AW_IOBP_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(7)  <=    Deb_out(41 DOWNTO 36);
                                                      IOBP_Input(7)  <= ( PIO_SYNC(35),  PIO_SYNC(43),  PIO_SYNC(45),  PIO_SYNC(39),  PIO_SYNC(37),  PIO_SYNC(41));
                                                      IOBP_Sel_LED(7)   <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7

                                               
                                                when others     =>  NULL;
                                            end case;

 --                                             IOBP_slot_state <= IOBP_slot8;

--              when IOBP_slot8=>			    
                                            conf_reg(8)<= IOBP_ID(8);
                                            case conf_reg(8) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 8
                                                      AW_IOBP_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(8)  <=    Deb_out(47 DOWNTO 42);
                                                      IOBP_Input(8)  <= ( PIO_SYNC(137), PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                                                      IOBP_Sel_LED(8)   <=  not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8
                                                      
                                                when "00000100" => -- 6 LWL Input Modul in slot 8
                                                      AW_IOBP_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(8)  <=    Deb_out(47 DOWNTO 42);
                                                      IOBP_Input(8)  <= ( PIO_SYNC(137), PIO_SYNC(127), PIO_SYNC(129), PIO_SYNC(141), PIO_SYNC(139), PIO_SYNC(125));
                                                      IOBP_Sel_LED(8)   <=  not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8
                                                
                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot9;

--              when IOBP_slot9=>			    
                                            conf_reg(9)<= IOBP_ID(9);
                                            case conf_reg(9) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 9
                                                      AW_IOBP_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(9)  <=    Deb_out(53 DOWNTO 48);
                                                      IOBP_Input(9)  <= ( PIO_SYNC(30),  PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                                                      IOBP_Sel_LED(9)   <=  not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9

                                                when "00000100" => -- 6 LWL Input Modul in slot 9
                                                      AW_IOBP_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(9)  <=    Deb_out(53 DOWNTO 48);
                                                      IOBP_Input(9)  <= ( PIO_SYNC(30),  PIO_SYNC(22),  PIO_SYNC(20),  PIO_SYNC(26),  PIO_SYNC(28),  PIO_SYNC(24));
                                                      IOBP_Sel_LED(9)   <=  not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9
 
                                                when others     =>  NULL;
                                                  
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot10;

--              when IOBP_slot10=>			    
                                            conf_reg(10)<= IOBP_ID(10);
                                            case conf_reg(10) is
                                                when "00000011"  => -- 6 LEMO Input Modul in slot 10
                                                    AW_IOBP_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                    IOBP_Aktiv_LED_i(10)  <=    Deb_out(59 DOWNTO 54);
                                                    IOBP_Input(10)  <= (PIO_SYNC(130), PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                                                    IOBP_Sel_LED(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10

                                                when "00000100" => -- 6 LWL Input Modul in slot 10
                                                    AW_IOBP_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                    IOBP_Aktiv_LED_i(10)  <=    Deb_out(59 DOWNTO 54);
                                                    IOBP_Input(10)  <= (PIO_SYNC(130), PIO_SYNC(140), PIO_SYNC(138), PIO_SYNC(126), PIO_SYNC(128), PIO_SYNC(142));
                                                    IOBP_Sel_LED(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10

                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot11;

--              when IOBP_slot11=>			    
                                            conf_reg(11)<= IOBP_ID(11);
                                            case conf_reg(11) is   
                                                when "00000011" => -- 6 LEMO Input Modul in slot 11
                                                    AW_IOBP_Input_Reg(6)( 5 downto  0) <=   (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto  0));
                                                    IOBP_Aktiv_LED_i(11)  <=    Deb_out(65 DOWNTO 60);
                                                    IOBP_Input(11)  <= (PIO_SYNC(48),PIO_SYNC(38), PIO_SYNC(46), PIO_SYNC(40), PIO_SYNC(44), PIO_SYNC(42));
                                                    IOBP_Sel_LED(11)  <=  not ( IOBP_Masken_Reg6(5 downto 0) );  -- Register für Sel-LED's vom Slave 11

                                                when "00000100" => -- 6 LWL Input Modul in slot 11
                                                    AW_IOBP_Input_Reg(6)( 5 downto  0) <=   (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto  0));
                                                    IOBP_Aktiv_LED_i(11)  <=    Deb_out(65 DOWNTO 60);
                                                    IOBP_Input(11)  <= (PIO_SYNC(48),PIO_SYNC(40), PIO_SYNC(38), PIO_SYNC(44), PIO_SYNC(46), PIO_SYNC(42));
                                                    IOBP_Sel_LED(11)  <=  not ( IOBP_Masken_Reg6(5 downto 0) );  -- Register für Sel-LED's vom Slave 11

                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot12;

--              when IOBP_slot12=>			    
                                            conf_reg(12)<= IOBP_ID(12);
                                            case conf_reg(12) is
                                                  
                                                when "00000101"  | "00000110" => -- Output Modul in slot 12
                                                    AW_IOBP_Input_Reg(6)(11 downto  6)<=   (OTHERS => '0');
                                                    ------------------------------------------------------------------
                                                    --- AW_Config register assigment to be defined
                                                    ------------------------------------------------------------------
                                                    if nBLM_out_ena ='0' then -- correct values to be checked

                                                        IOBP_Out <= INTL_Output;
                                                    else
                                                        IOBP_Out <= AW_Output_Reg AND not IOBP_Masken_Reg6(11 downto 6);
                                                    end if;
                                                      --------------------------------------------------------------------
                                                        
                                                    OUT_SLOT <= IOBP_Out;
                                                    ENA_SLOT<= std_logic_vector'("111111");
                                                    IOBP_Aktiv_LED_i(12)  <=  IOBP_Output;
                                                    
                                                    IOBP_Sel_LED(12)  <=  not ( IOBP_Masken_Reg6( 11 downto 6) );  -- Register für Sel-LED's vom Slave 12

                                                when others     =>  NULL;
                                            end case;

--                                              IOBP_slot_state <= IOBP_slot_idle;

  --                 when others =>           IOBP_slot_state <= IOBP_slot_idle;
 --         end case;

    end if;
   end process ID_Front_Board_proc;
   IOBP_Output   <= IOBP_Out;
end architecture Arch_front_board_id;
