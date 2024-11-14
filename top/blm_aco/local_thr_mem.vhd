library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;

entity local_thr_mem is
    port (
        clk : in std_logic;
        nRST : in std_logic;
        A_A : in std_logic_vector(15 downto 0);
        A_nDS : in std_logic;
        A_nBoardSel : in std_logic;
        A_RnW : in std_logic;
        load_thr : in std_logic;
        loaded_data_set : in std_logic_vector(11 downto 0);
        new_dataset_ready : in std_logic;
        counter_group_Reg : in t_IO_Reg_0_to_31_Array;
        --
        reg_trigger : in std_logic;
        reg_group_dataset : in std_logic_vector(11 downto 0);

        timing_trigger : in std_logic;
        timing_group_dataset : in std_logic_vector(11 downto 0);
        --

        A_D : inout std_logic_vector(15 downto 0);
        A_Dtack : out std_logic;
        -- thr_data: out std_logic_vector(63 downto 0);
        loc_pos_thr : out t_BLM_th_Array;
        loc_neg_thr : out t_BLM_th_Array;

        nSel_Ext_Data_Drv_out : out std_logic;

        box_state_nr : out std_logic_vector(2 downto 0);
        counter_nr_read : out std_logic_vector(7 downto 0); -- for tests
        reg_state_nr : out std_logic_vector(1 downto 0)
    );
end local_thr_mem;

architecture rtl of local_thr_mem is

    component th_ram is
        port (
            address_a : in std_logic_vector (13 downto 0);
            address_b : in std_logic_vector (11 downto 0);
            clock : in std_logic := '1';
            data_a : in std_logic_vector (15 downto 0);
            data_b : in std_logic_vector (63 downto 0);
            rden_a : in std_logic := '1';
            rden_b : in std_logic := '1';
            wren_a : in std_logic := '0';
            wren_b : in std_logic := '0';
            q_a : out std_logic_vector (15 downto 0);
            q_b : out std_logic_vector (63 downto 0)
        );
    end component th_RAM;

    component l0ad_thr_fifo is
        port (
            clock : in std_logic;
            data : in std_logic_vector (11 downto 0);
            rdreq : in std_logic;
            sclr : in std_logic;
            wrreq : in std_logic;
            empty : out std_logic;
            q : out std_logic_vector (11 downto 0)
        );
    end component l0ad_thr_fifo;

    component local_thr_box is
        port (
            clk : in std_logic;
            nRST : in std_logic;
            trigger : in std_logic;
            group_dataset : in std_logic_vector(11 downto 0);
            thr_data : in std_logic_vector(63 downto 0);
            addr_ena : out std_logic;
            addr_b_ram : out std_logic_vector(11 downto 0);
            counter_group : in t_group_Array; --128 x 4 bits
            state_nr : out std_logic_vector(2 downto 0);
            counter_nr_read : out std_logic_vector(7 downto 0); -- for tests
            set_ready : out std_logic;
            loc_pos_thr : out t_BLM_th_Array; --o to 127 
            loc_neg_thr : out t_BLM_th_Array
        );
    end component local_thr_box;

    component threshold_trigger_input is
        port (
            clk : in std_logic;
            nRST : in std_logic;
    
            in_trigger : in std_logic;
            in_group_dataset : in std_logic_vector(11 downto 0);
    
            out_trigger : out std_logic;
            out_group_dataset : out std_logic_vector(11 downto 0);
    
            ready : in std_logic
    
        );
        end component threshold_trigger_input;


    signal sync_A_nBoardSel : std_logic;

    signal sync_A_nDS : std_logic := '1';

    signal data_a : std_logic_vector (15 downto 0);
    signal thr_data : std_logic_vector (63 downto 0);
    signal rd_en : std_logic := '1';
    signal wr_en : std_logic := '0';
    signal q_data_a : std_logic_vector (15 downto 0);
    signal del_A_nDS : std_logic;
    signal del_A_nBoardSel : std_logic;
    type ram_state_type is (idle, check_state, write_state, write_2, write_3, write_end, read_state, read_1, read_2, read_3, read_end);
    signal ram_state : ram_state_type := idle;

    signal n_dtack : std_logic := '0';

    signal counter_nr : integer range 0 to 127;
    signal set_nr : integer range 0 to 31;

    signal address_b : std_logic_vector(11 downto 0);
    signal addr_nr : integer range 0 to 4095;
    --type thr_array is array (0 to 4095) of  std_logic_vector(31 downto 0);
    --signal neg_thr, pos_thr : thr_array;
    signal rd_en_b : std_logic;
    type load_state_type is (load_idle, load_addr, load_new_value1, load_new_value2, load_new_value3, load_end);
    signal load_state : load_state_type := load_idle;
    signal counter_group : t_group_Array; --128 x 4 bits
    signal cnt_vector : std_logic_vector(511 downto 0);
    signal blm_trigger : std_logic;

    type new_dataset_state_m is (reg_idle, timing,finish_state); --wait_state,
    signal new_dataset_state : new_dataset_state_m;
    signal fifo_in_data : std_logic_vector(11 downto 0);
    signal fifo_wr : std_logic;
    signal set_ready : std_logic;
    signal fifo_rd : std_logic;
    signal empty_fifo : std_logic;
    signal fifo_out_data : std_logic_vector(11 downto 0);
    signal state_sm : integer range 0 to 3 := 0;

    signal timing_trigger_out: std_logic;
    signal timing_dataset: std_logic_vector(11 downto 0);
    signal timing_set_ready: std_logic;
    signal reg_trigger_out: std_logic;
    signal reg_dataset: std_logic_vector(11 downto 0);
    signal reg_set_ready: std_logic;


