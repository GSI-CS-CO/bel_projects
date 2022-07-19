LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 
entity BLM_Interlock_out is

generic
        (n : integer range 0 to 62 :=62       -- counter pool inputs:  hardware inputs plus test signals
        
);
port (
        CLK              : in std_logic;      -- Clock
        nRST             : in std_logic;      -- Reset
        out_mux_sel      : in std_logic_vector(12 downto 0);
        UP_OVERFLOW      : in std_logic_vector((n-8)*(n-2)-1 downto 0) ; 
        DOWN_OVERFLOW    : in std_logic_vector((n-8)*(n-2) -1  downto 0) ; 
        gate_error       : in std_logic_vector(1 downto 0);
        Interlock_IN     : in std_logic_vector(8 downto 0);
        INTL_Output      : out std_logic_vector(5 downto 0) 
);
end BLM_Interlock_out;

architecture rtl of BLM_Interlock_out is
-- for n=62 up_overflow has 3240 bits, down_overflow has 3240 bits  -> 4096 = 2^12
-- which are divided in 6 bits lenghts (output width) tot 540  -> 1024 = 2^10 

 --component BLM_OUT_MUX is
 --       generic
 --           ( SIZE : integer: 12; --the mux has 2^SIZE inputs 
 --              WIDTH: integer: 6;  --the mux has a 6-bit output
 --       port 
 --            (
 --             CLK             : in std_logic;      -- Clock
 --             in_mux          : in  std_logic_vector(2**SIZE)*(WIDTH -1 downto 0));
 --             sel             : in std_logic_vector(SIZE -1 downto 0);
 --             out_mux         : out std_logic_vector(WIDTH -1 downto 0)
 --            );

-- end component;

      type event_count_type is array (0 to 5) of integer range 0 to 540;
      signal Int_up_event_counter: event_count_type;
      signal Int_down_event_counter: event_count_type;

      signal interlock_event: std_logic_vector(5 downto 0);

      begin
     
        
        interlock_event_counter: process (nRST, CLK)
        begin
                if not nRST='1' then 
                        for i in 0 to 5 loop
                                

                    Int_up_event_counter(i) <= 0; 
                    Int_down_event_counter(i) <= 0;
                    end loop;
                   


                elsif (CLK'EVENT AND CLK = '1') then
                  for j in 1 to 6 loop
                     for i in 0  to (j*9*(n-2)-1) loop
                        if UP_OVERFLOW(i)='1' then 
                                Int_up_event_counter(j-1) <= Int_up_event_counter(j-1) +1; 
                        end if;
                        if DOWN_OVERFLOW(i)='1' then 
                                Int_down_event_counter(j-1) <= Int_down_event_counter(j-1) +1;
                        end if;
                
                      end loop;        
                  end loop;
                end if;
         end process;
         
         control_int_process:process (nRST, CLK)
          begin
                if not nRST='1' then 
                        interlock_event <=   (OTHERS =>  '0');
                elsif (CLK'EVENT AND CLK = '1') then
                 for i in 0 to 5 loop
                        if ( (Int_up_event_counter(i) = 0) and (Int_down_event_counter(i)= 0)) then 
                                interlock_event(i) <= '0';
                        else 
                
                                interlock_event(i) <= '1';
                        end if;
                end loop;
               end if;
         end process;  

output_process: process(CLK, out_mux_sel)

--when no error and no interlock outputs, all the leds light up

  begin
    if  (CLK'EVENT AND CLK = '1') then
      case (out_mux_sel) is  

        when "1000000000000" =>

                        INTL_Output(0) <= not(interlock_event(0) or Interlock_IN(0) or Interlock_IN(1));
                        INTL_Output(1) <= not(interlock_event(1) or Interlock_IN(2) or Interlock_IN(3));
                        INTL_Output(2) <= not(interlock_event(2) or Interlock_IN(4) or Interlock_IN(5));
                        INTL_Output(3) <= not(interlock_event(3) or Interlock_IN(6) or Interlock_IN(7));
                        INTL_Output(4) <= not(interlock_event(4) or Interlock_IN(8));
                        INTL_Output(5) <= not(interlock_event(5) or gate_error(0) or gate_error(1));  
        
        when "0100000000000" =>

                        INTL_Output(0) <= not( Interlock_IN(0));
                        INTL_Output(1) <= not(Interlock_IN(1));
                        INTL_Output(2) <= not(Interlock_IN(2));
                        INTL_Output(3) <= not(Interlock_IN(3));
                        INTL_Output(4) <= not(Interlock_IN(4));
                        INTL_Output(5) <= not(Interlock_IN(5));

        when "0010000000000" =>

                        INTL_Output(0) <= not(Interlock_IN(6));
                        INTL_Output(1) <= not(Interlock_IN(7));
                        INTL_Output(2) <= not(Interlock_IN(8));
                        INTL_Output(3) <= not(gate_error(0));
                        INTL_Output(4) <= not(gate_error(1));
                        INTL_Output(5) <= '1';

        when others =>  INTL_Output <= (others =>'0');
                 --    Null; -- for now

              --  BM_mult: BLM_OUT_MUX
                 --  generic map ( SIZE => 12,
                 --                WIDTH => 6)
                --   port map (    CLK => CLK,
                --                 in_mux => UP_OVERFLOW & DOWN_OVERFLOW,
                --                 sel => out_mux_sel(12 downto 0),
                --                out_mux => INTL_Output
                --            );
     end case;
    end if;
end  process;
                
end rtl;
