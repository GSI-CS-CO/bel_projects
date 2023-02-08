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
        gate_UP_OVERFLOW  : out t_gate_counter_in_Array;
        gate_DOWN_OVERFLOW: out t_gate_counter_in_Array;
        gate_error       : in std_logic_vector(11 downto 0);
        Interlock_IN     : in std_logic_vector(53 downto 0);
        INTL_Output      : out std_logic_vector(5 downto 0);
        BLM_status_Reg : out t_IO_Reg_0_to_7_Array
);
end BLM_Interlock_out;

architecture rtl of BLM_Interlock_out is

      signal in_overflow: std_logic_vector(1023 downto 0);
      signal overflow: std_logic_vector(5 downto 0);
      signal gate_overflow: std_logic_vector(143 downto 0);
      signal overflow_in: std_logic_vector (1535 downto 0);
      signal m: integer range 0 to 255 :=0;
      signal overflow_cnt: std_logic_vector(7 downto 0);
      constant ZERO_OVERFLOW:  std_logic_vector (in_overflow'range) := (others => '0');
      constant ZERO_gate_OVERFLOW:  std_logic_vector (gate_overflow'range) := (others => '0');
      signal no_overflow: std_logic;
      signal no_gate_overflow: std_logic;
      component overflow_ram IS
      PORT
      (
        aclr		: IN STD_LOGIC  := '0';
        clock		: IN STD_LOGIC  := '1';
        data		: IN STD_LOGIC_VECTOR (5 DOWNTO 0);
        rdaddress		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
        rden		: IN STD_LOGIC  := '1';
        wraddress		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
        wren		: IN STD_LOGIC  := '0';
        q		: OUT STD_LOGIC_VECTOR (5 DOWNTO 0)
      );

      end component;

      begin

        mux_in_process:process (nRST, CLK)
        begin
        if not nRST='1' then 
              in_overflow <=   (OTHERS =>  '0');
       
              gate_overflow <= (others =>'0');

              overflow_in <= (OTHERS =>'0');
              overflow_cnt <= (OTHERS =>'0');
              no_overflow <='0';
              no_gate_overflow <='0';
          
       elsif (CLK'EVENT AND CLK = '1') then  
       

        in_overflow <= UP_OVERFLOW(7)&UP_OVERFLOW(6)&UP_OVERFLOW(5)&UP_OVERFLOW(4)&UP_OVERFLOW(3)&UP_OVERFLOW(2)&UP_OVERFLOW(1)&UP_OVERFLOW(0)& 
        DOWN_OVERFLOW(7)& DOWN_OVERFLOW(6)& DOWN_OVERFLOW(5)& DOWN_OVERFLOW(4)& DOWN_OVERFLOW(3)& DOWN_OVERFLOW(2)& DOWN_OVERFLOW(1) & DOWN_OVERFLOW(0);

        if in_overflow = ZERO_OVERFLOW then
          no_overflow <='1';
        end if;
        

        gate_overflow <= gate_UP_OVERFLOW(5) & gate_UP_OVERFLOW(4) & gate_UP_OVERFLOW(3) & gate_UP_OVERFLOW(2) & gate_UP_OVERFLOW(1) & gate_UP_OVERFLOW(0) &
         gate_DOWN_OVERFLOW(5) & gate_DOWN_OVERFLOW(4) & gate_DOWN_OVERFLOW(3) & gate_DOWN_OVERFLOW(2) & gate_DOWN_OVERFLOW(1) & gate_DOWN_OVERFLOW(0); 

       if gate_overflow = ZERO_gate_OVERFLOW then
          no_gate_overflow <='1';
        end if;

        overflow_in(1239 downto 0) <=  gate_error & Interlock_IN & "00" & gate_overflow & "0000" &in_overflow;
        overflow_in(1535 downto 1240) <= (others =>'0');
       

        if out_mux_sel(8) ='1' then 
           m <=0;

        elsif out_mux_sel(9) ='1' then
          m <= m + 1;
          if m = 255 then      
            m <= 0;
          end if;

        end if;

        overflow_cnt <= std_logic_vector(to_unsigned(m,8));

      end if;

    end process;


        overflow_ram_el: overflow_ram 

        port map(
  
          aclr	=> out_mux_sel(8),
          clock	=> CLK,
          data		=> overflow_in((6*m +5) downto (6*m)),
          rdaddress	=> out_mux_sel(7 downto 0),
          rden		=> out_mux_sel(10),
          wraddress	=> overflow_cnt,
          wren	=> out_mux_sel(9),
          q		=> overflow
        );

        INTL_Output <= overflow;

     --------------------------------------------------------------------------------------------------
     -----                         BLM_STATUS_REGISTERS               
     --------------------------------------------------------------------------------------------------

        BLM_status_reg(0)<=  out_mux_sel(7 downto 0)& no_gate_overflow & no_overflow &  INTL_Output; -- out_mux_sel(7..0) readback, gate_overflow e input_overflow absence, BLM output
        BLM_status_reg(1)<= "00"& gate_error(11 downto 6) & "00" & gate_error(5 downto 0);         -- gate error
        BLM_status_reg(2)<= "00"& interlock_IN(11 downto 6) & "00" & Interlock_IN(5 downto 0);    -- interlock board 1 and board 2
        BLM_status_reg(3)<= "00"& interlock_IN(23 downto 18) & "00" & Interlock_IN(17 downto 12); -- interlock board 3 and board 4
        BLM_status_reg(4)<= "00" & interlock_IN(35 downto 30)&"00"& Interlock_IN(29 downto 24);   -- interlock board 5 and board 6
        BLM_status_reg(5)<= "00"& interlock_IN(47 downto 42)&"00"& Interlock_IN(41 downto 36);    -- interlock board 7 and board 8
        BLM_status_reg(6)<= "0000000000"& interlock_IN(53 downto 48);                             -- interlock board 9
      
      
end rtl;
