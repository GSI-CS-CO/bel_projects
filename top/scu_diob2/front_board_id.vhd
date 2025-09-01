-- Identification of the front boards inserted in the slots of the Intermediate backplane 
--Author: Antonietta Russo <a.russo@gsi.de>

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.scu_diob_pkg.all;


entity front_board_id is 

Port ( clk               : in STD_LOGIC;
       nReset            : in STD_LOGIC;
       Deb_Sync          : in STD_LOGIC_VECTOR(71 downto 0);
       Deb_out           : in STD_LOGIC_VECTOR(71 downto 0);
       IOBP_Masken_Reg1  : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg2  : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg3  : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg4  : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg5  : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg6  : in STD_LOGIC_VECTOR(15 downto 0);
       PIO_SYNC          : in STD_LOGIC_VECTOR(142 DOWNTO 20);
       IOBP_ID           : in t_id_array;
       AW_Output_Reg     : in t_IO_Reg_1_to_7_Array;

       AW_IOBP_Input_Reg : out t_IO_Reg_1_to_7_Array;
       IOBP_Output       : out t_IOBP_array;    
       IOBP_Input        : out t_IOBP_array;
       IOBP_Aktiv_LED_i  : out t_led_array;
       OUT_SLOT          : out t_IOBP_array;
       ENA_SLOT          : out t_IOBP_array;
       IOBP_Sel_LED      : out t_led_array
);
end front_board_id ;

architecture Arch_front_board_id of front_board_id is

    signal conf_reg     :  t_id_array;
    signal IOBP_Out     :  t_IOBP_array; 

