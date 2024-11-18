library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;


entity local_thr_box is
    port(
        clk : in std_logic;     
        nRST : in std_logic;   
        trigger:in std_logic; -- trigger from register is  BLM_new_dataset_Reg(12),
        group_dataset: in std_logic_vector(11 downto 0);  -- from register  BLM_new_dataset_Reg(11 downto 0), 11..8 set nr
        thr_data: in std_logic_vector(63 downto 0);
        addr_ena: out std_logic;
        addr_b_ram: out std_logic_vector(11 downto 0);
        counter_group: in t_group_Array; --128 x 4 bits
        set_ready: out std_logic;
        state_nr : out std_logic_vector (2 downto 0); --for tests
        counter_nr_read: out std_logic_vector(7 downto 0); -- for tests
        loc_pos_thr:  out t_BLM_th_Array; --o to 127 
        loc_neg_thr:  out t_BLM_th_Array );
end local_thr_box;

architecture rtl of local_thr_box is

--signal busy: std_logic;
type set_buid_type is (state_idle, state_wait1, state_wait2, state_read, state_finish);
signal build_state: set_buid_type;
signal set_nr: integer range 0 to 31;
signal addr_nr : integer range 0 to 4095;
signal state_sm: integer range 0 to 7:= 0;
signal cnt_nr: integer range 0 to 128;
--signal set_cnt: integer range 0 to 31;
signal start_addr: std_logic_vector(11 downto 0);
signal group_nr: std_logic_vector(3 downto 0);
signal dataset: std_logic_vector(7 downto 0);
--signal cnt_nr_start: integer range 0 to 128;




begin 

    group_nr <= group_dataset(11 downto 8);
    dataset <= group_dataset(7 downto 0);
    start_addr <= dataset(4 downto 0) & "0000000";
    addr_b_ram <= std_logic_vector(to_unsigned(addr_nr, addr_b_ram'length));
    counter_nr_read<= std_logic_vector(to_unsigned(cnt_nr, counter_nr_read'length));
    state_nr <=  std_logic_vector(to_unsigned(state_sm, state_nr'length));

    state_sm_proc: process(clk, nRST)
        begin
            if ((nRST= '0')) then 
                state_sm <= 0;
            elsif rising_edge(clk) then
                case build_state is
                    when state_idle     => state_sm <= 0;
                    when state_wait1    => state_sm <= 1;
                    when state_wait2    => state_sm <= 2;
                    when state_read     => state_sm <= 3;
                    when state_finish   => state_sm <= 4;

                    when others         => state_sm <= 7;
                end case;
            end if;
  end process;


    thr_values_proc: process(clk,nRST)
        begin
    
        if not nRST='1' then 

            for i in 0 to 127 loop
                loc_pos_thr(i) <= (others => '0');
                loc_neg_thr(i) <= (others => '0');
            end loop;
           -- busy <= '0';
            build_state <= state_idle;
            set_ready <= '0';
           
            cnt_nr <= 0;
            --set_cnt <= 0;

        elsif (clk'EVENT AND clk= '1') then  


            case (build_state) is

                when state_idle =>
                    --busy <= '0';
                    set_ready <= '0';
                    --cnt_nr<=0;

                    if trigger ='1' then 
                        --set_ready <='1';
                        addr_nr <=   to_integer(unsigned(start_addr));          
   
                        cnt_nr <= 0;
                        build_state <= state_wait1;
                    end if;
                
                when state_wait1 =>
                    build_state <= state_wait2;        

                when state_wait2 =>
                    build_state <= state_read;        
    

                when state_read => 
                    if (group_nr(3 downto 0) = counter_group(cnt_nr) ) then
                        loc_neg_thr(cnt_nr) <= thr_data(63 downto 32);
                        loc_pos_thr(cnt_nr) <= thr_data(31 downto 0);  
                    end if;
                   -- loc_neg_thr(cnt_nr) <= x"12345678";
                   -- loc_pos_thr(cnt_nr) <=  x"12345678";
        
                    --  if cnt_nr = cnt_nr_start + 7 or cnt_nr =127 then 

                    if (cnt_nr = 127) then
                        set_ready <= '1';
                        build_state <= state_finish;
                    else
                        addr_nr <= addr_nr+1;
                        cnt_nr <= cnt_nr+1;
                        build_state <= state_wait1;
                    end if;

                when state_finish =>
                    --if trigger ='0' then 
                    --    build_state <= state_idle;
                    --end if;
                    set_ready <= '0';
                    build_state <= state_idle;

                WHEN others  => 
                    build_state <= state_idle;

            end case;
        end if;
    end process;
    --set_ready <= not busy;
    addr_ena  <= '1'; 
end architecture rtl;


