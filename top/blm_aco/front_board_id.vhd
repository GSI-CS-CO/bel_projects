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
       INTL_Output      : in std_logic_vector(11 downto 0);
       AW_Output_Reg    : in std_logic_vector(15 downto 0);

       AW_IOBP_Input_Reg     : out t_IO_Reg_1_to_7_Array;
       IOBP_Output     : out std_logic_vector (11 downto 0);     
       IOBP_Input     : out t_IOBP_array;
       IOBP_Aktiv_LED_i   : out t_led_array;
       OUT_SLOT_11         : out std_logic_vector(5 downto 0);
       ENA_SLOT_11         : out std_logic_vector(5 downto 0);
       OUT_SLOT_12         : out std_logic_vector(5 downto 0);
       ENA_SLOT_12         : out std_logic_vector(5 downto 0);
       IOBP_Sel_LED     : out t_led_array
);
end front_board_id ;

architecture Arch_front_board_id of front_board_id is
 
    type   t_reg_array         is array (1 to 12) of std_logic_vector(7 downto 0);
    signal conf_reg     :  t_reg_array;
    Signal IOBP_Out     :  std_logic_vector(11 downto 0);

begin
         
   
  ID_Front_Board_proc: process (clk, nReset)

  begin

      if (not  nReset= '1')    then
          for i in 1 to 12 loop
              conf_reg(i)<= (others => '0' );
          end loop;
          OUT_SLOT_11 <= (others => '0' );
          ENA_SLOT_11 <= (others => '0' );
          OUT_SLOT_12 <= (others => '0' );
          ENA_SLOT_12 <= (others => '0' );
     

      elsif (clk'EVENT AND clk = '1') then

                                            conf_reg(1)<= IOBP_ID(1);
                                            case conf_reg(1) is
                                                  when "00000011"  => --  6 LEMO Input Modul FG902.130 in slot 1
                                                      AW_IOBP_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(1)  <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1
                                                      IOBP_Input(1)  <= ( PIO_SYNC(56),  PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                                                      IOBP_Sel_LED(1)   <=   Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's 
                                                      
                                                  when "00000100" => -- 6 LWL  Input Modul in slot 1
                                                      AW_IOBP_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(1)  <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1
                                                      IOBP_Input(1)  <= ( PIO_SYNC(56),  PIO_SYNC(60),  PIO_SYNC(62),  PIO_SYNC(52),  PIO_SYNC(54),  PIO_SYNC(58));
                                                      IOBP_Sel_LED(1)   <=  Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's  
                                                
                                                when "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 1
                                                      AW_IOBP_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(1)  <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1
                                                      IOBP_Input(1)  <= not ( PIO_SYNC(56),  PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                                                      IOBP_Sel_LED(1)   <=  Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's
                                                      
                                                  when others     =>  NULL;
                                              end case;

 
                                            conf_reg(2)<= IOBP_ID(2);
                                            case conf_reg(2) is
                                                  when "00000011"  => --  6 LEMO Input Modul FG902.130 in slot 2
                                                      AW_IOBP_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(2)  <=  not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2
                                                      IOBP_Input(2)  <=( PIO_SYNC(96),  PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                                                      IOBP_Sel_LED(2)   <=    Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's
                                                      
                                                  when "00000100" => -- 6 LWL Input Modul in slot 2
                                                      AW_IOBP_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(2)  <=   not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2
                                                      IOBP_Input(2)  <=( PIO_SYNC(96),  PIO_SYNC(100), PIO_SYNC(102), PIO_SYNC(92),  PIO_SYNC(94),  PIO_SYNC(98));
                                                      IOBP_Sel_LED(2)   <=  Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's 

                                                  when "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 2
                                                      AW_IOBP_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(2)  <=   not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2
                                                      IOBP_Input(2)  <= not ( PIO_SYNC(96),  PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                                                      IOBP_Sel_LED(2)   <=   Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's
                                                      

                                                  when others     =>  NULL;
                                              end case;

 			    
                                            conf_reg(3)<= IOBP_ID(3);
                                            case conf_reg(3) is
                                                when "00000011"  => --  6 LEMO Input Modul FG902.130 in slot 3
                                                      AW_IOBP_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(3)  <= not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3
                                                      IOBP_Input(3)  <=( PIO_SYNC(73),  PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));
                                                      IOBP_Sel_LED(3)   <=  Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's 

                                                when  "00000100" => -- 6 LWL Input Modul in slot 3
                                                      AW_IOBP_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(3)  <=    not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3
                                                      IOBP_Input(3)  <=( PIO_SYNC(73),  PIO_SYNC(77),  PIO_SYNC(79),  PIO_SYNC(69),  PIO_SYNC(71),  PIO_SYNC(75));
                                                      IOBP_Sel_LED(3)   <= Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's
                                                
                                                when "00000111"   => -- 6 LEMO Input ModulFG902150 in slot 3
                                                      AW_IOBP_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(3)  <=   not( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3
                                                      IOBP_Input(3)  <= not ( PIO_SYNC(73),  PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));                    
                                                      IOBP_Sel_LED(3)   <= Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's
  
                                                when others     =>  NULL;
                                            end case;

 		    
                                            conf_reg(4)<= IOBP_ID(4);
                                            case conf_reg(4) is
                                                when "00000011"  => --  6 LEMO Input Modul FG902.130 in slot 4
                                                      AW_IOBP_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(4)  <= not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4
                                                      IOBP_Input(4)  <= ( PIO_SYNC(101), PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                                                      IOBP_Sel_LED(4)   <=  Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's 

                                                when "00000100" => -- 6 LWL Input Modul in slot 4
                                                      AW_IOBP_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(4)  <=  not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4
                                                      IOBP_Input(4)  <= ( PIO_SYNC(101), PIO_SYNC(91), PIO_SYNC(93), PIO_SYNC(105), PIO_SYNC(103), PIO_SYNC(89));
                                                      IOBP_Sel_LED(4)   <=  Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's 
                                                      
                                                when "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 4
                                                      AW_IOBP_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(4)  <=   not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4
                                                      IOBP_Input(4)  <= not ( PIO_SYNC(101), PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                                                      IOBP_Sel_LED(4)   <=   Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's


                 
                                                  when others     =>  NULL;
                                            end case;


                                            conf_reg(5)<= IOBP_ID(5);
                                              case conf_reg(5) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 5
                                                      AW_IOBP_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(5)  <=   not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5
                                                      IOBP_Input(5)  <= ( PIO_SYNC(53),  PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                                                      IOBP_Sel_LED(5)   <=   Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's

                                                when "00000100" => -- 6 LWL Input Modul in slot 5
                                                      AW_IOBP_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(5)  <=  not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5
                                                      IOBP_Input(5)  <= ( PIO_SYNC(53),  PIO_SYNC(61),  PIO_SYNC(63),  PIO_SYNC(57),  PIO_SYNC(55),  PIO_SYNC(59));
                                                      IOBP_Sel_LED(5)   <= Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's

                                                when "00000111"   => -- 6 LEMO Input Modul  FG902150 in slot 5
                                                      AW_IOBP_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(5)  <=   not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5
                                                      IOBP_Input(5)  <= not ( PIO_SYNC(53),  PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                                                      IOBP_Sel_LED(5)   <=   Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's

                                                    
                                              when others     =>  NULL;
                                          end case;

   
                                            conf_reg(6)<= IOBP_ID(6);
                                              case conf_reg(6) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 6
                                                      AW_IOBP_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(6)  <=   not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
                                                      IOBP_Input(6)  <= ( PIO_SYNC(119), PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));
                                                      IOBP_Sel_LED(6)   <=    Deb_out(35 DOWNTO 30);

                                                when "00000100" => -- 6 LWL Input Modul in slot 6
                                                      AW_IOBP_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(6)  <=  not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6;
                                                      IOBP_Input(6)  <= ( PIO_SYNC(119), PIO_SYNC(109), PIO_SYNC(111), PIO_SYNC(123), PIO_SYNC(121), PIO_SYNC(107));
                                                      IOBP_Sel_LED(6)   <=    Deb_out(35 DOWNTO 30); 

                                                when "00000111"   => -- 6 LEMO Input Modul  FG902150 in slot 6
                                                      AW_IOBP_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(6)  <=   not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
                                                      IOBP_Input(6)  <= not ( PIO_SYNC(119), PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));

                                                      IOBP_Sel_LED(6)   <=    Deb_out(35 DOWNTO 30);

                                          when others     =>  NULL;
                                      end case;
   
                                            conf_reg(7)<= IOBP_ID(7);
                                              case conf_reg(7) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 7
                                                      AW_IOBP_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(7)  <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7
                                                      IOBP_Input(7)  <= ( PIO_SYNC(35),  PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));
                                                      IOBP_Sel_LED(7)   <=     Deb_out(41 DOWNTO 36); 

                                                when  "00000100" => -- 6 LWL Input Modul in slot 7
                                                      AW_IOBP_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(7)  <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7
                                                      IOBP_Input(7)  <= ( PIO_SYNC(35),  PIO_SYNC(43),  PIO_SYNC(45),  PIO_SYNC(39),  PIO_SYNC(37),  PIO_SYNC(41));
                                                      IOBP_Sel_LED(7)   <=    Deb_out(41 DOWNTO 36); 

                                                when "00000111"   => -- 6 LEMO Input Modul F FG902150 in slot 7
                                                      AW_IOBP_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(7)  <=    not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7
                                                      IOBP_Input(7)  <= not ( PIO_SYNC(35),  PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));       
                                              
                                                      IOBP_Sel_LED(7)   <= Deb_out(41 DOWNTO 36);
                                               
                                          when others     =>  NULL;
                                      end case;
			    
                                            conf_reg(8)<= IOBP_ID(8);
                                              case conf_reg(8) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 8
                                                      AW_IOBP_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(8)  <=   not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8
                                                      IOBP_Input(8)  <= ( PIO_SYNC(137), PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                                                      IOBP_Sel_LED(8)   <=   Deb_out(47 DOWNTO 42);
                                                      
                                                when "00000100" => -- 6 LWL Input Modul in slot 8
                                                      AW_IOBP_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(8)  <=  not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8
                                                      IOBP_Input(8)  <= ( PIO_SYNC(137), PIO_SYNC(127), PIO_SYNC(129), PIO_SYNC(141), PIO_SYNC(139), PIO_SYNC(125));
                                                      IOBP_Sel_LED(8)   <=   Deb_out(47 DOWNTO 42);

                                                when "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 8
                                                      AW_IOBP_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED_i(8)  <=    not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8
                                                      IOBP_Input(8)  <= not ( PIO_SYNC(137), PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                                                      
                                                      IOBP_Sel_LED(8)   <=   Deb_out(47 DOWNTO 42);
  
                                                when others     =>  NULL;

                                              end case;

    
                                            conf_reg(9)<= IOBP_ID(9);
                                              case conf_reg(9) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 9
                                                      AW_IOBP_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(9)  <=   not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9
                                                      IOBP_Input(9)  <= ( PIO_SYNC(30),  PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                                                      IOBP_Sel_LED(9)   <=    Deb_out(53 DOWNTO 48); 

                                                when "00000100" => -- 6 LWL Input Modul in slot 9
                                                      AW_IOBP_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(9)  <=   not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9
                                                      IOBP_Input(9)  <= ( PIO_SYNC(30),  PIO_SYNC(22),  PIO_SYNC(20),  PIO_SYNC(26),  PIO_SYNC(28),  PIO_SYNC(24));
                                                      IOBP_Sel_LED(9)   <=    Deb_out(53 DOWNTO 48); 

                                                when  "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 9
                                                      AW_IOBP_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED_i(9)  <=   not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9
                                                      IOBP_Input(9)  <= not ( PIO_SYNC(30),  PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                                                    
                                                      IOBP_Sel_LED(9)   <= Deb_out(53 DOWNTO 48);

                                                when others     =>  NULL;
                                                  
                                              end case;

                                            conf_reg(10)<= IOBP_ID(10);
                                              case conf_reg(10) is
                                                when "00000011"  => -- 6 LEMO Input Modul FG902.130 in slot 10
                                                    AW_IOBP_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                    IOBP_Aktiv_LED_i(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10
                                                    IOBP_Input(10)  <= (PIO_SYNC(130), PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                                                    IOBP_Sel_LED(10)  <=    Deb_out(59 DOWNTO 54);

                                                when "00000100" => -- 6 LWL Input Modul in slot 10
                                                    AW_IOBP_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                    IOBP_Aktiv_LED_i(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10
                                                    IOBP_Input(10)  <= (PIO_SYNC(130), PIO_SYNC(140), PIO_SYNC(138), PIO_SYNC(126), PIO_SYNC(128), PIO_SYNC(142));
                                                    IOBP_Sel_LED(10)  <=    Deb_out(59 DOWNTO 54);

                                                when  "00000111"   => -- 6 LEMO Input Modul FG902150 in slot 10
                                                    AW_IOBP_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                    IOBP_Aktiv_LED_i(10)  <=   not ( IOBP_Masken_Reg5(11 downto 6)  ); 
                                                    IOBP_Input(10)  <= not (PIO_SYNC(130), PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                                                   
                                                    IOBP_Sel_LED(10)  <=  Deb_out(59 DOWNTO 54);
                                                               
                                                  when others     =>  NULL;
                                              end case;
	 
                                            conf_reg(11)<= IOBP_ID(11);
                                              case conf_reg(11) is
                                                when "00000101"  | "00000110" => -- Output Modul in slot 11
                                                  AW_IOBP_Input_Reg(6)(5 downto  0)<=   (OTHERS => '0');                                              
                                                  IOBP_Out(5 downto 0) <= INTL_Output(5 downto 0);                                                
                                                  OUT_SLOT_11 <= not IOBP_Out(5 downto 0);                                                  
                                             
                                                  ENA_SLOT_11 <= std_logic_vector'("111111");
                                                  IOBP_Aktiv_LED_i(11)  <=  not ( IOBP_Masken_Reg6( 5 downto 0) );  -- Register für Sel-LED's vom Slave 12
                                                  IOBP_Sel_LED(11)  <=  not IOBP_Out(5 downto 0);  
                                                  
                                              

                                                when others     =>  NULL;
                                              end case;
	    
                                            conf_reg(12)<= IOBP_ID(12);
                                              case conf_reg(12) is
                                                  
                                                  when "00000101"  | "00000110" => -- Output Modul in slot 12
                                                      AW_IOBP_Input_Reg(6)(11 downto  6)<=   (OTHERS => '0');
                                                      IOBP_Out(11 downto 6) <= INTL_Output(11 downto 6);     
                                                      OUT_SLOT_12 <= not IOBP_Out(11 downto 6);                                                  
                                             
                                                      ENA_SLOT_12<= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED_i(12)  <=  not ( IOBP_Masken_Reg6( 11 downto 6) );  -- Register für Sel-LED's vom Slave 12
                                                      IOBP_Sel_LED(12)  <=  not IOBP_Out(11 downto 6);  
                                                  when others     =>  NULL;
                                              end case;


    end if;
   end process ID_Front_Board_proc;
   IOBP_Output   <= (others =>'0');
   
end architecture Arch_front_board_id;
