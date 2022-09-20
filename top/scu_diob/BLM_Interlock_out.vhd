LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity BLM_Interlock_out is

        
port (
        CLK              : in std_logic;      -- Clock
        nRST             : in std_logic;      -- Reset
        out_mux_sel      : in std_logic_vector(31 downto 0);
        UP_OVERFLOW      : in t_counter_in_Array ; 
        DOWN_OVERFLOW    : in t_counter_in_Array  ; 
        gate_error       : in std_logic_vector(11 downto 0);
        Interlock_IN     : in std_logic_vector(53 downto 0);
        INTL_Output      : out std_logic_vector(5 downto 0);
        BLM_status_Reg : out std_logic_vector(15 downto 0)
);
end BLM_Interlock_out;

architecture rtl of BLM_Interlock_out is
--out_mux_sel 0 for gate error
--out_mux_sel 4..1 for interlock_in
--out_mux_sel 12..6 for input:
-- if out_mux_sel 5 ='0' up_overflow_control
-- if out_mux_sel 5 ='1' down_overflow_control




      signal in_overflow: std_logic_vector(511 downto 0);
      signal status_Reg: std_logic_vector(15 downto 0);
      signal out_mux_res: std_logic_vector(5 downto 0);
      signal gate_err_res: std_logic_vector(5 downto 0);
      signal intl_wd_res: std_logic_vector(5 downto 0);
      signal overflow: std_logic_vector(5 downto 0);
      begin
     
         
         mux_in_process:process (nRST, CLK)
          begin
          if not nRST='1' then 
                in_overflow <=   (OTHERS =>  '0');
                gate_err_res <=   (OTHERS =>  '0');
                intl_wd_res <=   (OTHERS =>  '0');
                status_Reg <= (OTHERS =>'0');
                overflow <= (OTHERS =>'0');
                out_mux_res <= (OTHERS =>'0');

            
         elsif (CLK'EVENT AND CLK = '1') then 

                --gate_Error_control
                   if out_mux_sel(0) ='0' then 
                        gate_err_res <= gate_error(5 downto 0);
                   else                        
                        gate_err_res <= gate_error(11 downto 6);
                   end if;

                   case out_mux_sel(4 downto 1) is

                        when "0000" => intl_wd_res <= Interlock_IN(5 downto 0);
                        when "0001" => intl_wd_res <= Interlock_IN(11 downto 6);
                        when "0010" => intl_wd_res <= Interlock_IN(17 downto 12);
                        when "0011" => intl_wd_res <= Interlock_IN(23 downto 18);
                        when "0100" => intl_wd_res <= Interlock_IN(29 downto 24);
                        when "0101" => intl_wd_res <= Interlock_IN(35 downto 30);
                        when "0110" => intl_wd_res <= Interlock_IN(41 downto 36);
                        when "0111" => intl_wd_res <= Interlock_IN(47 downto 42);
                        when "1000" => intl_wd_res <= Interlock_IN(53 downto 48);

                        when others => NULL;
                  end case;
                  
                if gate_err_res ="000000" AND intl_wd_res = "000000"  then

                  if out_mux_sel(6)='0' then 
                        in_overflow <= UP_OVERFLOW(7)&UP_OVERFLOW(6)&UP_OVERFLOW(5)&UP_OVERFLOW(4)&UP_OVERFLOW(3)&UP_OVERFLOW(2)&UP_OVERFLOW(1)&UP_OVERFLOW(0) ;
                  else
           --down_overflow_control
                        in_overflow <= DOWN_OVERFLOW(7)& DOWN_OVERFLOW(6)& DOWN_OVERFLOW(5)& DOWN_OVERFLOW(4)& DOWN_OVERFLOW(3)& DOWN_OVERFLOW(2)& DOWN_OVERFLOW(1) & DOWN_OVERFLOW(0);
                 end if;  

                 case out_mux_sel(13 downto 7) is
                        when "0000000" => overflow <= in_overflow(5 downto 0);
                        when "0000001" => overflow <= in_overflow(11 downto 6);
                        when "0000010" => overflow <= in_overflow(17 downto 12);
                        when "0000011" => overflow <= in_overflow(23 downto 18);
                        when "0000100" => overflow <= in_overflow(29 downto 24);
                        when "0000101" => overflow <= in_overflow(35 downto 30);
                        when "0000110" => overflow <= in_overflow(41 downto 36);
                        when "0000111" => overflow <= in_overflow(47 downto 42);
                        when "0001000" => overflow <= in_overflow(53 downto 48);
                        when "0001001" => overflow <= in_overflow(59 downto 54);
                        when "0001010" => overflow <= in_overflow(65 downto 60);
                        when "0001011" => overflow <= in_overflow(71 downto 66);
                        when "0001100" => overflow <= in_overflow(77 downto 72);
                        when "0001101" => overflow <= in_overflow(83 downto 78);
                        when "0001110" => overflow <= in_overflow(89 downto 84);
                        when "0001111" => overflow <= in_overflow(95 downto 90);
                        when "0010000" => overflow <= in_overflow(101 downto 96);
                        when "0010001" => overflow <= in_overflow(107 downto 102);
                        when "0010010" => overflow <= in_overflow(113 downto 108);
                        when "0010011" => overflow <= in_overflow(119 downto 114);
                        when "0010100" => overflow <= in_overflow(125 downto 120);
                        when "0010101" => overflow <= in_overflow(131 downto 126);
                        when "0010110" => overflow <= in_overflow(137 downto 132);
                        when "0010111" => overflow <= in_overflow(143 downto 138);
                        when "0011000" => overflow <= in_overflow(149 downto 144);
                        when "0011001" => overflow <= in_overflow(155 downto 150);
                        when "0011010" => overflow <= in_overflow(161 downto 156);
                        when "0011011" => overflow <= in_overflow(167 downto 162);
                        when "0011100" => overflow <= in_overflow(173 downto 168);
                        when "0011101" => overflow <= in_overflow(179 downto 174);
                        when "0011110" => overflow <= in_overflow(185 downto 180);
                        when "0011111" => overflow <= in_overflow(191 downto 186);
                        when "0100000" => overflow <= in_overflow(197 downto 192);
                        when "0100001" => overflow <= in_overflow(203 downto 198);
                        when "0100010" => overflow <= in_overflow(209 downto 204);
                        when "0100011" => overflow <= in_overflow(215 downto 210);
                        when "0100100" => overflow <= in_overflow(221 downto 216);
                        when "0100101" => overflow <= in_overflow(227 downto 222);
                        when "0100110" => overflow <= in_overflow(233 downto 228);
                        when "0100111" => overflow <= in_overflow(239 downto 234);
                        when "0101000" => overflow <= in_overflow(245 downto 240);
                        when "0101001" => overflow <= in_overflow(251 downto 246);
                        when "0101010" => overflow <= in_overflow(257 downto 252);
                        when "0101011" => overflow <= in_overflow(263 downto 258);
                        when "0101100" => overflow <= in_overflow(269 downto 264);
                        when "0101101" => overflow <= in_overflow(275 downto 270);
                        when "0101110" => overflow <= in_overflow(281 downto 276);
                        when "0101111" => overflow <= in_overflow(287 downto 282);
                        when "0110000" => overflow <= in_overflow(293 downto 288);
                        when "0110001" => overflow <= in_overflow(299 downto 294);
                        when "0110010" => overflow <= in_overflow(305 downto 300);
                        when "0110011" => overflow <= in_overflow(311 downto 306);
                        when "0110100" => overflow <= in_overflow(317 downto 312);
                        when "0110101" => overflow <= in_overflow(323 downto 318);
                        when "0110110" => overflow <= in_overflow(329 downto 324);
                        when "0110111" => overflow <= in_overflow(335 downto 330);
                        when "0111000" => overflow <= in_overflow(341 downto 336);
                        when "0111001" => overflow <= in_overflow(347 downto 342);
                        when "0111010" => overflow <= in_overflow(353 downto 348);
                        when "0111011" => overflow <= in_overflow(359 downto 354);
                        when "0111100" => overflow <= in_overflow(365 downto 360);
                        when "0111101" => overflow <= in_overflow(371 downto 366);
                        when "0111110" => overflow <= in_overflow(377 downto 372);
                        when "0111111" => overflow <= in_overflow(383 downto 378);
                        when "1000000" => overflow <= in_overflow(389 downto 384);
                        when "1000001" => overflow <= in_overflow(395 downto 390);
                        when "1000010" => overflow <= in_overflow(401 downto 396);
                        when "1000011" => overflow <= in_overflow(407 downto 402);
                        when "1000100" => overflow <= in_overflow(413 downto 408);
                        when "1000101" => overflow <= in_overflow(419 downto 414);
                        when "1000110" => overflow <= in_overflow(425 downto 420);
                        when "1000111" => overflow <= in_overflow(431 downto 426);
                        when "1001000" => overflow <= in_overflow(437 downto 432);
                        when "1001001" => overflow <= in_overflow(443 downto 438);
                        when "1001010" => overflow <= in_overflow(449 downto 444);
                        when "1001011" => overflow <= in_overflow(455 downto 450);
                        when "1001100" => overflow <= in_overflow(461 downto 456);
                        when "1001101" => overflow <= in_overflow(467 downto 462);
                        when "1001110" => overflow <= in_overflow(473 downto 468);
                        when "1001111" => overflow <= in_overflow(479 downto 474);
                        when "1010000" => overflow <= in_overflow(485 downto 480);
                        when "1010001" => overflow <= in_overflow(491 downto 486);
                        when "1010010" => overflow <= in_overflow(497 downto 492);
                        when "1010011" => overflow <= in_overflow(503 downto 498);
                        when "1010100" => overflow <= in_overflow(509 downto 504);
                        when "1010101" => overflow <= "0000" & in_overflow(511 downto 510);
                        when others => NULL;

                        end case;
                       -- status_Reg for overflow 
                  status_Reg(15 downto 14)<= (others => '0');
                  status_Reg(13 downto 7)<= out_mux_sel(13 downto 7);
                  status_Reg(6)<=  '0';
                  status_Reg(5 downto 0) <= overflow;           
                  out_mux_res <= overflow;
                else
                  if gate_err_res = "000000" then
      

                -- if no gate error we check the wachdog results
                -- watchdog_Error_control
     
         -- status_Reg for watchdog Error Control 
                  status_Reg(15 downto 11)<= (others => '0');
                  status_Reg(10 downto 7)<= out_mux_sel(4 downto 1);
                  status_Reg(6)<=  '0';
                  status_Reg(5 downto 0) <= intl_wd_res;
                  out_mux_res <= intl_wd_res;
                 else
                        
                 status_Reg(15 downto 7)<= (others => '0');
                 status_Reg(6) <= out_mux_sel(0);
                 status_Reg(5 downto 0) <= gate_err_res;
                 out_mux_res <= gate_err_res;
                 end if;
                end if;
         end if;

        end process;



        INTL_Output <= out_mux_res;
        BLM_status_Reg  <= status_Reg; -- I send a copy of the interlock output to the status register?
end rtl;