begin

    ram_state_proc : process (clk, nRST)
    begin
        if not nRST = '1' then
            data_a <= (others => '0');
            sync_A_nBoardSel <= '0';

            ram_state <= idle;
            rd_en <= '0';
            wr_en <= '0';
            n_dtack <= '1';

            nSel_Ext_Data_Drv_out <= '1';

        elsif (clk'EVENT and clk = '1') then

            del_A_nBoardSel <= A_nBoardSel;
            sync_A_nBoardSel <= del_A_nBoardSel;
            del_A_nDS <= A_nDS;
            sync_A_nDS <= del_A_nDS;

            case (ram_state) is

                when idle =>
                    rd_en <= '0';
                    wr_en <= '0';
                    n_dtack <= '1';
                    nSel_Ext_Data_Drv_out <= '1';
                    if (sync_A_nDS = '0') and (sync_A_nBoardSel = '0') then
                        ram_state <= check_state;
                    end if;

                when check_state =>

                    if A_RnW = '0' then
                        A_D <= (others => 'Z');
                        data_a <= A_D;
                        ram_state <= write_state;
                    elsif A_RnW = '1' then
                        rd_en <= '1';
                        nSel_Ext_Data_Drv_out <= '0';
                        ram_state <= read_state;
                    end if;

                when write_state =>
                    wr_en <= '1';
                    ram_state <= write_2;

                when write_2 =>
                    wr_en <= '0';
                    ram_state <= write_3;

                when write_3 =>
                    n_dtack <= '0';

                    if sync_A_nDS = '1' then
                        ram_state <= write_end;
                    end if;

                when write_end =>
                    n_dtack <= '1';
                    ram_state <= idle;

                when read_state =>
                    ram_state <= read_1;

                when read_1 =>
                    ram_state <= read_2;

                when read_2 =>
                    A_D <= q_data_a;
                    ram_state <= read_3;

                when read_3 =>
                    n_dtack <= '0';
                    if sync_A_nDS = '1' then
                        nSel_Ext_Data_Drv_out <= '1';
                        A_D <= (others => 'Z');
                        rd_en <= '0';
                        ram_state <= read_end;
                    end if;

                when read_end =>
                    n_dtack <= '1';
                    ram_state <= idle;

                when others => null;

            end case;

        end if;

    end process;

    local_thresholds_ram : th_ram
    port map
    (

        address_a => A_A(13 downto 0),
        address_b => address_b,
        clock => clk,
        data_a => data_a,
        data_b => (others => 'Z'),
        rden_a => rd_en,
        rden_b => rd_en_b, --'0',
        wren_a => wr_en,
        wren_b => '0',
        q_a => q_data_a,
        q_b => thr_data
    );

    A_Dtack <= not(n_dtack);

    counter_group_proc : process (counter_group_Reg)
    begin
        for i in 0 to 31 loop
            cnt_vector(15 + 16 * i downto 16 * i) <= counter_group_reg(i);
        end loop;
        for j in 0 to 127 loop
            counter_group(j) <= cnt_vector(3 + 4 * j downto 4 * j);
        end loop;
    end process;

    dataset_setup_proc : process (clk, nRST)
    begin
        if not nRST = '1' then
            --   blm_trigger <= '0';

            new_dataset_state <= reg_idle;
            fifo_rd <= '0';
            fifo_wr <= '0';
        elsif (clk'EVENT and clk = '1') then

            case (new_dataset_state) is
                when reg_idle =>

                    if timing_trigger_out = '1' then
                        fifo_in_data <= timing_dataset;
                        fifo_wr <= '1';
                        timing_set_ready <= '1';
                        new_dataset_state <= finish_state;
                    elsif reg_trigger_out = '1' then
                        fifo_in_data <= reg_dataset;
                        fifo_wr <= '1';
                        reg_set_ready <= '1';
                        new_dataset_state <= finish_state;    
                    end if;

                when finish_state =>
                    fifo_wr <= '0';
                    timing_set_ready <= '0';
                    reg_set_ready <= '0';
                    new_dataset_state <= reg_idle;

                when others => null;

            end case;

        end if;

    end process;

    state_sm_proc : process (clk, nRST)
    begin
        if ((nRST = '0')) then
            state_sm <= 0;
        elsif rising_edge(clk) then
            case new_dataset_state is
                when reg_idle => state_sm <= 0;
                when timing => state_sm <= 1;
                when finish_state => state_sm <= 2;

                when others => state_sm <= 3;
            end case;
        end if;
    end process;

    reg_state_nr <= std_logic_vector(to_unsigned(state_sm, reg_state_nr'length));

    fifo_element : l0ad_thr_fifo
    port map
    (
        clock => clk,
        data => fifo_in_data,
        rdreq => set_ready,
        sclr => not nRST,
        wrreq => fifo_wr,
        empty => empty_fifo,
        q => fifo_out_data
    );

    local_threshold_collection_box : local_thr_box
    port map(
        clk => clk,
        nRST => nRST,
        trigger => not empty_fifo,
        group_dataset => fifo_out_data,
        counter_group => counter_group,
        thr_data => thr_data,
        addr_ena => rd_en_b,
        addr_b_ram => address_b,
        set_ready => set_ready,
        state_nr => box_state_nr,
        counter_nr_read => counter_nr_read,
        loc_pos_thr => loc_pos_thr,
        loc_neg_thr => loc_neg_thr
    );

   thres_timing_trig_input_sm: threshold_trigger_input 
        port map(
            clk => clk,
            nRST => nRST,
            in_trigger => timing_trigger,
            in_group_dataset => timing_group_dataset,
            out_trigger => timing_trigger_out,
            out_group_dataset => timing_dataset,
            ready => timing_set_ready
        );
 
        thres_reg_trig_input_sm: threshold_trigger_input 
        port map(
            clk => clk,
            nRST => nRST,
            in_trigger => reg_trigger,
            in_group_dataset => reg_group_dataset,
            out_trigger => reg_trigger_out,
            out_group_dataset => reg_dataset,
            ready => reg_set_ready
        );

end architecture rtl;