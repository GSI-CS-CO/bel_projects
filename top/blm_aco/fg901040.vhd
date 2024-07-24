--TITLE "'fg901040' Autor: R.Hartmann";


library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.fg_quad_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;



ENTITY fg901040 IS
		
	generic
		(
			stretch_cnt:			INTEGER := 5
		);
	port(

    nReset:               in  std_logic;
    Clk:                  in  std_logic;
    Ena_Every_20ms:       in  std_logic;
--    
    AD_Trigger_Mode:      in  STD_LOGIC_VECTOR(1 DOWNTO 0);
    AD_sw_Trigger:        in  std_logic;
    AD_ext_Trigger:       in  std_logic;
    AD_Data:              in  STD_LOGIC_VECTOR(7 DOWNTO 0);
      
    AD_ByteSwap:          out std_logic;
    AD_nCNVST:            out std_logic;
    AD_Reset:             out std_logic;
    AD_nCS:               out std_logic;
    AD_Busy:              in  std_logic;
--    
    AD_Out:               Out STD_LOGIC_VECTOR(15 DOWNTO 0);
    AD_ext_Trigger_nLED:  Out std_logic
		);	
	end fg901040;


  
ARCHITECTURE Arch_fg901040 OF fg901040 IS



    ------------ Single-Puls ----------------------------------------------------------------------------------------------
    
  signal  AD_HB:          STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal  AD_LB:          STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal  AD_HLB:         STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal  ByteSwap:       std_logic;                      -- Umschaltung H/L-Byte
          
  signal  pb_cnt:         integer range 0 to 63;         -- Pulsbreite
  signal  pa_cnt:         integer range 0 to 63;         -- Pulsabstand
  signal  wait_hb_cnt:    integer range 0 to 63;         -- Wait_HB-Data
  signal  wait_lb_cnt:    integer range 0 to 63;         -- Wait_LB-Data
  signal  wait_end_cnt:   integer range 0 to 63;         -- Wait_End
  signal  cc_out:         std_logic;                     -- Ausgangspuls

  type   conv_state_t is   (conv_idle, conv_start, conv_pb_start, conv_pa_start, conv_w_hb, conv_hb, conv_w_lb, conv_lb, conv_w_end, conv_end);

  signal conv_state:        conv_state_t:= conv_idle;


  signal ext_trigger_puls_i:        std_logic;                       -- input  ext. Trigger
  signal ext_trigger_puls_o:        std_logic;                       -- Output Strobe für "state-machine" (1 CLK breit)
  signal ext_trigger_puls_shift:    std_logic_vector(2  downto 0);   -- Shift-Reg.

  signal sw_trigger_puls_i:         std_logic;                       -- input  Software-Trigger
  signal sw_trigger_puls_o:         std_logic;                       -- Output Strobe für "state-machine" (1 CLK breit)
  signal sw_trigger_puls_shift:     std_logic_vector(2  downto 0);   -- Shift-Reg.
  
  

begin



p_led_sel: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk, Sig_in => ext_trigger_puls_o, nLED => AD_ext_Trigger_nLED);-- LED


  

--------- Puls aus ext. Trigger (1 Clock breit) --------------------

