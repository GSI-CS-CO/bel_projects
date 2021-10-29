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
       Deb_Sync : in STD_LOGIC_VECTOR(71 downto 0);
       Deb_out   :in STD_LOGIC_VECTOR(71 downto 0);

       IOBP_Masken_Reg1 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg2 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg3 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg4 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg5 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg6 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg7 : in STD_LOGIC_VECTOR(15 downto 0);
       PIO_SYNC         : in STD_LOGIC_VECTOR(150 DOWNTO 16);
       Config           : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_ID          : in t_id_array;
       AW_Output_Reg    : in t_IO_Reg_1_to_7_Array;
       quench_sk_out    : in std_logic_vector (4 downto 0);
       AW_Input_Reg     : out t_IO_Reg_1_to_7_Array;
       IOBP_Out_Reg     : out t_IOBP_array;
       
       IOBP_In_Reg      : out t_IOBP_array;
       IOBP_Aktiv_LED   : out t_led_array;
       OUT_SLOT         : out t_IOBP_array;
       ENA_SLOT         : out t_IOBP_array;
       IOBP_Sel_LED     : out t_led_array
);
end front_board_id ;

architecture Arch_front_board_id of front_board_id is
    type   IOBP_slot_state_t is   (IOBP_slot_idle, IOBP_slot1, IOBP_slot2,IOBP_slot3,IOBP_slot4,IOBP_slot5,IOBP_slot6,IOBP_slot7,IOBP_slot8,IOBP_slot9,IOBP_slot10,IOBP_slot11,IOBP_slot12);
    signal IOBP_slot_state:   IOBP_slot_state_t:= IOBP_slot_idle;
    signal quench_det_cnt : integer range 0 to 4 :=0;
    type   t_reg_array         is array (1 to 12) of std_logic_vector(7 downto 0);
    signal conf_reg     :  t_reg_array;
    Signal IOBP_Out     :  t_IOBP_array;

