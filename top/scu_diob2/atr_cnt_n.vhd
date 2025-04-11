--TITLE "'atr_cnt_n' Autor: R.Hartmann";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY atr_cnt_n IS

	port(
    clk:                    IN  STD_LOGIC;
    nReset:                 IN  STD_LOGIC;
    clk_250mhz:             IN  STD_LOGIC;
    nReset_250mhz:          IN  STD_LOGIC;
    ATR_cnt_puls:           IN  STD_LOGIC;
    ATR_comp_cnt_err_res:   IN  STD_LOGIC;   -- Reset Counter
--
    ATR_counter:            OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    ATR_cnt_err:            OUT STD_LOGIC
		);
	end atr_cnt_n;



ARCHITECTURE Arch_atr_cnt_n OF atr_cnt_n IS

signal ATR_cnt_puls_syn1    : std_logic;
signal ATR_cnt_puls_syn     : std_logic;
-- signal ATR_cnt_puls_syn_t   : std_logic;

type state_t is   (idle, count, save, error, w_end);
signal state:      state_t := idle;

-- signal Count_error     : std_logic; -- Error bei Zählerüberlauf  kk Not used

signal counter:       integer range 0 to 131071;       -- Counter Delay
signal s_save_cnt:    STD_LOGIC;                       -- Save Counter
signal ATR_Count:     STD_LOGIC_VECTOR(1 DOWNTO 0);
signal s_ATR_cnt_err: STD_LOGIC;                       -- Error-Flag

begin


sync_input:
process (clk_250mhz, nReset_250mhz, ATR_cnt_puls)
begin
  if ( nReset_250mhz       = '0') then
    ATR_cnt_puls_syn1     <= '0';
    ATR_cnt_puls_syn      <= '0';
    --ATR_cnt_puls_syn_t    <= '0';  --kk not used
  elsif (rising_edge(clk_250mhz)) then
    ATR_cnt_puls_syn1     <= ATR_cnt_puls;
    ATR_cnt_puls_syn      <= ATR_cnt_puls_syn1;  -- Synchroner Comperator Puls
   -- ATR_cnt_puls_syn_t    <= ATR_cnt_puls_syn;   -- Synchroner Comperator Puls, um 1 Takt verzögert --kk not used
    end if;
end process;



P_Counter:  process (clk_250mhz, nReset_250mhz, ATR_cnt_puls_syn)

    begin
      if (nReset_250mhz    = '0') then
        state             <= idle;
        --Count_error       <= '0';   -- Error bei Zählerüberlauf kk not used
        Counter           <=  0 ;
        s_save_cnt        <= '0';   -- Save Counter
        s_ATR_cnt_err     <= '0';   -- Error-Flag

      ELSIF rising_edge(clk_250mhz) then
      case state is
         when idle      =>  Counter                  <=  0 ;  -- Counter
                            --Count_error              <= '0';   -- Counter_Error  kk not used
                            if     (ATR_cnt_puls_syn  = '1')  then
                                  state  <= count;
                            else  state  <= idle;
                            end if;

        when count      =>  Counter <= Counter + 1; --Counter + 1

                            if (Counter > 65535)  then -- größer 16 Bit (FFFF) ==> Error
                                  state  <= error;
                            else
                              if   (ATR_cnt_puls_syn  = '1')  then
                                    state  <= count;
                              else  state  <= save;
                              end if;
                            end if;


         when save      =>  s_save_cnt  <= '1';      -- set Strobe "Save Counter"
                            state       <= w_end;

         when error     =>  Counter        <= 65535; -- = x"FFFF"
                            s_save_cnt     <= '1';   -- set Strobe "Save Counter"
                            s_ATR_cnt_err  <= '1';   -- set Error-Flag
                                     state <= w_end;

         when w_end     =>  s_save_cnt    <= '0';   -- reset Strobe "Save Counter"
                            s_ATR_cnt_err <= '0';   -- reset Error-Flag

                            if (ATR_cnt_puls_syn  = '1')  then 	-- warte bis Puls wieder "0" ist
                                  state  <= w_end;
                            else  state  <= idle;
                            end if;

      end case;
    end if;
  end process P_Counter;




P_Save:	PROCESS (clk_250mhz, nReset_250mhz, ATR_comp_cnt_err_res)
	BEGIN
		IF  ((nReset_250mhz = '0') or (ATR_comp_cnt_err_res = '1')) THEN
        ATR_counter <= (OTHERS => '0');

		ELSIF rising_edge(clk_250mhz) THEN
			IF s_save_cnt    = '1' THEN -- Save Counter
        ATR_counter <= std_logic_vector(to_unsigned(Counter, 16));   -- Umwandlung Integer ==> Hex(16Bit)
			END IF;
		END IF;
	END PROCESS P_Save;

P_Error:	PROCESS (clk_250mhz, nReset_250mhz, ATR_comp_cnt_err_res)
	BEGIN
		IF  ((nReset_250mhz = '0') or (ATR_comp_cnt_err_res = '1')) THEN
        ATR_cnt_err <= '0';

		ELSIF rising_edge(clk_250mhz) THEN
			IF s_ATR_cnt_err    = '1'   THEN -- Save Error
        ATR_cnt_err      <= '1';       -- set Error-Flag
			END IF;
		END IF;
	END PROCESS P_Error;





end Arch_atr_cnt_n;