p_ext_trigger_puls_Start:  PROCESS (clk, nReset, AD_ext_Trigger)
  BEGin
    IF not nReset  = '1' THEN
      ext_trigger_puls_shift  <= (OTHERS => '0');
      ext_trigger_puls_o    <= '0';

    ELSIF rising_edge(clk) THEN
      ext_trigger_puls_shift <= (ext_trigger_puls_shift(ext_trigger_puls_shift'high-1 downto 0) & (AD_ext_Trigger));

      IF ext_trigger_puls_shift(ext_trigger_puls_shift'high) = '0' AND ext_trigger_puls_shift(ext_trigger_puls_shift'high-1) = '1' THEN
        ext_trigger_puls_o <= '1';
      ELSE
        ext_trigger_puls_o <= '0';
      END IF;
    END IF;
  END PROCESS p_ext_trigger_puls_Start;
  

--------- Puls aus Software-Trigger (1 Clock breit) --------------------

p_sw_trigger_puls_Start:  PROCESS (clk, nReset, AD_sw_Trigger)
  BEGin
    IF not nReset  = '1' THEN
      sw_trigger_puls_shift  <= (OTHERS => '0');
      sw_trigger_puls_o    <= '0';

    ELSIF rising_edge(clk) THEN
      sw_trigger_puls_shift <= (sw_trigger_puls_shift(sw_trigger_puls_shift'high-1 downto 0) & (AD_sw_Trigger));

      IF sw_trigger_puls_shift(sw_trigger_puls_shift'high) = '0' AND sw_trigger_puls_shift(sw_trigger_puls_shift'high-1) = '1' THEN
        sw_trigger_puls_o <= '1';
      ELSE
        sw_trigger_puls_o <= '0';
      END IF;
    END IF;
  END PROCESS p_sw_trigger_puls_Start;
    

  
  
--=============================================================================================================================

P_S_SM:  process (clk, nReset,  conv_state)
                      
    begin
      if (nReset = '0') then

          conv_state   <= conv_idle;
          cc_out       <= '0';              -- Ausgangspuls = 0
          ByteSwap     <= '0';              -- Umschaltung H/L-Byte
        
        
        
  ELSIF rising_edge(clk) then

     
---------------------------------------------------------- Puls 'n' ---------------------------------------------------------
      case conv_state is

        when conv_idle     =>   pa_cnt        <= 31;                -- 31 x 8ns = Pulsabstand
                                pb_cnt        <= 2;                 -- Pulsbreite
                                wait_hb_cnt   <= 12;                -- Wait_HB-Data ready (12 = 96ns)
                                wait_lb_cnt   <= 12;                -- Wait_LB-Data ready
 --                             wait_hb_cnt   <= 2;                 -- Wait_HB-Data ready
 --                             wait_lb_cnt   <= 2;                 -- Wait_LB-Data ready
                                wait_end_cnt  <= 10;                -- Wait_End
                                cc_out        <= '0';               -- Ausgangspuls
                                ByteSwap      <= '0';               -- Umschaltung H/L-Byte ('0'= H-Byte)

                                
--              +================+==================================================
--              |                | 10 =  Software/TAG (AW_Output_Reg(1)(0))         
--              | Trigger-Mode:  | 01 =  ext. Trigger                               
--              |                | 00 =  autom. Conversion alle 460ns (Default)    
--              +================+==================================================


--                                IF    AD_Trigger_Mode        =  B"00"  THEN 
--                                         conv_state         <=  conv_start;
--                                ELSIF AD_Trigger_Mode        =  B"01"  THEN 
--                                      IF ext_trigger_puls_o  =  '1' THEN      -- ex. Trigger
--                                         conv_state         <=  conv_start;
--                                      end if;   
--                                ELSIF AD_Trigger_Mode        =  B"10"  THEN 
--                                      IF sw_trigger_puls_o   =  '1' THEN      -- Software-Trigger
--                                         conv_state         <=  conv_start;
--                                      end if;   
                                IF    AD_Trigger_Mode        =  B"00"  THEN 
                                         conv_state         <=  conv_start;
                                ELSIF ((AD_Trigger_Mode      =  B"01") and  (ext_trigger_puls_o = '1')) THEN      -- ex. Trigger
                                         conv_state         <=  conv_start;
                                ELSIF ((AD_Trigger_Mode      =  B"10") and  (sw_trigger_puls_o  = '1')) THEN      -- Software-Trigger
                                         conv_state         <=  conv_start;
                                else
                                  conv_state    <=  conv_idle;   
                                end if;   

  
        when conv_start    =>   cc_out        <= '1';              -- Ausgangspuls = 1
                                conv_state    <=  conv_pb_start;   
 

                --============== Pulsbreite ==============             
               
        when conv_pb_start  =>  IF pb_cnt        =  0  THEN 
                                  cc_out        <= '0';              -- Ausgangspuls = 0
                                  conv_state    <=  conv_pa_start;
                                else
                                  pb_cnt        <=  (pb_cnt -1);   -- 
                                  conv_state    <=  conv_pb_start;   
                                end if;   
      
                --============== Pulsabstand ==============             
               
        when conv_pa_start  =>  IF pa_cnt        =  0  THEN 
                                  conv_state    <=  conv_w_lb;
                                else
                                  pa_cnt        <=  (pa_cnt -1);   -- 
                                  conv_state    <=  conv_pa_start;   
                                end if;   
       
                --============== Wait_Data-LB ready ==============             
               
        when conv_w_lb       =>  IF wait_lb_cnt      =  0  THEN 
                                   conv_state       <=  conv_lb;
                                 else 
                                   wait_lb_cnt      <=  (wait_lb_cnt -1);   -- 
                                   conv_state       <=  conv_w_lb;   
                                 end if;   

                --============== Data L-Byte ==============             
               
        when conv_lb         =>  AD_LB        <=  AD_Data;                -- Read L-Byte
                                 conv_state   <=  conv_w_hb;

                                
                --============== Wait_Data-HB ready ==============             
               
        when conv_w_hb       =>  ByteSwap           <= '1';               -- Umschaltung H/L-Byte ('0'= L-Byte)
                                 IF wait_hb_cnt      =  0  THEN 
                                   conv_state       <=  conv_hb;
                                 else 
                                   wait_hb_cnt      <=  (wait_hb_cnt -1); -- 
                                   conv_state       <=  conv_w_hb;   
                                 end if;   
                                

                --============== Data HByte ==============             
               
        when conv_hb         =>  AD_HB        <=  AD_Data;                -- Read H-Byte
                                 conv_state   <=  conv_w_end;

 

                --============== Wait_End ==============             
               
        when conv_w_end      =>  ByteSwap           <= '0';               -- Umschaltung H/L-Byte ('0'= L-Byte)
        
                                 IF wait_end_cnt     =  0  THEN 
                                   conv_state       <=  conv_end;
                                 else 
                                   wait_end_cnt     <=  (wait_end_cnt -1);  -- wait
                                   conv_state       <=  conv_w_end;   
                                 end if;   

                --============== Ende ==============                 
    
        when conv_end       =>   AD_HLB(15 DOWNTO 8)  <=  not AD_HB;        -- inv. Eingangsbeschaltung
                                 AD_HLB( 7 DOWNTO 0)  <=  not AD_LB;        -- inv. Eingangsbeschaltung
              
                                 conv_state   <= conv_idle;
            
       when others          =>   conv_state   <= conv_idle;
      end case;


    end if;
  end process P_S_SM;


  
  
    AD_nCS        <=    '0';          -- nCS für den ADC
    AD_Reset      <=    not nReset;   -- ADC-Reset
    AD_nCNVST     <=    not cc_out;   -- ADC-nSTCNV

    AD_ByteSwap   <=    ByteSwap;     -- Umschaltung H/L-Byte ('0'= H-Byte)
--
    AD_Out        <=    AD_HLB;       -- 16 Bit Istwert
    
end Arch_fg901040;













        
                           