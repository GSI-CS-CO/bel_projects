--Generates Signals for Spill Abort
--Author: Kai Lüghausen <k.lueghausen@gsi.de>

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity spill_abort is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;
           time_pulse : in STD_LOGIC;
           armed : in STD_LOGIC;
           req : in STD_LOGIC;
           abort : out STD_LOGIC;
           abort_rst : out STD_LOGIC);
end spill_abort;

architecture Arch_spill_abort of spill_abort is

type SM_STATES is (P_WAIT,D1,D2);
signal STATE      : SM_STATES := P_Wait;
signal NEXT_STATE : SM_STATES := P_Wait;



signal assert_count  : std_logic_vector (11 downto 0):=X"000";
signal assert_ctr_tc : std_logic;
signal assert_ctr_en : std_logic;
signal assert_ctr_load : std_logic;


begin


--Clocking
process (clk, nReset)
    begin
        if rising_edge (clk)
        then
            if nReset = '0' then
            STATE <= P_Wait;
            else
            STATE  <= NEXT_STATE;
            end if;
        end if;

end process;
---------------------------------------------------------------------------------

process (STATE, armed,req)
    begin

    NEXT_STATE <= STATE;
        case STATE is

            when P_WAIT =>          if ( armed = '1' and req = '0' )
                                    then NEXT_STATE <= D1;
                                    else
                                    NEXT_STATE <= P_Wait;
                                    end if;
            when D1 =>              if armed = '0'
                                    then NEXT_STATE <= D2;
                                    else
                                    NEXT_STATE <= D1;
                                    end if;
            when D2 =>              NEXT_STATE <= P_WAIT;
        end case;
end process;

process (clk)
begin
        if rising_edge (clk) then
          if ( STATE /= P_WAIT )
          then
            abort<= '0';
          else
            abort <= '1';
          end if;
        end if;
end process;

--command <= armed and not req;

---Assert-Count-------------------------------------------
process (clk, nReset)
begin
        if nReset = '0' then
          assert_count <= X"000";
        elsif rising_edge (clk) then
            if assert_ctr_load= '1' then
                assert_count <= X"032";
            elsif (assert_ctr_tc = '0') then
                if time_pulse = '1' then
                assert_count <= assert_count - 1;
                end if;
            end if;
        end if;
end process;
assert_ctr_tc <= '1' when (assert_count = 0) else '0';
assert_ctr_load <=  '1' when (State = D2)  else '0';

abort_rst <= not assert_ctr_tc;

end Arch_spill_abort;
