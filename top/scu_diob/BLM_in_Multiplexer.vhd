library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.scu_diob_pkg.all;

entity BLM_In_Multiplexer is

port(
  clk_i : in std_logic;          -- chip-internal pulsed clk signal
  rstn_i : in std_logic;        -- reset signal
  AW_IOBP_Input_Reg:  in  t_IO_Reg_1_to_7_Array;
  watchdog_ena  : in std_logic_vector( 8 downto 0);
  In_Mtx        : out t_in_array;
  INTL_out      : out t_in_array
);
end BLM_In_Multiplexer;

architecture rtl of BLM_In_Multiplexer is

  component BLM_watchdog is
      generic (

          hold    : integer range 1 TO 10:= 2
        
      );
      port(
          clk_i : in std_logic;     -- chip-internal pulsed clk signal
          rstn_i : in std_logic;   -- reset signal
          in_watchdog : in std_logic;     -- input signal
          ena_i : in std_logic;     -- enable '1' for input connected to the counter
          INTL_out: out std_logic   -- interlock output for signal that doesn't change for a given time 
      
      );
 end component BLM_watchdog;
      
 signal in_wd: t_in_array;  
 signal out_wd: t_in_array;  
 signal in_to_mux: t_in_array;   

       
 begin
   
   in_multiplexer_proc: process (rstn_i, clk_i)  
   
    begin
       if not rstn_i='1' then 
         for i in 0 to 8 loop
           in_wd(i) <= (others =>'0');
           in_to_mux(i) <= (others =>'0');
          
         end loop;
       
       elsif (clk_i'EVENT AND clk_i = '1') then
       
         in_wd(0) <= AW_IOBP_Input_Reg(1)(5 downto 0);
         in_wd(1) <= AW_IOBP_Input_Reg(1)(11 downto 6);	
         in_wd(2) <= AW_IOBP_Input_Reg(2)(5 downto 0);
         in_wd(3) <= AW_IOBP_Input_Reg(2)(11 downto 6);
         in_wd(4) <= AW_IOBP_Input_Reg(3)(5 downto 0);
         in_wd(5) <= AW_IOBP_Input_Reg(3)(11 downto 6);
         in_wd(6) <= AW_IOBP_Input_Reg(4)(5 downto 0);
         in_wd(7) <= AW_IOBP_Input_Reg(4)(11 downto 6);
         in_wd(8) <= AW_IOBP_Input_Reg(5)(5 downto 0);

         for n in 0 to 8 loop
          
          for m in 0 to 5 loop
            
            if out_wd(n)(m) ='0' then 
              in_to_mux (n)(m) <= in_wd(n)(m);
              INTL_out(n)(m) <= '0';
            else
              INTL_out(n)(m) <= '1';
          
            end if;

          end loop;

        end loop;
      end if;
         
      end process;

    watchdog_module: for j in 0 to 8 generate 

    begin

       wd_elem_gen: for i in 0 to 5 generate

        input_Watchdog: BLM_watchdog
          generic map(
         
            hold   => 2
         
            )
          port map(
            clk_i => clk_i,  
            rstn_i => rstn_i,   -- reset signal
            in_watchdog => in_wd(j)(i),
            ena_i =>    watchdog_ena(j),  -- enable for input connected to the counter
            INTL_out =>   out_wd(j)(i));
      end generate wd_elem_gen;
   end generate   watchdog_module;
      

 IN_Mtx <= in_to_mux;
end architecture;
