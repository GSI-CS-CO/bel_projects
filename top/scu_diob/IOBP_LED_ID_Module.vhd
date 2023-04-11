LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity IOBP_LED_ID_Module is

port (
        clk_sys           : in  std_logic;      
        rstn_sys          : in  std_logic;      
        Ena_Every_250ns   : in  std_logic; 
        AW_ID             : in  std_logic_vector(7 downto 0); -- Application_ID
        IOBP_LED_ID_Bus_i : in  std_logic_vector(7 downto 0);   -- LED_ID_Bus_In
        IOBP_Aktiv_LED_o  : in  t_led_array;    -- Active LEDs of the "Slave-Boards"
        IOBP_Sel_LED      : in  t_led_array;    -- Sel-LED of the "Slave-Boards"
        IOBP_LED_En       : out std_logic; -- Output-Enable for LED -ID-Bus
        IOBP_STR_rot_o    : out std_logic_vector(12 downto 1);  -- LED-Str Red  for Slave 12-1
        IOBP_STR_gruen_o  : out std_logic_vector(12 downto 1);  -- LED-Str Green for Slave 12-1
        IOBP_STR_ID_o     : out std_logic_vector(12 downto 1);  -- ID-Str Green for Slave 12-1
        IOBP_LED_ID_Bus_o : out std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
        IOBP_ID           : out t_id_array     -- IDs of the "Slave-Boards"
        
        );
        end IOBP_LED_ID_Module;
        
architecture rtl of IOBP_LED_ID_Module is

signal Slave_Loop_cnt:      integer range 0 to 12;         -- 1-12   -- Loop-Counter

type   IOBP_LED_state_t is   (IOBP_idle, led_id_wait, led_id_loop, led_str_rot_h, led_str_rot_l, led_gruen,
                              led_str_gruen_h, led_str_gruen_l, iobp_led_dis, iobp_led_z, iobp_id_str_h, iobp_rd_id, iobp_id_str_l, iobp_end);
signal IOBP_state:   IOBP_LED_state_t:= IOBP_idle;
        
begin

P_IOBP_LED_ID_Loop:  process (clk_sys, Ena_Every_250ns, rstn_sys, IOBP_state)

    begin
      if (not rstn_sys = '1') then
        Slave_Loop_cnt       <=   1;                 --  Loop-Counter
        IOBP_LED_En          <=  '0';                --  Output-Enable for LED- ID-Bus
        IOBP_STR_rot_o       <=  (others => '0');    --  Led-Strobs 'red'
        IOBP_STR_gruen_o     <=  (others => '0');    --  Led-Strobs 'green'
        IOBP_STR_id_o        <=  (others => '0');    --  ID-Strobs


    ELSIF (clk_sys'EVENT AND clk_sys = '1' AND Ena_Every_250ns = '1') THEN
--  ELSIF ((rising_edge(clk_sys)) or Ena_Every_100ns)  then
      case IOBP_state is
        when IOBP_idle   =>  Slave_Loop_cnt       <=  1;                 -- Loop-Counter

                            if  (AW_ID(7 downto 0) = "00010011") THEN  IOBP_state  <= led_id_wait; -- AW_ID(7 downto 0) = c_AW_INLB12S1.ID
                                                                       else  IOBP_state  <= IOBP_idle;
                            end if;

        when led_id_wait      =>  IOBP_LED_En          <=  '1';                --  Output-Enable for LED- ID-Bus
                                  IOBP_state  <= led_id_loop;

        when led_id_loop      =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("0" & "0");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  IOBP_Aktiv_LED_o(Slave_Loop_cnt)(6 downto 1);   -- Active-LED for Slave to LED-Port
                                  IOBP_state  <= led_str_rot_h;

        when led_str_rot_h    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '1';   -- Active LED for Slave (Slave_Loop_cnt) to LED-Port
                                  IOBP_state  <= led_str_rot_l;

        when led_str_rot_l    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '0';   -- Active LED for Slave (Slave_Loop_cnt) to LED-Port
                                  IOBP_state  <= led_gruen;

        when led_gruen        =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("0" & "0");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  not IOBP_Sel_LED(Slave_Loop_cnt)(6 downto 1);   -- Sel-LED for Slave to LED-Port
                                  IOBP_state  <= led_str_gruen_h;

        when led_str_gruen_h  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '1';   -- Sel-LED for Slave (Slave_Loop_cnt) to LED-Port
                                  IOBP_state  <= led_str_gruen_l;

        when led_str_gruen_l  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '0';   -- Sel-LED for Slave (Slave_Loop_cnt)to LED-Port
                                  IOBP_state  <= iobp_led_dis;

        when iobp_led_dis     =>  IOBP_LED_En <=  '0';                        --  Disable Output for LED- ID-Bus
                                  IOBP_state  <= iobp_led_z;

        when iobp_led_z       =>  IOBP_state  <= iobp_id_str_l;

        when iobp_id_str_l    =>  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '1';   -- Sel-ID for Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_rd_id;

        when iobp_rd_id       =>  IOBP_ID(Slave_Loop_cnt) <=  IOBP_LED_ID_Bus_i;   -- Sel-ID for Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_id_str_h;

        when iobp_id_str_h    =>  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '0';   -- Sel-ID for Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_end;

        when iobp_end         =>  Slave_Loop_cnt <=  Slave_Loop_cnt + 1;       -- Loop +1

                                  if Slave_Loop_cnt < 13 then
                                    IOBP_state     <= led_id_wait;
                                  else
                                    IOBP_state     <= IOBP_idle;
                                  end if;

        when others           =>  IOBP_state       <= IOBP_idle;

      end case;
    end if;
  end process P_IOBP_LED_ID_Loop;
  
  end rtl; 