begin
         
    process (clk, nReset)

        begin

      if (not  nReset= '1')    then
          for i in 1 to 12 loop
              conf_reg(i)<= (others => '0' );
              quench_det_cnt <=0;
            
          end loop;

          IOBP_slot_state <= IOBP_slot_idle;

      elsif (clk'EVENT AND clk = '1') then
        
          case IOBP_slot_state is

              when IOBP_slot_idle	=>
                                              IOBP_slot_state <= IOBP_slot1;

              when IOBP_slot1=>			    conf_reg(1)<= IOBP_ID(1);
                                              case conf_reg(1) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 1
                                                      AW_Input_Reg(1)( 5 downto  0) <=   (Deb_Sync( 5 downto  0)   AND not IOBP_Masken_Reg1( 5 downto  0));
                                                      IOBP_Aktiv_LED(1)  <=  Deb_out( 5 DOWNTO 0);   -- Signale für Aktiv-LED's
                                                      IOBP_In_Reg(1)  <= ( PIO_SYNC(56),  PIO_SYNC(60),  PIO_SYNC(62),  PIO_SYNC(52),  PIO_SYNC(54),  PIO_SYNC(58));
                                                      IOBP_Out(1) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(1)   <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 1
                                                      AW_Input_Reg(1)( 5 downto  0) <=  (OTHERS => '0');
                                                      IOBP_Out(1) <= (AW_Output_Reg(1)(5 downto 0) AND not IOBP_Masken_Reg1( 5 downto 0));
                                                      OUT_SLOT(1) <= IOBP_Out(1);
                                                      ENA_SLOT(1)<= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(1)  <=  IOBP_Out(1);
                                                      IOBP_In_Reg(1)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(1)   <=  not ( IOBP_Masken_Reg1( 5 downto 0) );  -- Register für Sel-LED's vom Slave 1

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 1
                                                 
                                                      AW_Input_Reg(1)( 4 downto  0) <=   (Deb_Sync( 4 downto  0) AND not IOBP_Masken_Reg1( 4 downto  0));
                                                      AW_Input_Reg(1)(5) <='0';
                                                      IOBP_In_Reg(1) (4 downto 0) <= ( PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58));
                                                      ENA_SLOT(1) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(1) <=  not ( IOBP_Masken_Reg7( 0) & IOBP_Masken_Reg1( 4 downto  0) );  -- Register für Sel-LED's vom Slave 1
                                                      if  (Config = x"DEDE") then      
                                                        IOBP_Out(1)(5)  <= quench_sk_out(quench_det_cnt); --0
                                                        quench_det_cnt <= quench_det_cnt+1;
                                                      else 
                                                      IOBP_Out(1)(5)  <= (AW_Output_Reg(1)( 5) AND not IOBP_Masken_Reg7( 0));
                                                      end if;
                                                      IOBP_Aktiv_LED(1)  <=  (IOBP_Out(1)(5)    &  Deb_out( 4 DOWNTO  0));  -- Signale für Aktiv-LED's
                                                      OUT_SLOT(1) <= IOBP_out(1);
                                                     
                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot2;

              when IOBP_slot2=>			    conf_reg(2)<= IOBP_ID(2);
                                        case conf_reg(2) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 2
                                                      AW_Input_Reg(1)( 11 downto  6)<=   (Deb_Sync( 11 downto  6)  AND not IOBP_Masken_Reg1( 11 downto  6));
                                                      IOBP_Aktiv_LED(2)  <=   Deb_out(11 DOWNTO 6);   -- Signale für Aktiv-LED's
                                                      IOBP_In_Reg(2)  <=( PIO_SYNC(96),  PIO_SYNC(100), PIO_SYNC(102), PIO_SYNC(92),  PIO_SYNC(94),  PIO_SYNC(98));
                                                      IOBP_Out(2) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(2)   <=  not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 2
                                                      AW_Input_Reg(1)( 11 downto  6) <=  (OTHERS => '0');
                                                      IOBP_Out(2) <= (AW_Output_Reg(1)(11 downto 6) AND not IOBP_Masken_Reg1(11 downto 6));
                                                      OUT_SLOT(2) <= IOBP_Out(2);
                                                      ENA_SLOT(2)<= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(2)  <=  IOBP_Out(2);
                                                      IOBP_In_Reg(2)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(2)   <=  not ( IOBP_Masken_Reg1(11 downto 6) );  -- Register für Sel-LED's vom Slave 2

                                                    when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 2
                                                      AW_Input_Reg(1)( 10 downto  6) <=   (Deb_Sync( 10 downto  6) AND not IOBP_Masken_Reg1( 10 downto  6));  -- Input, IO-Modul Nr. 2
                                                      AW_Input_Reg(1)(11) <='0';
                                                      
                                                      IOBP_In_Reg(2) (4 downto 0) <= ( PIO_SYNC(102), PIO_SYNC(94), PIO_SYNC(100),  PIO_SYNC(92),  PIO_SYNC(98));
                                                      ENA_SLOT(2) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(2)<=    not ( IOBP_Masken_Reg7( 1) & IOBP_Masken_Reg1( 10 downto  6) );  -- Register für Sel-LED's vom Slave 2
                                                      if  (Config = x"DEDE") then      
                                                        IOBP_Out(2)(5)  <= quench_sk_out(quench_det_cnt); --0 1
                                                        quench_det_cnt <= quench_det_cnt+1;
                                                      else 
                                                       IOBP_Out(2)(5)  <= (AW_Output_Reg(1)( 11) AND not IOBP_Masken_Reg7(1));
                                                      end if;
                                                      OUT_SLOT(2)<= IOBP_Out(2);
                                                      IOBP_Aktiv_LED(2)  <=  (IOBP_Out(2)(5)    &  Deb_out( 10 DOWNTO  6));  -- Signale für Aktiv-LED's
                                                      

                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot3;

              when IOBP_slot3=>			    conf_reg(3)<= IOBP_ID(3);
                                            case conf_reg(3) is
                                               when "00000011" | "00000100" => -- Input Modul in slot 3
                                                      AW_Input_Reg(2)( 5 downto  0) <=   (Deb_Sync( 17 downto  12) AND not IOBP_Masken_Reg2( 5 downto  0));
                                                      IOBP_Aktiv_LED(3)  <=   Deb_out(17 DOWNTO 12);   -- Signale für Aktiv-LED's
                                                      IOBP_In_Reg(3)  <=( PIO_SYNC(73),  PIO_SYNC(77),  PIO_SYNC(79),  PIO_SYNC(69),  PIO_SYNC(71),  PIO_SYNC(75));
                                                      IOBP_Out(3) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(3)   <=  not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3

                                                when "00000101"  | "00000110" => -- Output Modul in slot 3
                                                      AW_Input_Reg(2)( 5 downto  0) <=  (OTHERS => '0');
                                                      IOBP_Out(3) <= (AW_Output_Reg(2)(5 downto 0) AND not IOBP_Masken_Reg2(5 downto 0));
                                                      OUT_SLOT(3) <= IOBP_Out(3);
                                                      ENA_SLOT(3)<= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(3)  <=  IOBP_Out(3);
                                                      IOBP_In_Reg(3)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(3)   <=  not ( IOBP_Masken_Reg2( 5 downto 0) );  -- Register für Sel-LED's vom Slave 3

                                                when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 3
                                                      AW_Input_Reg(2)( 4 downto  0) <=   (Deb_Sync( 16 downto  12) AND not IOBP_Masken_Reg2( 4 downto  0));  -- Input, IO-Modul Nr. 3
                                                      AW_Input_Reg(2)(5) <='0';
                                                      IOBP_In_Reg(3) (4 downto 0) <= ( PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75));
                                                      ENA_SLOT(3)<= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(3) <=  not ( IOBP_Masken_Reg7( 2) & IOBP_Masken_Reg2( 4 downto  0) );  -- Register für Sel-LED's vom Slave 3
                                                      if  (Config = x"DEDE") then      
                                                        IOBP_Out(3)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2
                                                        quench_det_cnt <= quench_det_cnt+1;
                                                      else 
                                                        IOBP_Out(3)(5)  <= (AW_Output_Reg(2)( 5) AND not IOBP_Masken_Reg7( 2));
                                                      end if;
                                                      IOBP_Aktiv_LED(3)  <=  (IOBP_Out(3)(5)    &  Deb_out( 16 DOWNTO  12));  -- Signale für Aktiv-LED's
                                                      OUT_SLOT(3) <= IOBP_Out(3);
                                                      

                                                when others     =>  NULL;
                                            end case;

                                              IOBP_slot_state <= IOBP_slot4;

               when IOBP_slot4=>			    conf_reg(4)<= IOBP_ID(4);
                                            case conf_reg(4) is
                                              when "00000011" | "00000100" => -- Input Modul in slot 4
                                                      AW_Input_Reg(2)( 11 downto  6)<=   (Deb_Sync( 23 downto  18) AND not IOBP_Masken_Reg2( 11 downto  6));
                                                      IOBP_Aktiv_LED(4)  <=  Deb_out(23 DOWNTO 18);  -- Signale für Aktiv-LED's
                                                      IOBP_In_Reg(4)  <= ( PIO_SYNC(101), PIO_SYNC(91), PIO_SYNC(93), PIO_SYNC(105), PIO_SYNC(103), PIO_SYNC(89));
                                                      IOBP_Out(4) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(4)   <=  not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 4
                                                      AW_Input_Reg(2)( 11 downto  6) <=  (OTHERS => '0');
                                                      IOBP_Out(4) <= AW_Output_Reg(2)( 11 downto  6) AND not IOBP_Masken_Reg2(11 downto 6);
                                                      OUT_SLOT(4) <= IOBP_Out(4);
                                                      ENA_SLOT(4) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(4)  <=  IOBP_Out(4);
                                                      IOBP_In_Reg(4)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(4)   <=  not ( IOBP_Masken_Reg2(11 downto 6) );  -- Register für Sel-LED's vom Slave 4

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 4
                                                      AW_Input_Reg(2)( 10 downto  6) <=   (Deb_Sync( 22 downto  18) AND not IOBP_Masken_Reg2( 10 downto  6));  -- Input, IO-Modul Nr. 4
                                                      AW_Input_Reg(2)(11) <='0';
                                                      IOBP_In_Reg(4) (4 downto 0) <= ( PIO_SYNC(93), PIO_SYNC(103), PIO_SYNC(91), PIO_SYNC(105), PIO_SYNC(89));
                                                      ENA_SLOT(4) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(4) <=  not ( IOBP_Masken_Reg7( 3) & IOBP_Masken_Reg2( 10 downto  6) );  -- Register für Sel-LED's vom Slave 4
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(4)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(4)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2  
                                                        end if;
                                                      quench_det_cnt <= quench_det_cnt+1;
                                                    else 
                                                      IOBP_Out(4)(5)  <= (AW_Output_Reg(2)( 11) AND not IOBP_Masken_Reg7(3));
                                                    end if;
                                                    OUT_SLOT(4) <= IOBP_Out(4);
                                                    IOBP_Aktiv_LED(4)  <=  (IOBP_Out(4)(5)    &  Deb_out( 22 DOWNTO  18));  -- Signale für Aktiv-LED's
                                                    

                                                  when others     =>  NULL;
                                            end case;

                                              IOBP_slot_state <= IOBP_slot5;

              when IOBP_slot5=>			    conf_reg(5)<= IOBP_ID(5);
                                              case conf_reg(5) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 5
                                                      AW_Input_Reg(3)( 5 downto  0) <=   (Deb_Sync( 29 downto  24) AND not IOBP_Masken_Reg3( 5 downto  0));
                                                      IOBP_Aktiv_LED(5)  <=   Deb_out(29 DOWNTO 24);   -- Signale für Aktiv-LED's
                                                      IOBP_In_Reg(5)  <= ( PIO_SYNC(53),  PIO_SYNC(61),  PIO_SYNC(63),  PIO_SYNC(57),  PIO_SYNC(55),  PIO_SYNC(59));
                                                      IOBP_Out(5) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(5)   <=  not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 5
                                                      AW_Input_Reg(3)( 5 downto  0) <=  (OTHERS => '0');
                                                      IOBP_Out(5) <= AW_Output_Reg(3)(5 downto  0) AND not IOBP_Masken_Reg3(5 downto 0);
                                                      OUT_SLOT(5)<= IOBP_Out(5);
                                                      ENA_SLOT(5) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(5)  <=  IOBP_Out(5);
                                                      IOBP_In_Reg(5)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(5)   <=  not ( IOBP_Masken_Reg3( 5 downto 0) );  -- Register für Sel-LED's vom Slave 5

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 5
                                                      AW_Input_Reg(3)( 4 downto  0) <=   (Deb_Sync( 28 downto  24) AND not IOBP_Masken_Reg3( 4 downto  0));  -- Input, IO-Modul Nr. 5
                                                      AW_Input_Reg(3)(5) <='0';
                                                      IOBP_In_Reg(5) (4 downto 0) <= ( PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59));
                                                      ENA_SLOT(5) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(5) <=  not ( IOBP_Masken_Reg7( 4) & IOBP_Masken_Reg3( 4 downto  0) );  -- Register für Sel-LED's vom Slave 5
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(5)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(5)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3
                                                        end if;                    
                                                          quench_det_cnt <= quench_det_cnt+1;          
                                                      else 
                                                       IOBP_Out(5)(5)  <= (AW_Output_Reg(3)( 5) AND not IOBP_Masken_Reg7(4));
                                                      end if;
                                                      IOBP_Aktiv_LED(5)  <=  (IOBP_Out(5)(5)    &  Deb_out( 28 DOWNTO  24));  -- Signale für Aktiv-LED's
                                                      OUT_SLOT(5) <= IOBP_Out(5);
                                                     


                                              when others     =>  NULL;
                                          end case;

                                              IOBP_slot_state <= IOBP_slot6;

              when IOBP_slot6=>			    conf_reg(6)<= IOBP_ID(6);
                                              case conf_reg(6) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 6
                                                      AW_Input_Reg(3)( 11 downto  6)<=   (Deb_Sync( 35 downto  30) AND not IOBP_Masken_Reg3( 11 downto  6));
                                                      IOBP_Aktiv_LED(6)  <=    Deb_out(35 DOWNTO 30);
                                                      IOBP_In_Reg(6)  <= ( PIO_SYNC(119), PIO_SYNC(109), PIO_SYNC(111), PIO_SYNC(123), PIO_SYNC(121), PIO_SYNC(107));
                                                      IOBP_Out(6) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(6)   <=  not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6

                                              when "00000101"  | "00000110" => -- Output Modul in slot 6
                                                      AW_Input_Reg(3)( 11 downto  6)<=   (OTHERS => '0');
                                                      IOBP_Out(6) <= AW_Output_Reg(3)(11 downto  6) AND not IOBP_Masken_Reg3(11 downto 6);
                                                      OUT_SLOT(6) <= IOBP_Out(6);
                                                      ENA_SLOT(6) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(6)  <=  IOBP_Out(6);
                                                      IOBP_In_Reg(6)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(6)   <=  not ( IOBP_Masken_Reg3(11 downto 6) );  -- Register für Sel-LED's vom Slave 6

                                              when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 6
                                                      AW_Input_Reg(3)( 10 downto  6) <=   (Deb_Sync( 34 downto  30) AND not IOBP_Masken_Reg3( 10 downto  6));  -- Input, IO-Modul Nr. 6
                                                      AW_Input_Reg(3)(11) <='0';
                                                      IOBP_In_Reg(6) (4 downto 0) <= ( PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107));
                                                      ENA_SLOT(6) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(6)<=  not ( IOBP_Masken_Reg7( 5) & IOBP_Masken_Reg3( 10 downto  6) );  -- Register für Sel-LED's vom Slave 6
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(6)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(6)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                          quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                    else 
                                                     IOBP_Out(6)(5)  <= (AW_Output_Reg(3)( 11) AND not IOBP_Masken_Reg7(5));
                                                    end if;
                                                      OUT_SLOT(6)<= IOBP_Out(6);
                                                      IOBP_Aktiv_LED(6)  <=  (IOBP_Out(6)(5) & Deb_out( 34 DOWNTO  30));  -- Signale für Aktiv-LED's
                                                      
                                          when others     =>  NULL;
                                      end case;

                                              IOBP_slot_state <= IOBP_slot7;

              when IOBP_slot7=>			    conf_reg(7)<= IOBP_ID(7);
                                              case conf_reg(7) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 7
                                                      AW_Input_Reg(4)( 5 downto  0) <=   (Deb_Sync( 41 downto  36) AND not IOBP_Masken_Reg4( 5 downto  0));
                                                      IOBP_Aktiv_LED(7)  <=    Deb_out(41 DOWNTO 36);
                                                      IOBP_In_Reg(7)  <= ( PIO_SYNC(35),  PIO_SYNC(43),  PIO_SYNC(45),  PIO_SYNC(39),  PIO_SYNC(37),  PIO_SYNC(41));
                                                      IOBP_Out(7) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(7)   <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7

                                              when "00000101"  | "00000110" => -- Output Modul in slot 7
                                                      AW_Input_Reg(4)( 5 downto  0)<=   (OTHERS => '0');
                                                      IOBP_Out(7) <= AW_Output_Reg(4)(5 downto  0) AND not IOBP_Masken_Reg4(5 downto 0);
                                                      OUT_SLOT(7) <= IOBP_Out(7);
                                                      ENA_SLOT(7) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(7)  <=  IOBP_Out(7);
                                                      IOBP_In_Reg(7)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(7)   <=  not ( IOBP_Masken_Reg4( 5 downto 0) );  -- Register für Sel-LED's vom Slave 7

                                                when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 7
                                                      AW_Input_Reg(4)( 4 downto  0) <=   (Deb_Sync( 40 downto  36) AND not IOBP_Masken_Reg4( 4 downto  0));  -- Input, IO-Modul Nr. 7
                                                      AW_Input_Reg(4)(5) <='0';       
                                                      IOBP_In_Reg(7) (4 downto 0) <= ( PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41));
                                                      ENA_SLOT(7) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(7) <=  not ( IOBP_Masken_Reg7( 6) & IOBP_Masken_Reg4( 4 downto  0) );  -- Register für Sel-LED's vom Slave 7
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(7)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(7)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4  
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                          quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                  else 
                                                      IOBP_Out(7)(5)  <= (AW_Output_Reg(4)( 5) AND not IOBP_Masken_Reg7(6));
                                                  end if;
                                                      OUT_SLOT(7) <= IOBP_Out(7);
                                                      IOBP_Aktiv_LED(7)  <=  (IOBP_Out(7)(5)    &  Deb_out( 40 DOWNTO  36));  -- Signale für Aktiv-LED's

                                          when others     =>  NULL;
                                      end case;

                                              IOBP_slot_state <= IOBP_slot8;

              when IOBP_slot8=>			    conf_reg(8)<= IOBP_ID(8);
                                              case conf_reg(8) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 8
                                                      AW_Input_Reg(4)( 11 downto  6)<=   (Deb_Sync(47 downto  42) AND not IOBP_Masken_Reg4( 11 downto  6));
                                                      IOBP_Aktiv_LED(8)  <=  Deb_out (47 DOWNTO 42);
                                                      IOBP_In_Reg(8)  <= ( PIO_SYNC(137), PIO_SYNC(127), PIO_SYNC(129), PIO_SYNC(141), PIO_SYNC(139), PIO_SYNC(125));
                                                      IOBP_Out(8) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(8)   <=  not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 8
                                                      AW_Input_Reg(4)(11 downto  6)<=   (OTHERS => '0');
                                                      IOBP_Out(8) <= AW_Output_Reg(4)(11 downto  6) AND not IOBP_Masken_Reg4(11 downto 6);
                                                      OUT_SLOT(8) <= IOBP_Out(8);
                                                      ENA_SLOT(8) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(8)  <=  IOBP_Out(8);
                                                      IOBP_In_Reg(8)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(8)   <=  not ( IOBP_Masken_Reg4(11 downto 6) );  -- Register für Sel-LED's vom Slave 8

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 8
                                                      AW_Input_Reg(4)( 10 downto  6) <=   (Deb_Sync( 46 downto  42) AND not IOBP_Masken_Reg4( 10 downto  6));  -- Input, IO-Modul Nr. 8
                                                      AW_Input_Reg(4)(11) <='0';               
                                                      IOBP_In_Reg(8) (4 downto 0) <= ( PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125));
                                                      ENA_SLOT(8)<= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(8) <=  not ( IOBP_Masken_Reg7( 7) & IOBP_Masken_Reg4( 10 downto  6) );  -- Register für Sel-LED's vom Slave 8
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                            IOBP_Out(8)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                            IOBP_Out(8)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                            quench_det_cnt <=0;
                                                        else
                                                            quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                      else 
                                                        IOBP_Out(8)(5)  <= (AW_Output_Reg(4)( 11) AND not IOBP_Masken_Reg7(7));
                                                    end if;
                                                      OUT_SLOT(8) <= IOBP_Out(8);
                                                      IOBP_Aktiv_LED(8)  <=  (IOBP_Out(8)(5)    &  Deb_out( 46 DOWNTO  42));  -- Signale für Aktiv-LED's
                                                     
                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot9;

              when IOBP_slot9=>			    conf_reg(9)<= IOBP_ID(9);
                                              case conf_reg(9) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 9
                                                      AW_Input_Reg(5)( 5 downto  0) <=   (Deb_Sync(53 DOWNTO 48) AND not IOBP_Masken_Reg5( 5 downto  0));
                                                      IOBP_Aktiv_LED(9)  <=    Deb_out(53 DOWNTO 48);
                                                      IOBP_In_Reg(9)  <= ( PIO_SYNC(30),  PIO_SYNC(22),  PIO_SYNC(20),  PIO_SYNC(26),  PIO_SYNC(28),  PIO_SYNC(24));
                                                      IOBP_Out(9) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(9)   <=  not ( IOBP_Masken_Reg5( 5 downto 0) );  -- Register für Sel-LED's vom Slave 9

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 9
                                                      AW_Input_Reg(5)(5 downto  0)<=   (OTHERS => '0');
                                                      IOBP_Out(9) <= AW_Output_Reg(5)(5 downto  0) AND not IOBP_Masken_Reg5(5 downto 0);
                                                      OUT_SLOT(9)<= IOBP_Out(9);
                                                      ENA_SLOT(9) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(9)  <=  IOBP_Out(9); 
                                                      ENA_SLOT(9) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(9) <=  not ( IOBP_Masken_Reg7( 8) & IOBP_Masken_Reg5( 4 downto  0) );  -- Register für Sel-LED's vom Slave 9 
                                                  
                                                      when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 9
                                                      AW_Input_Reg(5)( 4 downto  0) <=   (Deb_Sync( 52 downto  48) AND not IOBP_Masken_Reg5( 4 downto  0));  -- Input, IO-Modul Nr. 9
                                                      AW_Input_Reg(5)(5) <='0';              
                                                      IOBP_In_Reg(9) (4 downto 0) <= ( PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24));
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(9)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(9)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                          quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                else 
                                                      IOBP_Out(9)(5)  <= (AW_Output_Reg(5)(5) AND not IOBP_Masken_Reg7(8));
                                                end if;
                                                      OUT_SLOT(9) <= IOBP_Out(9);
                                                      IOBP_Aktiv_LED(9)  <=  (IOBP_Out(9)(5)    &  Deb_out( 52 DOWNTO 48));  -- Signale für Aktiv-LED's
                                                      
                                                  when others     =>  NULL;
                                              end case;
  ---
                                              IOBP_slot_state <= IOBP_slot10;

              when IOBP_slot10=>			    conf_reg(10)<= IOBP_ID(10);
                                              case conf_reg(10) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 10
                                                      AW_Input_Reg(5)( 11 downto  6) <=   (Deb_Sync(59 DOWNTO 54) AND not IOBP_Masken_Reg5( 11 downto  6));
                                                      IOBP_Aktiv_LED(10)  <=    Deb_out(59 DOWNTO 54);
                                                      IOBP_In_Reg(10)  <= (PIO_SYNC(130), PIO_SYNC(140), PIO_SYNC(138), PIO_SYNC(126), PIO_SYNC(128), PIO_SYNC(142));
                                                      IOBP_Out(10) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 10
                                                      AW_Input_Reg(5)(11 downto  6)<=   (OTHERS => '0');
                                                      IOBP_Out(10) <= AW_Output_Reg(5)(11 downto  6) AND not IOBP_Masken_Reg5(11 downto 6);
                                                      OUT_SLOT(10) <= IOBP_Out(10);
                                                      ENA_SLOT(10) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(10)  <=  IOBP_Out(10);
                                                      IOBP_In_Reg(10)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(10)  <=  not ( IOBP_Masken_Reg5(11 downto 6)  );  -- Register für Sel-LED's vom Slave 10

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 10
                                                      AW_Input_Reg(5)( 10 downto  6) <=   (Deb_Sync( 58 downto  54) AND not IOBP_Masken_Reg5( 10 downto  6));  -- Input, IO-Modul Nr. 10
                                                      AW_Input_Reg(5)(11) <='0';              
                                                      IOBP_In_Reg(10) (4 downto 0) <= (PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142));
                                                      ENA_SLOT(10) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(10) <=  not ( IOBP_Masken_Reg7( 9) & IOBP_Masken_Reg5( 10 downto  6) );  -- Register für Sel-LED's vom Slave 10
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(10)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(10)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4 
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                          quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                       end if;
                                                      else 
                                                        IOBP_Out(10)(5)  <= (AW_Output_Reg(5)( 11) AND not IOBP_Masken_Reg7(9));
                                                      end if;
                                                      OUT_SLOT(10) <= IOBP_Out(10);
                                                      IOBP_Aktiv_LED(10)  <=  (IOBP_Out(10)(5)    &  Deb_out( 58 DOWNTO  54));  -- Signale für Aktiv-LED's
                                                      
                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot11;

              when IOBP_slot11=>			    conf_reg(11)<= IOBP_ID(11);
                                              case conf_reg(11) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 11
                                                      AW_Input_Reg(6)( 5 downto  0) <=   (Deb_Sync(65 DOWNTO 60) AND not IOBP_Masken_Reg6( 5 downto  0));
                                                      IOBP_Aktiv_LED(11)  <=    Deb_out(65 DOWNTO 60);
                                                      IOBP_In_Reg(11)  <= (PIO_SYNC(48),PIO_SYNC(40), PIO_SYNC(38), PIO_SYNC(44), PIO_SYNC(46), PIO_SYNC(42));
                                                      IOBP_Out(11) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(11)  <=  not ( IOBP_Masken_Reg6(5 downto 0) );  -- Register für Sel-LED's vom Slave 11

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 11
                                                      AW_Input_Reg(6)(5 downto  0)<=   (OTHERS => '0');
                                                      IOBP_Out(11) <= AW_Output_Reg(6)(5 downto  0) AND not IOBP_Masken_Reg6(5 downto 0);
                                                      OUT_SLOT(11) <= IOBP_Out(11);
                                                      ENA_SLOT(11) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(11)  <=  IOBP_Out(11);
                                                      IOBP_In_Reg(11)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(11)  <=  not ( IOBP_Masken_Reg6(5 downto 0) );  -- Register für Sel-LED's vom Slave 11

                                                  when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 11
                                                      AW_Input_Reg(6)( 4 downto  0) <=   (Deb_Sync( 64 downto  60) AND not IOBP_Masken_Reg6( 4 downto  0));  -- Input, IO-Modul Nr. 11
                                                      AW_Input_Reg(6)(5) <='0';
                                                      IOBP_In_Reg(11) (4 downto 0) <= ( PIO_SYNC(38), PIO_SYNC(46), PIO_SYNC(40), PIO_SYNC(44), PIO_SYNC(42));
                                                      ENA_SLOT(11) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(11) <=  not ( IOBP_Masken_Reg7( 10) & IOBP_Masken_Reg6( 4 downto  0) );  -- Register für Sel-LED's vom Slave 11
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(11)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(11)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4 
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                          quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                    else 
                                                      IOBP_Out(11)(5)  <= (AW_Output_Reg(6)( 5) AND not IOBP_Masken_Reg7(10));
                                                    end if;
                                                      OUT_SLOT(11) <= IOBP_Out(11);
                                                      IOBP_Aktiv_LED(11)  <=  (IOBP_Out(11)(5)    &  Deb_out( 64 DOWNTO 60));  -- Signale für Aktiv-LED's
                                                      
                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot12;

              when IOBP_slot12=>			    conf_reg(12)<= IOBP_ID(12);
                                              case conf_reg(12) is
                                                  when "00000011" | "00000100" => -- Input Modul in slot 12
                                                      AW_Input_Reg(6)( 11 downto  6) <=   (Deb_Sync(71 DOWNTO 66) AND not IOBP_Masken_Reg6( 11 downto  6));
                                                      IOBP_Aktiv_LED(12)  <=    Deb_out(71 DOWNTO 66);
                                                      IOBP_In_Reg(12)  <= (PIO_SYNC(112),PIO_SYNC(122), PIO_SYNC(120), PIO_SYNC(108), PIO_SYNC(110), PIO_SYNC(124));
                                                      IOBP_Out(12) <=  (OTHERS => '0');
                                                      IOBP_Sel_LED(12)  <=  not ( IOBP_Masken_Reg6( 11 downto 6) );  -- Register für Sel-LED's vom Slave 12

                                                  when "00000101"  | "00000110" => -- Output Modul in slot 12
                                                      AW_Input_Reg(6)(11 downto  6)<=   (OTHERS => '0');
                                                      IOBP_Out(12) <= AW_Output_Reg(6)(11 downto  6) AND not IOBP_Masken_Reg6(11 downto 6);
                                                      OUT_SLOT(12) <= IOBP_Out(12);
                                                      ENA_SLOT(12) <= std_logic_vector'("111111");
                                                      IOBP_Aktiv_LED(12)  <=  IOBP_Out(12);
                                                      IOBP_In_Reg(12)  <= (OTHERS => '0');
                                                      IOBP_Sel_LED(12)  <=  not ( IOBP_Masken_Reg6( 11 downto 6) );  -- Register für Sel-LED's vom Slave 12

                                                      when "00000001"|"00000010" => -- 5 In/1 Out Modul in slot 12
                                                      AW_Input_Reg(6)( 10 downto  6) <=   (Deb_Sync( 70 downto  66) AND not IOBP_Masken_Reg6( 10 downto  6));  -- Input, IO-Modul Nr. 12
                                                      AW_Input_Reg(6)(11) <='0';
                                                      IOBP_In_Reg(12) (4 downto 0) <= ( PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124));
                                                      ENA_SLOT(12) <= std_logic_vector'("100000");
                                                      IOBP_Sel_LED(12) <=  not ( IOBP_Masken_Reg7( 11) & IOBP_Masken_Reg6( 10 downto  6) );  -- Register für Sel-LED's vom Slave 12
                                                      if  (Config = x"DEDE") then      
                                                        if quench_det_cnt = 2 then
                                                          IOBP_Out(12)(5)  <= quench_sk_out(quench_det_cnt-2); --0
                                                        else
                                                          IOBP_Out(12)(5)  <= quench_sk_out(quench_det_cnt); --0 1 2 3 4 
                                                        end if;
                                                        if quench_det_cnt =4 then 
                                                            quench_det_cnt <=0;
                                                        else
                                                          quench_det_cnt <= quench_det_cnt+1;
                                                        end if;
                                                      else 
                                                       IOBP_Out(12)(5)  <= (AW_Output_Reg(6)( 11) AND not IOBP_Masken_Reg7(11));
                                                      end if;
                                                      OUT_SLOT(12) <= IOBP_Out(12);
                                                      IOBP_Aktiv_LED(12)  <=  (IOBP_Out(12)(5)    &  Deb_out( 70 DOWNTO  66));  -- Signale für Aktiv-LED's
                                                      
                                                  when others     =>  NULL;
                                              end case;

                                              IOBP_slot_state <= IOBP_slot_idle;

                   when others =>           IOBP_slot_state <= IOBP_slot_idle;
          end case;

    end if;
   end process ;
   IOBP_Out_Reg    <= IOBP_Out;
end architecture Arch_front_board_id;