begin
         
   
  ID_Front_Board_proc: process (clk, nReset)

  begin

    if (not  nReset= '1')    then

        for i in 1 to 12 loop
            conf_reg(i) <= (others => '0');
            OUT_SLOT(i) <= (others => '0');
            ENA_SLOT(i) <= (others => '0');
        end loop;

    elsif (clk'EVENT AND clk = '1') then
	    
        conf_reg(1)<= IOBP_ID(1); --slot 1
        case conf_reg(1) is

            when "00000011"              => -- 6 LEMO Input Modul FG902130 

                AW_IOBP_Input_Reg(1)( 5 downto 0)  <= (Deb_Sync( 5 downto  0) AND not IOBP_Masken_Reg1( 5 downto  0));                           
                IOBP_Aktiv_LED_i(1)                <= not(IOBP_Masken_Reg1( 5 downto 0)); 
                IOBP_Input(1)                      <= (PIO_SYNC(56),  PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                IOBP_Output(1)                     <= (OTHERS => '0');
                IOBP_Sel_LED(1)                    <= Deb_out( 5 DOWNTO 0);   

                                                  
            when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 

                AW_IOBP_Input_Reg(1)( 5 downto 0)  <= (Deb_Sync( 5 downto  0) AND not IOBP_Masken_Reg1( 5 downto  0));
                IOBP_Aktiv_LED_i(1)                <= not(IOBP_Masken_Reg1( 5 downto 0));  
                IOBP_Input(1)                      <= not( PIO_SYNC(56),  PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                IOBP_Output(1)                     <= (OTHERS => '0');
                IOBP_Sel_LED(1)                    <= Deb_out( 5 DOWNTO 0);   


            when "00000100"              => -- 6 LWL Input Modul FG902110 

                AW_IOBP_Input_Reg(1)( 5 downto 0)  <= (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                IOBP_Aktiv_LED_i(1)                <= not(IOBP_Masken_Reg1( 5 downto 0));  
                IOBP_Input(1)                      <= (PIO_SYNC(56),  PIO_SYNC(60),  PIO_SYNC(62),  PIO_SYNC(52),  PIO_SYNC(54),  PIO_SYNC(58));
                IOBP_Output(1)                     <= (OTHERS => '0');
                IOBP_Sel_LED(1)                    <= Deb_out( 5 DOWNTO 0);


            when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                
                AW_Input_Reg(1)( 5 downto 0)       <= (OTHERS => '0');
                IOBP_Output(1)                     <= (AW_Output_Reg(1)(5 downto 0) AND not IOBP_Masken_Reg1( 5 downto 0));
                PIO_OUT_SLOT(1)                    <= IOBP_Output(1);
                PIO_ENA_SLOT(1)                    <= std_logic_vector'("111111");
                IOBP_Aktiv_LED_i(1)                <= not(IOBP_Masken_Reg1( 5 downto 0)); 
                IOBP_Input(1)                      <= (OTHERS => '0');
                IOBP_Sel_LED(1)                    <= IOBP_Output(1);


            when others     =>  NULL;

        end case;

        conf_reg(2)<= IOBP_ID(2); -- slot 2
        case conf_reg(2) is

            when "00000011"              => -- 6 LEMO Input Modul FG902130 

                AW_Input_Reg(1)(11 downto 6)       <= (Deb_Sync(11 downto  6) AND not IOBP_Masken_Reg1( 11 downto 6));
                IOBP_Aktiv_LED_i(2)                <= not(IOBP_Masken_Reg1(11 downto 6));
                IOBP_Input(2)                      <= (PIO_SYNC(96),  PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                IOBP_Output(2)                     <= (OTHERS => '0');
                IOBP_Sel_LED(2)                    <= Deb_out(11 DOWNTO 6); 


            when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 

                AW_Input_Reg(1)(11 downto 6)       <= (Deb_Sync(11 downto  6) AND not IOBP_Masken_Reg1(11 downto 6));
                IOBP_Aktiv_LED_i(2)                <= not(IOBP_Masken_Reg1(11 downto 6)); 
                IOBP_Input(2)                      <= not(PIO_SYNC(96),  PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                IOBP_Output(2)                     <= (OTHERS => '0');
                IOBP_Sel_LED(2)                    <= Deb_out(11 DOWNTO 6); 

                                                      

            when "00000100"              => -- 6 LWL Input Modul FG902110 

                AW_Input_Reg(1)(11 downto 6)      <= (Deb_Sync(11 downto  6) AND not IOBP_Masken_Reg1(11 downto  6));
                IOBP_Aktiv_LED_i(2)               <=  not(IOBP_Masken_Reg1(11 downto 6));  
                IOBP_Input(2)                     <=  (PIO_SYNC(96),  PIO_SYNC(100), PIO_SYNC(102), PIO_SYNC(92),  PIO_SYNC(94),  PIO_SYNC(98));
                IOBP_Output(2)                    <=  (OTHERS => '0');
                IOBP_Sel_LED(2)                   <=   Deb_out(11 DOWNTO 6); 


            when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                                      
                AW__Input_Reg(1)( 11 downto  6)   <= (OTHERS => '0');
                IOBP_Output(2)                    <= (AW_Output_Reg(1)(11 downto 6) AND not IOBP_Masken_Reg1(11 downto 6));
                PIO_OUT_SLOT(2)                   <= IOBP_Output(2);
                PIO_ENA_SLOT(2)                   <= std_logic_vector'("111111");
                IOBP_Aktiv_LED_i(2)               <= not(IOBP_Masken_Reg1(11 downto 6));  
                IOBP_Input(2)                     <= (OTHERS => '0');
                IOBP_Sel_LED(2)                   <=  IOBP_Output(2);


            when others =>  NULL;

        end case;

        conf_reg(3)<= IOBP_ID(3);  -- slot3
        
        case conf_reg(3) is

            when "00000011"              => -- 6 LEMO Input Modul FG902130 

                AW_Input_Reg(2)(5 downto 0)       <= (Deb_Sync(17 downto 12) AND not IOBP_Masken_Reg2( 5 downto  0));
                IOBP_Aktiv_LED_i(3)               <= not(IOBP_Masken_Reg2( 5 downto 0));  
                IOBP_Input(3)                     <= (PIO_SYNC(73),  PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));
                IOBP_Output(3)                    <= (OTHERS => '0');
                IOBP_Sel_LED(3)                   <= Deb_out(17 DOWNTO 12); 


            when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 

                AW_Input_Reg(2)( 5 downto 0)      <= (Deb_Sync(17 downto 12) AND not IOBP_Masken_Reg2( 5 downto 0));
                IOBP_Aktiv_LED_i(3)               <= not(IOBP_Masken_Reg2( 5 downto 0));  
                IOBP_Input(3)                     <= not(PIO_SYNC(73),  PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));
                IOBP_Output(3)                    <= (OTHERS => '0');
                IOBP_Sel_LED(3)                   <= Deb_out(17 DOWNTO 12);   


            when "00000100"              => -- 6 LWL Input Modul FG902110 
                
                AW_Input_Reg(2)( 5 downto 0)     <= (Deb_Sync(17 downto 12) AND not IOBP_Masken_Reg2( 5 downto 0));
                IOBP_Aktiv_LED_i(3)              <= not(IOBP_Masken_Reg2( 5 downto 0));  
                IOBP_Input(3)                    <= (PIO_SYNC(73),  PIO_SYNC(77),  PIO_SYNC(79),  PIO_SYNC(69),  PIO_SYNC(71),  PIO_SYNC(75));
                IOBP_Output(3)                   <= (OTHERS => '0');
                IOBP_Sel_LED(3)                  <= Deb_out(17 DOWNTO 12);  


            when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140

                AW_Input_Reg(2)( 5 downto  0)    <= (OTHERS => '0');
                IOBP_Output(3)                   <= (AW_Output_Reg(2)(5 downto 0) AND not IOBP_Masken_Reg2(5 downto 0));      
                PIO_OUT_SLOT(3)                   <= IOBP_Output(3);
                PIO_ENA_SLOT(3)                   <= std_logic_vector'("111111");
                IOBP_Aktiv_LED_i(3)              <= not(IOBP_Masken_Reg2( 5 downto 0));  -- Register für Sel-LED's vom Slave 3
                IOBP_Input(3)                    <= (OTHERS => '0');
                IOBP_Sel_LED(3)                  <= IOBP_Output(3);


            when others     =>   NULL;

        end case;

        conf_reg(4)<= IOBP_ID(4); -- slot 4
                                            
        case conf_reg(4) is

            when "00000011"              => -- 6 LEMO Input Modul FG902130 

                AW_Input_Reg(2)( 11 downto 6)    <= (Deb_Sync( 23 downto 18) AND not IOBP_Masken_Reg2(11 downto  6));
                IOBP_Aktiv_LED_i(4)              <= not(IOBP_Masken_Reg2(11 downto 6)); 
                IOBP_Input(4)                    <= (PIO_SYNC(101), PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                IOBP_Output(4)                   <= (OTHERS => '0');
                IOBP_Sel_LED(4)                  <=  Deb_out(23 DOWNTO 18); 

            when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 
                                                      
                AW_Input_Reg(2)(11 downto 6)     <= (Deb_Sync(23 downto 18) AND not IOBP_Masken_Reg2(11 downto 6));
                IOBP_Aktiv_LED_i(4)              <= not(IOBP_Masken_Reg2(11 downto 6));  
                IOBP_Input(4)                    <= not(PIO_SYNC(101), PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                IOBP_Output(4)                   <= (OTHERS => '0');
                IOBP_Sel_LED(4)                  <= Deb_out(23 DOWNTO 18); 


            when "00000100"              => -- 6 LWL Input Modul FG902110 
                                                      
                AW_Input_Reg(2)( 11 downto  6)   <= (Deb_Sync(23 downto 18) AND not IOBP_Masken_Reg2(11 downto 6));
                IOBP_Aktiv_LED_i(4)              <= not(IOBP_Masken_Reg2(11 downto 6) ); 
                IOBP_Input(4)                    <= (PIO_SYNC(101), PIO_SYNC(91), PIO_SYNC(93), PIO_SYNC(105), PIO_SYNC(103), PIO_SYNC(89));
                IOBP_Output(4)                   <= (OTHERS => '0');
                IOBP_Sel_LED(4)                  <= Deb_out(23 DOWNTO 18);  

            
            when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140

                AW_Input_Reg(2)(11 downto  6)    <= (OTHERS => '0');
                IOBP_Output(4)                   <= AW_Output_Reg(2)(11 downto 6) AND not IOBP_Masken_Reg2(11 downto 6);    
                PIO_OUT_SLOT(4)                  <= IOBP_Output(4);
                PIO_ENA_SLOT(4)                  <= std_logic_vector'("111111");
                IOBP_Aktiv_LED_i(4)              <= not(IOBP_Masken_Reg2(11 downto 6));  
                IOBP_Input(4)                    <= (OTHERS => '0');
                IOBP_Sel_LED(4)                  <=  IOBP_Output(4);


           when others     =>   NULL;
        
        end case;

        conf_reg(5)<= IOBP_ID(5);  --slot 5
                                              
        case conf_reg(5) is
                                                  
            when "00000011"              => -- 6 LEMO Input Modul FG902130 
                                                    
                AW_Input_Reg(3)( 5 downto 0)     <= (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto 0));
                IOBP_Aktiv_LED_i(5)              <=  not(IOBP_Masken_Reg3( 5 downto 0)); 
                IOBP_Input(5)                    <= (PIO_SYNC(53),  PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                IOBP_Output(5)                   <= (OTHERS => '0');
                IOBP_Sel_LED(5)                  <=  Deb_out(29 DOWNTO 24); 


            when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 

                AW_Input_Reg(3)( 5 downto 0)     <= (Deb_Sync(29 downto 24) AND not IOBP_Masken_Reg3( 5 downto 0));
                IOBP_Aktiv_LED_i(5)              <= not(IOBP_Masken_Reg3( 5 downto 0));  
                IOBP_Input(5)                    <= not(PIO_SYNC(53),  PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                IOBP_Output(5)                   <= (OTHERS => '0');
                IOBP_Sel_LED(5)                  <= Deb_out(29 DOWNTO 24);


            when "00000100"              => -- 6 LWL Input Modul FG902110 

                AW_Input_Reg(3)( 5 downto 0)      <= (Deb_Sync(29 downto 24) AND not IOBP_Masken_Reg3( 5 downto 0));
                IOBP_Aktiv_LED_i(5)               <=  not(IOBP_Masken_Reg3( 5 downto 0)); 
                IOBP_Input(5)                     <= (PIO_SYNC(53),  PIO_SYNC(61),  PIO_SYNC(63),  PIO_SYNC(57),  PIO_SYNC(55),  PIO_SYNC(59));
                IOBP_Output(5)                    <=  (OTHERS => '0');
                IOBP_Sel_LED(5)                   <=  Deb_out(29 DOWNTO 24); 

            when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                                      
                AW_Input_Reg(3)( 5 downto 0)      <= (OTHERS => '0');
                IOBP_Output(5)                    <= AW_Output_Reg(3)(5 downto  0) AND not IOBP_Masken_Reg3(5 downto 0); 
                PIO_OUT_SLOT(5)                   <= IOBP_Output(5);
                PIO_ENA_SLOT(5)                   <= std_logic_vector'("111111");
                IOBP_Aktiv_LED_i(5)               <= not (IOBP_Masken_Reg3( 5 downto 0 );  
                IOBP_Input(5)                     <= (OTHERS => '0');
                IOBP_Sel_LED(5)                   <= IOBP_Output(5); 


            when others     =>  NULL;

        end case;

			    
        conf_reg(6)<= IOBP_ID(6); -- slot 6
                                              
            case conf_reg(6) is
                                                  
                when "00000011"              => -- 6 LEMO Input Modul FG902130 

                    AW_Input_Reg(3)(11 downto 6)  <= (Deb_Sync(35 downto 30) AND not IOBP_Masken_Reg3(11 downto  6));
                    IOBP_Aktiv_LED_i(6)           <= not(IOBP_Masken_Reg3(11 downto 6)); 
                    IOBP_Input(6)                 <= (PIO_SYNC(119), PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));
                    IOBP_Output(6)                <= (OTHERS => '0');
                    IOBP_Sel_LED(6)               <= Deb_out(35 DOWNTO 30);


                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170 
    
                    AW_Input_Reg(3)(11 downto 6)  <= (Deb_Sync(35 downto 30) AND not IOBP_Masken_Reg3(11 downto 6));
                    IOBP_Aktiv_LED_i(6)           <= not(IOBP_Masken_Reg3(11 downto 6));
                    IOBP_Input(6)                 <= not(PIO_SYNC(119), PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));
                    IOBP_Output(6)                <= (OTHERS => '0');
                    IOBP_Sel_LED(6)               <= Deb_out(35 DOWNTO 30);


                when "00000100"              => -- 6 LWL Input Modul FG902110 
                    
                    AW_Input_Reg(3)(11 downto 6)  <= (Deb_Sync(35 downto 30) AND not IOBP_Masken_Reg3(11 downto 6));
                    IOBP_Aktiv_LED_i(6)           <= not(IOBP_Masken_Reg3(11 downto 6)); 
                    IOBP_Input(6)                 <= (PIO_SYNC(119), PIO_SYNC(109), PIO_SYNC(111), PIO_SYNC(123), PIO_SYNC(121), PIO_SYNC(107));
                    IOBP_Output(6)                <= (OTHERS => '0');
                    IOBP_Sel_LED(6)               <=  Deb_out(35 DOWNTO 30);


                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                    
                    AW_Input_Reg(3)(11 downto 6)  <= (OTHERS => '0');
                    IOBP_Output(6)                <= AW_Output_Reg(3)(11 downto 6) AND not IOBP_Masken_Reg3(11 downto 6);
                    PIO_OUT_SLOT(6)               <= IOBP_Output(6);
                    PIO_ENA_SLOT(6)               <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(6)           <= not(IOBP_Masken_Reg3(11 downto 6));  
                    IOBP_Input(6)                 <= (OTHERS => '0');
                    IOBP_Sel_LED(6)               <= IOBP_Output(6);


                when others     =>  NULL;

            end case;

    
	    
        conf_reg(7)<= IOBP_ID(7);  slot 7
                        
            case conf_reg(7) is

                when "00000011"              => -- 6 LEMO Input Modul FG902130 

                    AW_Input_Reg(4)( 5 downto 0)  <= (Deb_Sync( 41 downto 36) AND not IOBP_Masken_Reg4( 5 downto 0));
                    IOBP_Aktiv_LED_i(7)           <= not(IOBP_Masken_Reg4( 5 downto 0));  
                    IOBP_Input(7)                 <= (PIO_SYNC(35),  PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));
                    IOBP_Output(7)                <= (OTHERS => '0');
                    IOBP_Sel_LED(7)               <= Deb_out(41 DOWNTO 36);


                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170

                    AW_Input_Reg(4)( 5 downto 0)  <= (Deb_Sync(41 downto 36) AND not IOBP_Masken_Reg4( 5 downto 0));
                    IOBP_Aktiv_LED_i(7)           <= not(IOBP_Masken_Reg4( 5 downto 0));  
                    IOBP_Input(7)                 <= not( PIO_SYNC(35),  PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));
                    IOBP_Output(7)                <= (OTHERS => '0');
                    IOBP_Sel_LED(7)               <= Deb_out(41 DOWNTO 36);


                when "00000100"              => -- 6 LWL Input Modul FG902110 

                    AW_Input_Reg(4)( 5 downto 0)  <= (Deb_Sync(41 downto 36) AND not IOBP_Masken_Reg4( 5 downto 0));
                    IOBP_Aktiv_LED_i(7)           <= not(IOBP_Masken_Reg4( 5 downto 0)); 
                    IOBP_Input(7)                 <= ( PIO_SYNC(35),  PIO_SYNC(43),  PIO_SYNC(45),  PIO_SYNC(39),  PIO_SYNC(37),  PIO_SYNC(41));
                    IOBP_Output(7)                <= (OTHERS => '0');
                    IOBP_Sel_LED(7)               <= Deb_out(41 DOWNTO 36);


                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                        
                    AW_Input_Reg(4)( 5 downto 0)  <= (OTHERS => '0');
                    IOBP_Output(7)                <= AW_Output_Reg(4)(5 downto 0) AND not IOBP_Masken_Reg4(5 downto 0);
                    PIO_OUT_SLOT(7)               <= IOBP_Output(7);
                    PIO_ENA_SLOT(7)               <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(7)           <= not(IOBP_Masken_Reg4( 5 downto 0));  
                    IOBP_Input(7)                 <= (OTHERS => '0');
                    IOBP_Sel_LED(7)               <= IOBP_Output(7);

                 when others     =>  NULL;

            end case;  
            
                   
        conf_reg(8)<= IOBP_ID(8);  --slot 8
                                              
            case conf_reg(8) is
                                            
                when "00000011"              => -- 6 LEMO Input Modul FG902130 
                                                      
                    AW_Input_Reg(4)(11 downto 6)<= (Deb_Sync(47 downto 42) AND not IOBP_Masken_Reg4(11 downto  6));
                    IOBP_Aktiv_LED_i(8)         <= not(IOBP_Masken_Reg4(11 downto 6));  
                    IOBP_Input(8)               <= (PIO_SYNC(137), PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                    IOBP_Output(8)              <= (OTHERS => '0');
                    IOBP_Sel_LED(8)             <= Deb_out(47 DOWNTO 42);

                                                   
                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170

                    AW_Input_Reg(4)(11 downto 6)<= (Deb_Sync( 47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                    IOBP_Aktiv_LED_i(8)         <= not(IOBP_Masken_Reg4(11 downto 6) );  
                    IOBP_Input(8)               <= not( PIO_SYNC(137), PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                    IOBP_Output(8)              <= (OTHERS => '0');
                    IOBP_Sel_LED(8)             <= Deb_out(47 DOWNTO 42);


                when "00000100"              => -- 6 LWL Input Modul FG902110 
                                                      
                    AW_Input_Reg(4)(11 downto 6)<= (Deb_Sync( 47 downto 42) AND not IOBP_Masken_Reg4( 11 downto 6));
                    IOBP_Aktiv_LED_i(8)         <= not(IOBP_Masken_Reg4(11 downto 6)); 
                    IOBP_Input(8)               <= ( PIO_SYNC(137), PIO_SYNC(127), PIO_SYNC(129), PIO_SYNC(141), PIO_SYNC(139), PIO_SYNC(125));
                    IOBP_Output(8)              <= (OTHERS => '0');
                    IOBP_Sel_LED(8)             <= Deb_out(47 DOWNTO 42);


                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140

                    AW_Input_Reg(4)(11 downto 6)<= (OTHERS => '0');
                    IOBP_Output(8)              <= AW_Output_Reg(4)(11 downto 6) AND not IOBP_Masken_Reg4(11 downto 6);
                    PIO_OUT_SLOT(8)             <= IOBP_Output(8);
                    PIO_ENA_SLOT(8)             <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(8)         <= not(IOBP_Masken_Reg4(11 downto 6) ); 
                    IOBP_Input(8)               <= (OTHERS => '0');
                    IOBP_Sel_LED(8)             <= IOBP_Output(8);

                
                when others     =>  NULL;

            end case;    

        conf_reg(9)<= IOBP_ID(9);  -- slot 9
                                              
            case conf_reg(9) is
                                            
                when "00000011"              => -- 6 LEMO Input Modul FG902130 
  
                    AW_Input_Reg(5)( 5 downto 0)<= (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto 0));
                    IOBP_Aktiv_LED_i(9)         <= not(IOBP_Masken_Reg5( 5 downto 0));  
                    IOBP_Input(9)               <= (PIO_SYNC(30),  PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                    IOBP_Output(9)              <= (OTHERS => '0');
                    IOBP_Sel_LED(9)             <= Deb_out(53 DOWNTO 48);


                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170
                                        
                    AW_Input_Reg(5)( 5 downto 0)<= (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto 0));
                    IOBP_Aktiv_LED_i(9)         <= not(IOBP_Masken_Reg5( 5 downto 0));  
                    IOBP_Input(9)               <= not( PIO_SYNC(30),  PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                    IOBP_Output(9)              <= (OTHERS => '0');
                    IOBP_Sel_LED(9)             <= Deb_out(53 DOWNTO 48);


                when "00000100"              => -- 6 LWL Input Modul FG902110 
                                                      
                    AW_Input_Reg(5)( 5 downto 0)<= (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                    IOBP_Aktiv_LED_i(9)         <= not(IOBP_Masken_Reg5( 5 downto 0)); 
                    IOBP_Input(9)               <= (PIO_SYNC(30),  PIO_SYNC(22),  PIO_SYNC(20),  PIO_SYNC(26),  PIO_SYNC(28),  PIO_SYNC(24));
                    IOBP_Output(9)              <= (OTHERS => '0');
                    IOBP_Sel_LED(9)             <= Deb_out(53 DOWNTO 48);

                                                      
                 
                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                                      
                    AW_Input_Reg(5)(5 downto 0) <= (OTHERS => '0');
                    IOBP_Output(9)              <= AW_Output_Reg(5)(5 downto 0) AND not IOBP_Masken_Reg5(5 downto 0);
                    PIO_OUT_SLOT(9)             <= IOBP_Output(9);
                    PIO_ENA_SLOT(9)             <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(9)         <= not(IOBP_Masken_Reg5( 5 downto 0)); 
                    IOBP_Input(9)               <= (OTHERS => '0');
                    IOBP_Sel_LED(9)             <= IOBP_Output(9);


                when others     =>  NULL;
            
            end case;

        conf_reg(10)<= IOBP_ID(10);  -- slot 10
                                              
            case conf_reg(10) is
                                                  
                when "00000011"              => -- 6 LEMO Input Modul FG902130

                    AW_Input_Reg(5)(11 downto 6)<= (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5(11 downto  6));
                    IOBP_Aktiv_LED_i(10)        <= not(IOBP_Masken_Reg5(11 downto 6)); 
                    IOBP_Input(10)              <= (PIO_SYNC(130), PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                    IOBP_Output(10)             <= (OTHERS => '0');
                    IOBP_Sel_LED(10)            <= Deb_out(59 DOWNTO 54); 


                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170

                    AW_Input_Reg(5)(11 downto 6)<= (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5(11 downto 6));
                    IOBP_Aktiv_LED_i(10)        <= not(IOBP_Masken_Reg5(11 downto 6) ); 
                    IOBP_Input(10)              <= not(PIO_SYNC(130), PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                    IOBP_Output(10)             <= (OTHERS => '0');
                    IOBP_Sel_LED(10)            <= Deb_out(59 DOWNTO 54);
                 
                    
                when "00000100"              => -- 6 LWL Input Modul FG902110 
                    
                    AW_Input_Reg(5)(11 downto 6)<= (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5(11 downto 6));
                    IOBP_Aktiv_LED_i(10)        <= not(IOBP_Masken_Reg5(11 downto 6));
                    IOBP_Input(10)              <= (PIO_SYNC(130), PIO_SYNC(140), PIO_SYNC(138), PIO_SYNC(126), PIO_SYNC(128), PIO_SYNC(142));
                    IOBP_Output(10)             <= (OTHERS => '0');
                    IOBP_Sel_LED(10)            <= Deb_out(59 DOWNTO 54);

               
                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140

                    AW_Input_Reg(5)(11 downto 6)<= (OTHERS => '0');
                    IOBP_Output(10)             <= AW_Output_Reg(5)(11 downto 6) AND not IOBP_Masken_Reg5(11 downto 6);
                    PIO_OUT_SLOT(10)            <= IOBP_Output(10);
                    PIO_ENA_SLOT(10)            <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(10)        <= not(IOBP_Masken_Reg5(11 downto 6));  
                    IOBP_Input(10)              <= (OTHERS => '0');
                    IOBP_Sel_LED(10)            <=  IOBP_Output(10); 

                when others     =>  NULL;
                                              
            end case;       
            
        conf_reg(11)<= IOBP_ID(11);  -- slot 11

            case conf_reg(11) is
                                                  
                when "00000011"              => -- 6 LEMO Input Modul FG902130 
                                                      
                    AW_Input_Reg(6)( 5 downto 0)<= (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto 0));
                    IOBP_Aktiv_LED_i(11)        <= not(IOBP_Masken_Reg6(5 downto 0)); 
                    IOBP_Input(11)              <= (PIO_SYNC(48),PIO_SYNC(38), PIO_SYNC(46), PIO_SYNC(40), PIO_SYNC(44), PIO_SYNC(42));
                    IOBP_Output(11)             <= (OTHERS => '0');
                    IOBP_Sel_LED(11)            <= Deb_out(65 DOWNTO 60); 

                
                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170
                                                      
                    AW_Input_Reg(6)( 5 downto 0)<= (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto 0));
                    IOBP_Aktiv_LED_i(11)        <= not(IOBP_Masken_Reg6(5 downto 0)); 
                    IOBP_Input(11)              <= not(PIO_SYNC(48),PIO_SYNC(38), PIO_SYNC(46), PIO_SYNC(40), PIO_SYNC(44), PIO_SYNC(42));
                    IOBP_Output(11)             <= (OTHERS => '0');
                    IOBP_Sel_LED(11)            <= Deb_out(65 DOWNTO 60);

                
                when "00000100"              => -- 6 LWL Input Modul FG902110 

                    AW_Input_Reg(6)( 5 downto 0)<= (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto  0));
                    IOBP_Aktiv_LED_i(11)        <= not(IOBP_Masken_Reg6(5 downto 0)); 
                    IOBP_Input(11)              <= (PIO_SYNC(48),PIO_SYNC(40), PIO_SYNC(38), PIO_SYNC(44), PIO_SYNC(46), PIO_SYNC(42));
                    IOBP_Output(11)             <= (OTHERS => '0');
                    IOBP_Sel_LED(11)            <= Deb_out(65 DOWNTO 60);
                                                     

                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                                      
                    AW_Input_Reg(6)(5 downto 0) <= (OTHERS => '0');
                    IOBP_Output(11)             <= AW_Output_Reg(6)(5 downto 0) AND not IOBP_Masken_Reg6(5 downto 0);
                    PIO_OUT_SLOT(11)            <= IOBP_Output(11);
                    PIO_ENA_SLOT(11)            <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(11)        <= not(IOBP_Masken_Reg6(5 downto 0) );
                    IOBP_Input(11)              <= (OTHERS => '0');
                    IOBP_Sel_LED(11)            <=  IOBP_SK_Output(11); 


                when others     =>  NULL;

            end case; 


         conf_reg(12)<= IOBP_ID(12);  -- slot 12
                                              
            case conf_reg(12) is
                                                
                when "00000011"              => -- 6 LEMO Input Modul FG902130 
                                                      
                    AW_Input_Reg(6)(11 downto 6)<= (Deb_Sync(71 DOWNTO 66) AND not IOBP_Masken_Reg6( 11 downto  6));
                    IOBP_Aktiv_LED_i(12)        <= not(IOBP_Masken_Reg6(11 downto 6));
                    IOBP_Input(12)              <= (PIO_SYNC(112), PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124));
                    IOBP_Output(12)             <= (OTHERS => '0');
                    IOBP_Sel_LED(12)            <= Deb_out(71 DOWNTO 66);  


                when "00000111"|"00001001"   => -- 6 LEMO Input Modul FG902150 or 6 opt. Input Modul FG902170

                    AW_Input_Reg(6)(11 downto 6)<= (Deb_Sync(71 DOWNTO 66) AND not IOBP_Masken_Reg6( 11 downto  6));
                    IOBP_Aktiv_LED_i(12)        <= not ( IOBP_Masken_Reg6(11 downto 6));  
                    IOBP_Input(12)              <= not (PIO_SYNC(112), PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124));
                    IOBP_Output(12)             <= (OTHERS => '0');
                    IOBP_Sel_LED(12)            <= Deb_out(71 DOWNTO 66);  


                when "00000100"              => -- 6 LWL Input Modul FG902110 

                    AW_Input_Reg(6)(11 downto 6)<= (Deb_Sync(71 DOWNTO 66) AND not IOBP_Masken_Reg6( 11 downto 6));
                    IOBP_Aktiv_LED_i(12)        <= not(IOBP_Masken_Reg6(11 downto 6)); 
                    IOBP_Input(12)              <= (PIO_SYNC(112),PIO_SYNC(122), PIO_SYNC(120), PIO_SYNC(108), PIO_SYNC(110), PIO_SYNC(124));
                    IOBP_Output(12)             <= (OTHERS => '0');
                    IOBP_Sel_LED(12)            <= Deb_out(71 DOWNTO 66);


                when "00000101"|"00000110"   => -- 6 LWL Output Modul FG902120 or 6 LEMO Output Modul FG902140
                                                      
                    AW_Input_Reg(6)(11 downto 6)<= (OTHERS => '0');
                    IOBP_Output(12)             <= AW_Output_Reg(6)(11 downto  6) AND not IOBP_Masken_Reg6(11 downto 6);
                    PIO_OUT_SLOT(12)            <= IOBP_Output(12);
                    PIO_ENA_SLOT(12)            <= std_logic_vector'("111111");
                    IOBP_Aktiv_LED_i(12)        <= not(IOBP_Masken_Reg6(11 downto 6)); 
                    IOBP_Input(12)              <= (OTHERS => '0');
                    IOBP_Sel_LED(12)            <= IOBP_Output(12);

                when others     =>  NULL;
                                              
            end case;

    end if;

    end process ID_Front_Board_proc;

   IOBP_Output   <= IOBP_Out;
   
end architecture Arch_front_board_id;  