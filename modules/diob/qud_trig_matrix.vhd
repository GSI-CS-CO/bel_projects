
----------------------------------------------------------------------------------------------------- 
 -- Prototype matrix test configuration 				
--
--		3 x [5 electrical inputs and 1 electrical output]
--		2 x [5 optical inputs and 1 optical outputs]	  
---------------------------------------------------------------------------------------------------
 --		5 optical inputs and 1 optical outputs card			|-SUB- Piggy-ID	 00000001
 --		5 electrical inputs and 1 electrical output card	|-SUB- Piggy-ID	 00000010
-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
-- Expected matrix configurations
----------------------------------------------------------------------------------------------------- 
--		6 electrical inputs									|-SUB- Piggy-ID  00000011
--		6 optical inputs									|-SUB- Piggy-ID  00000100
--		6 optical outputs									|-SUB- Piggy-ID  00000101

----------------------------------------------------------------------------------------------------- 
-- 4 new matrix configurations:
----------------------------------------------------------------------------------------------------- 
 -- STANDARD MATRIX 										
 --		9 x [6 electrical inputs] 
 --		3 x [6 optical outputs]
----------------------------------------------------------------------------------------------------- 
 --	MIXED INPUT MATRIX 										
 --		7 x [6 electrical inputs]
 --		2 x [6 optical inputs]  
 --		3 x [6 optical outputs]						
----------------------------------------------------------------------------------------------------- 
 -- OPTICAL MATRIX 											
 --		9 x [6 optical inputs] 
 --		1 x [6 optical outputs]
----------------------------------------------------------------------------------------------------- 

-----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity qud_trig_matrix is

PORT
(
clk : in std_logic;
  nReset: in std_logic;
  slave1_ID: in std_logic_vector(7 downto 0);
  slave2_ID: in std_logic_vector(7 downto 0);
  slave3_ID: in std_logic_vector(7 downto 0);
  slave4_ID: in std_logic_vector(7 downto 0);
  slave5_ID: in std_logic_vector(7 downto 0);
  slave6_ID: in std_logic_vector(7 downto 0);
  slave7_ID: in std_logic_vector(7 downto 0);
  slave8_ID: in std_logic_vector(7 downto 0);
  slave9_ID: in std_logic_vector(7 downto 0);
  slave10_ID: in std_logic_vector(7 downto 0);
  slave11_ID: in std_logic_vector(7 downto 0);
  slave12_ID: in std_logic_vector(7 downto 0);
  Trigger_matrix_Config:out  std_logic_vector(7 downto 0) -- maximum 5!= 120 theoretically possible configurations (01111000)
);
end qud_trig_matrix;


architecture qud_trig_matrix_arch of qud_trig_matrix is

type   t_reg_array         is array (1 to 12) of std_logic_vector(7 downto 0);
signal conf_reg:           t_reg_array; 
signal  IN_LEMO_prot_cnt: integer range 0 to 12; 
signal  IN_OPT_prot_cnt: integer range 0 to 12; 
signal  IN_LEMO_cnt: integer range 0 to 12; 
signal  IN_OPT_I_cnt: integer range 0 to 12; 
signal  IN_OPT_o_cnt: integer range 0 to 12; 
type   IOBP_slot_check_state_t is   (IOBP_slot_check_idle, IOBP_slot1, IOBP_slot2,IOBP_slot3,IOBP_slot4,IOBP_slot5,IOBP_slot6,IOBP_slot7,IOBP_slot8,IOBP_slot9,IOBP_slot10,IOBP_slot11,IOBP_slot12,IOBP_slot_check_end);
signal IOBP_slot_check_state:   IOBP_slot_check_state_t:= IOBP_slot_check_idle;
signal who_I_am: std_logic_vector(7 downto 0);

begin
	


Matrix_configuration_proc: process (clk, nReset)

