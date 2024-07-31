library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;

entity event_ctrl_el is

port(
    clk : in std_logic;     -- chip-internal pulsed clk signal
    nRST : in std_logic;   -- general reset signal
    nEvent_Str : in std_logic; -- low active SCU_Bus runs timing cycle
    A_Address: in std_logic_vector(15 downto 0); -- SCU-Adress Bus
    A_Data: in std_logic_vector(15 downto 0); -- SCU-Data Bus (inout)
    BLM_event_key_Reg : in std_logic_vector(15 downto 0); --mask in_data
    BLM_event_ctrl_Reg : in std_logic_vector(15 downto 0); --config signals reg

    prepare_out: out std_logic_vector(11 downto 0);
    recover_out: out std_logic_vector(11 downto 0);
    reset_ctr_out: out std_logic;
    load_thr_out: out std_logic;
    reg_curr_data_set_by_ev: out std_logic_vector(11 downto 0);
    BLM_event_readout_Reg : out t_IO_Reg_0_to_2_Array  -- virt acc readed address + value
   
);
end event_ctrl_el;

architecture rtl of event_ctrl_el is
signal sync_str : std_logic;




type event_state_type is (idle, check_state, select_state, wait_state);
signal event_state: event_state_type;


signal prepare_sig: std_logic_vector(11 downto 0);
signal recover_sig: std_logic_vector(11 downto 0);
signal reset_ctr_sig: std_logic;
signal load_thr_sig: std_logic;
signal reg_curr_data_set_sig : std_logic_vector(11 downto 0);
signal data_value : std_logic_vector(15 downto 0);
signal data_value_del : std_logic_vector(15 downto 0);
signal latched_value : std_logic_vector(31 downto 0);
signal data_address : std_logic_vector(15 downto 0);
signal data_address_del : std_logic_vector(15 downto 0);
signal compare: std_logic;


signal sync_str_del: std_logic;
signal compare_vector: std_logic_vector(15 downto 0);
signal tag_code: std_logic_vector(15 downto 0);

signal event_enable: std_logic;



begin


event_sync_proc: process (clk, nRST)
    begin
        if not nRST='1' then 
           sync_str <='0';
        elsif (clk'EVENT AND clk= '1') then  
            sync_str_del <= not (nEvent_Str);
            sync_str <= sync_str_del;
        end if;
end process;

events_processing: process(clk, nRST) 
begin
    if not nRST='1' then 

        latched_value <= (others =>'0');
        tag_code <= (others =>'0');
        event_state <= idle;
        prepare_sig <=(others =>'0');
        recover_sig <= (others =>'0');
        reset_ctr_sig <= '0';
        load_thr_sig <= '0';
        reg_curr_data_set_sig <= (others => '0');

        event_enable <= '0';

    elsif (clk'EVENT AND clk= '1') then 

        data_address_del <= A_Address;
        data_value_del <= A_Data;
        data_address <= data_address_del;
        data_value <= data_value_del;
        event_enable <= BLM_event_ctrl_Reg(0);
        case (event_state) is
            when idle => 
                --for i in 31 to 16 loop
                --    compare_vector(i) <= latched_value(i) XNOR BLM_event_key_Reg(i);
                --end loop; 
                --    compare <= and_reduce (compare_vector);
                if BLM_event_key_Reg = data_address then
                    compare <= '1';
                else
                    compare <= '0';
                end if;
               -- if sync_str ='1' then 
		
                if (sync_str = '1' and event_enable ='1') then    
 		    latched_value  <= data_address & data_value;
                    event_state <= check_state;
                end if;
                
            when check_state => 
                if compare = '1' then 
                    event_state <= select_state;
                    tag_code <= latched_value(15 downto 0);
                else
                    event_state <= wait_state; 
                end if;


            when select_state => 


                case tag_code (15 downto 12) is
                    when "0001" => load_thr_sig <= '1';
                                   reg_curr_data_set_sig <= tag_code(11 downto 0);

                    when "0010" => prepare_sig <= tag_code(11 downto 0);

                    when "0011" => recover_sig <= tag_code(11 downto 0);

                    when "0100" => reset_ctr_sig <= '1';

    
                    when others => NULL;
                                
                end case;
                    event_state <= wait_state;

            when wait_state =>
                prepare_sig <=(others =>'0');
		recover_sig <= (others =>'0');
		reset_ctr_sig <= '0';
		load_thr_sig <= '0';
            --    if sync_str ='0' then 
                  if sync_str ='0' then 
                    event_state <= idle;
                    
                end if;


            when others => null;
        end case;
    end if;
end process;

BLM_event_readout_Reg(0) <= latched_value(15 downto 0); 
BLM_event_readout_Reg(1) <= latched_value(31 downto 16); 
BLM_event_readout_Reg (2) <= tag_code;
prepare_out <= prepare_sig;
recover_out<= recover_sig;
reset_ctr_out <= reset_ctr_sig;
load_thr_out<= load_thr_sig;
reg_curr_data_set_by_ev <= reg_curr_data_set_sig;





end architecture rtl;