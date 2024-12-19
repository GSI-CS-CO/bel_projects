library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.scu_diob_pkg.all;
library work;

entity front_cards_control is
    port (
      
        
        check_enable: in STD_LOGIC_VECTOR(11 downto 0);
        Effective_ID : in  t_id_array;
        Expected_ID : in  std_logic_vector(95 downto 0);
       conf_reg:  out t_id_array   --(1 to 12) of std_logic_vector(7 downto 0);
        );
    end front_cards_control;

architecture rtl of front_cards_control is
    
 
    signal config_reg: t_id_array;

    begin 

    process (check_enable, Effective_ID,Expected_ID) 
    begin
      
            for i in 0 to 11 loop
                if check_enable (i) ='1' then
                    if to_integer (unsigned(Effective_ID(i+1))) = to_integer(unsigned(Expected_ID(7+ 8*i downto 8*i))) then 
                        config_reg(i+1) <= Effective_ID(i+1);
                    else
                        config_reg(i+1) <= (others =>'0');
                    end if;
                else
                    config_reg(i+1) <= Effective_ID(i+1);
                end if;
            end loop;
 
  
    end process;
    conf_reg <= config_reg;
end architecture rtl;



  --  IOBP_expected_ID <= IOBP_Exp_ID_Reg6 & IOBP_Exp_ID_Reg5 & IOBP_Exp_ID_Reg4 & IOBP_Exp_ID_Reg3 & IOBP_Exp_ID_Reg2 & IOBP_Exp_ID_Reg1;
--Expected_ID <= to_integer(signed (Expected_ID));
--Effective_ID <=IOBP_ID(12) & IOBP_ID(11) &IOBP_ID(10) & IOBP_ID(9) &IOBP_ID(8) & IOBP_ID(7) &IOBP_ID(6) & IOBP_ID(5) &IOBP_ID(4) & IOBP_ID(3) &IOBP_ID(2) & IOBP_ID(1);