begin
 
	if (not  nReset= '1') then
		IN_LEMO_cnt <=0;
		IN_OPT_I_cnt <=0;
        IN_OPT_O_cnt <=0;
        who_I_am <=(others =>'0');

        for i in 1 to 12 loop
            conf_reg(i)<= (others => '0' );
        end loop; 

        IOBP_slot_check_state <= IOBP_slot_check_idle;

    elsif (clk'EVENT AND clk = '1') then
        

        case IOBP_slot_check_state is
            when IOBP_slot_check_idle	=>  IN_LEMO_cnt <=0;
                                            IN_OPT_I_cnt <=0;
                                            IN_OPT_O_cnt <=0;
    
                                            IOBP_slot_check_state <= IOBP_slot1;

            when IOBP_slot1=>			    conf_reg(1)<= slave1_ID;
                                            case conf_reg(1) is
                                                when "00000001" => 
                                                                    IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;
                        
                                                when "00000010" => 
                                                                   IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;
                        
                                                when "00000011" => 
                                                                   IN_LEMO_cnt <= IN_LEMO_cnt +1;
                        
                                                when "00000100" => 
                                                                   IN_OPT_I_cnt <= IN_OPT_I_cnt +1;
                        
                                                when "00000101" => 	
                                                                   IN_OPT_O_cnt <= IN_OPT_O_cnt +1;
                        
                                                when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot2;

            when IOBP_slot2=>			    conf_reg(2)<= slave2_ID;
                                            case conf_reg(2) is
                                                when "00000001" => 
                                                    IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;
    
                                                when "00000010" => 
                                                    IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;
    
                                                when "00000011" => 
                                                    IN_LEMO_cnt <= IN_LEMO_cnt +1;
    
                                                when "00000100" => 
                                                    IN_OPT_I_cnt <= IN_OPT_I_cnt +1;
    
                                                when "00000101" => 	
                                                    IN_OPT_O_cnt <= IN_OPT_O_cnt +1;
    
                           
                        
                                                when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot3;

            when IOBP_slot3=>			    conf_reg(3)<= slave3_ID;
                                            case conf_reg(3) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot4;

             when IOBP_slot4=>			    conf_reg(4)<= slave4_ID;
                                            case conf_reg(4) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot5;

            when IOBP_slot5=>			    conf_reg(5)<= slave5_ID;
                                            case conf_reg(5) is
                                                when "00000001" => 
                                                        IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;
    
                                                when "00000010" => 
                                                        IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;
    
                                                when "00000011" => 
                                                        IN_LEMO_cnt <= IN_LEMO_cnt +1;
    
                                                when "00000100" => 
                                                        IN_OPT_I_cnt <= IN_OPT_I_cnt +1;
    
                                                when "00000101" => 	
                                                        IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                                                when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot6;

            when IOBP_slot6=>			    conf_reg(6)<= slave6_ID;
                                            case conf_reg(6) is
                                                when "00000001" => 
                                                    IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;
    
                                                when "00000010" => 
                                                    IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;
    
                                                when "00000011" => 
                                                    IN_LEMO_cnt <= IN_LEMO_cnt +1;
    
                                                when "00000100" => 
                                                    IN_OPT_I_cnt <= IN_OPT_I_cnt +1;
    
                                                 when "00000101" => 	
                                                    IN_OPT_O_cnt <= IN_OPT_O_cnt +1;
    
                            
                        
                                                when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot7;

            when IOBP_slot7=>			    conf_reg(7)<= slave7_ID;
                                            case conf_reg(7) is
                                                when "00000001" => 
                                                    IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;
    
                                                when "00000010" => 
                                                    IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;
    
                                                when "00000011" => 
                                                    IN_LEMO_cnt <= IN_LEMO_cnt +1;
    
                                                when "00000100" => 
                                                    IN_OPT_I_cnt <= IN_OPT_I_cnt +1;
    
                                                when "00000101" => 	
                                                    IN_OPT_O_cnt <= IN_OPT_O_cnt +1;
    
                           
                        
                                                when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot8;

            when IOBP_slot8=>			    conf_reg(8)<= slave8_ID;
                                            case conf_reg(8) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL;  
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot9;

            when IOBP_slot9=>			    conf_reg(9)<= slave9_ID;
                                            case conf_reg(9) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL; 
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot10;

            when IOBP_slot10=>			    conf_reg(10)<= slave10_ID;
                                            case conf_reg(10) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL; 
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot11;

            when IOBP_slot11=>			    conf_reg(11)<= slave11_ID;
                                            case conf_reg(11) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL;
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot12;

            when IOBP_slot12=>			    conf_reg(12)<= slave12_ID;
                                            case conf_reg(12) is
                                                when "00000001" => 
                                                IN_OPT_prot_cnt <= IN_OPT_prot_cnt +1;

                                            when "00000010" => 
                                                IN_LEMO_prot_cnt <= IN_LEMO_prot_cnt +1;

                                            when "00000011" => 
                                                IN_LEMO_cnt <= IN_LEMO_cnt +1;

                                            when "00000100" => 
                                                IN_OPT_I_cnt <= IN_OPT_I_cnt +1;

                                            when "00000101" => 	
                                                IN_OPT_O_cnt <= IN_OPT_O_cnt +1;

                       
                    
                                            when others     =>  NULL;   
                                            end case;
                                            IOBP_slot_check_state <= IOBP_slot_check_end;
            
                when IOBP_slot_check_end => if (IN_LEMO_prot_cnt=0) and (IN_OPT_prot_cnt=0) and (IN_LEMO_cnt=0) and (IN_OPT_I_cnt=0) and (IN_opt_O_cnt =0) then 
                                                who_I_am <= "00000000";
                                            else     
                
                                            if (IN_LEMO_prot_cnt /=0) or (IN_OPT_prot_cnt /=0)  then 
                                                if IN_LEMO_prot_cnt = 3 and IN_OPT_prot_cnt =2 then 
                                                    who_I_am <= "00000001";
                                                else
                                                    who_I_am <= "00000110";-- other proto configuration Matrix
                                                end if;
                                            
                                            else  
                                                if (IN_LEMO_cnt /=0) or (IN_OPT_I_cnt /=0) or (IN_opt_O_cnt /=0) then     
                                                    if IN_LEMO_cnt=9 and IN_opt_O_cnt = 3 then
                                                        who_I_am<= "00000010"; --standard Matrix
                                                    else 
                                                        if IN_LEMO_cnt=0 and IN_opt_I_cnt = 9 and IN_opt_O_cnt =1 then 
                                                            who_I_am  <= "00000011";--optical Matrix
                                                        else
                                                            if IN_LEMO_cnt=7 and IN_opt_I_cnt = 2 and IN_opt_O_cnt =3 then
                                                                who_I_am  <= "00000100";--mixed input Matrix  
                                                            else  
                                                                who_I_am  <= "00000101";-- other new configuration Matrix 
                                                            end if;
                                                        end if;
                                                    end if;
                                                end if;
                                            end if;
                                            end if;

                                            IOBP_slot_check_state <= IOBP_slot_check_idle;

                 when others =>           IOBP_slot_check_state <= IOBP_slot_check_idle;
        end case;
		
	end if;
    end process Matrix_configuration_proc; 
    Trigger_matrix_Config  <= who_I_am;
end architecture qud_trig_matrix_arch;