-------------------------------------------------------------------------------
-- Title      : Scalable Control Unit Bus Interface
-- Project    : SCU
-------------------------------------------------------------------------------
-- File       : scu_bus_master.vhd
-- Author     : Wolfgang Panschow
-- Company    : GSI
-- Created    : 2009-08-17
-- Last update: 2012-07-19
-- Platform   : FPGA-generics
-- Standard   : VHDL '93
-------------------------------------------------------------------------------
-- Description:
--
-- Master Bus Interface for the SCU Bus
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2009-08-17  1.0      W.Panschow      Created
-- 2009-08-17  1.1      W.Panschow
-- 2009-08-17  2.0      W.Panschow
-- 2012-07-19  2.1      S.Rauch         switched to numeric_std
-- 2012-07-19  2.2      W.Panschow    a) address decoding of internal registers now 16 bit deep
--                                    b) multicast slave write implemented
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.genram_pkg.all;


entity wb_scu_bus is

  generic(
      g_interface_mode        : t_wishbone_interface_mode      := CLASSIC;
      g_address_granularity   : t_wishbone_address_granularity := WORD;
      CLK_in_Hz               : integer := 100000000;
      Time_Out_in_ns          : integer := 350;
      dly_multicast_dt_in_ns  : integer := 200;
      Sel_dly_in_ns           : integer := 30;              -- delay to the I/O pins is not included
      Sel_release_in_ns       : integer := 30;              -- delay to the I/O pins is not included
      D_Valid_to_DS_in_ns     : integer := 30;              -- delay to the I/O pins is not included
      Timing_str_in_ns        : integer := 80;              -- delay to the I/O pins is not included
      Test                    : integer range 0 to 1 := 0
      );

port(

  -- Wishbone
  slave_i                            : in  t_wishbone_slave_in;
  slave_o                            : out t_wishbone_slave_out;

  srq_active                         : out std_logic_vector(12 downto 0);    -- vector of slave service requests

  clk                                : in std_logic;
  nrst                               : in std_logic;

  Timing_In                          : in std_logic_vector(31 downto 0)  := (others => '0');
  Start_Timing_Cycle                 : in std_logic                      := '0';

  SCUB_Data_Out                      : out std_logic_vector(15 downto 0);
  SCUB_Data_In                       : in std_logic_vector(15 downto 0);
  SCUB_Data_Tri_Out                  : out std_logic;
  nSCUB_DS                           : out std_logic;                      -- SCU_Bus Data Strobe, low active.
  nSCUB_Dtack                        : in  std_logic;                      -- SCU_Bus Data Acknowledge, low active.
  SCUB_Addr                          : out std_logic_vector(15 downto 0);  -- Address Bus of SCU_Bus
  SCUB_RDnWR                         : out std_logic;                      -- Read/Write Signal of SCU_Bus. Read is active high.
  -- Direction seen from this marco.
  nSCUB_SRQ_Slaves                   : in std_logic_vector(12 downto 0);   -- Input of service requests up to 12 SCU_Bus slaves, active low.
  nSCUB_Slave_Sel                    : out std_logic_vector(12 downto 0);  -- Output select one or more of 12 SCU_Bus slaves, active low.
  nSCUB_Timing_Cycle                 : out std_logic;                      -- Strobe to signal a timing cycle on SCU_Bus, active low.
  nSel_Ext_Data_Drv                  : out std_logic                       -- select for external data transceiver to the SCU_Bus, active low.
  );

end wb_scu_bus;

architecture Arch_SCU_Bus_Master of wb_scu_bus is


  signal Wr_Data            : std_logic_vector(15 downto 0);  -- IN wite data to SCU_Bus, or internal FPGA register
  signal Rd_Data            : std_logic_vector(15 downto 0);  -- OUT read data from SCU_Bus, or internal FPGA register
  signal Adr                : std_logic_vector(15 downto 0);  -- IN
  signal Slave_Nr           : std_logic_vector(3 downto 0);   -- IN 0x0 => internal access, 0x1 to 0xC => slave 1 to 12 access
  signal Start_Cycle        : std_logic;                      -- IN start data access from/to SCU_Bus
  signal Wr                 : std_logic;                      -- IN direction of SCU_Bus data access, write is active high.
  signal Rd                 : std_logic;                      -- IN

  signal SCU_Bus_Access_Active : std_logic;                   -- OUT active high signal: read or write access to SCUB is not finished
                                                              -- the access can be terminated bei an error, so look also to the
                                                              -- access error bis in the status register
  signal Intr               : std_logic;                      -- OUT One or more slave interrupts, or internal Interrupts (like
                                                              -- SCU_Bus-busy or SCU_Bus-timeout) are active. Intr is ative high.
  signal SCUB_Rd_Err_no_Dtack : std_logic;
  signal SCUB_Rd_Fin          : std_logic;
  signal SCUB_Rd_active       : std_logic;
  signal SCUB_Wr_Err_no_Dtack : std_logic;
  signal SCUB_Wr_Fin          : std_logic;
  signal SCUB_Wr_active       : std_logic;
  signal SCUB_Ti_Cyc_Err      : std_logic;
  signal SCUB_Ti_Fin          : std_logic;
  signal SCU_Wait_Request     : std_logic;                    -- active high signal: SCU_Bus is busy should be connect to avalon-bus wait.



  function set_vers_or_revi( vers_or_revi, Test: integer) return integer is
    begin
      if test > 0 then
        return 0;
      else
        return vers_or_revi;
      end if;
    end set_vers_or_revi;

  constant  C_SCUB_Version  : integer range 0 to 255 := set_vers_or_revi(2, Test);    -- define the version of this macro
  constant  C_SCUB_Revision : integer range 0 to 255 := set_vers_or_revi(2, Test);    -- define the revision of this macro

  constant  Clk_in_ps     : integer := 1000000000 / (Clk_in_Hz / 1000);
  constant  Clk_in_ns     : integer := 1000000000 / Clk_in_Hz;


  function set_ge_1  (a : integer) return integer IS
    begin
      if a > 1 then
        return a;
      else
        return 1;
      end if;
    end set_ge_1;

  function How_many_Bits (int: integer) return integer IS

    variable i, tmp : integer;

    begin
      tmp   := int;
      i   := 0;
      while tmp > 0 loop
        tmp := tmp / 2;
        i := i + 1;
      end loop;
      return i;
    end How_many_bits;


  constant  c_multicast_slave_acc : std_logic_vector(slave_Nr'range)  := X"D";

  constant  C_Sel_dly_cnt     : integer := set_ge_1(Sel_dly_in_ns * 1000 / Clk_in_ps)-2;    --  -2 because counter needs two more clock for unerflow
  signal    S_Sel_dly_cnt     : unsigned(How_many_Bits(C_Sel_dly_cnt) downto 0);

  constant  C_Sel_release_cnt   : integer := set_ge_1(Sel_release_in_ns * 1000 / Clk_in_ps)-2;  --  -2 because counter needs two more clock for unerflow
  signal    S_Sel_release_cnt   : unsigned(How_many_Bits(C_Sel_release_cnt) downto 0);

  constant  C_Timing_str_cnt    : integer := set_ge_1(Timing_str_in_ns * 1000 / Clk_in_ps)-2;   --  -2 because counter needs two more clock for unerflow
  signal    S_Timing_str_cnt    : unsigned(How_many_Bits(C_Timing_str_cnt) downto 0);

  constant  C_D_Valid_to_DS_cnt   : integer := set_ge_1(D_Valid_to_DS_in_ns * 1000 / Clk_in_ps)-2;  --  -2 because counter needs two more clock for unerflow
  signal    S_D_Valid_to_DS_cnt   : unsigned(How_many_Bits(C_D_Valid_to_DS_cnt) downto 0);

  constant  C_time_out_cnt      : integer := set_ge_1(time_out_in_ns * 1000 / Clk_in_ps)-2;   --  -2 because counter needs two more clock for unerflow
  signal    s_time_out_cnt      : unsigned(How_many_Bits(C_time_out_cnt) downto 0);

  constant  c_dly_multicast_dt_cnt  : integer := set_ge_1(dly_multicast_dt_in_ns * 1000 / Clk_in_ps)-2;   --  -2 because counter needs two more clock for unerflow
  signal    s_dly_multicast_dt_cnt  : unsigned(How_many_Bits(c_dly_multicast_dt_cnt) downto 0);

  constant  c_adr_width                     : integer := 16;          -- define how many address bits are used to decode the internal FPGA-register
  constant  C_Status_Adr                    : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0000#, c_adr_width);	-- real address is multiplied by two
  constant  C_Global_Intr_Ena_Adr           : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0002#, c_adr_width);	-- real address is multiplied by two
  constant  C_Vers_Revi_Adr                 : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0004#, c_adr_width);	-- real address is multiplied by two
  constant  C_SRQ_Ena_Adr                   : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0006#, c_adr_width);	-- real address is multiplied by two
  constant  C_SRQ_Active_Adr                : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0008#, c_adr_width);	-- real address is multiplied by two
  constant  C_SRQ_In_Adr                    : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#000A#, c_adr_width);	-- real address is multiplied by two
  constant  C_Wr_Multi_Slave_Sel_Adr        : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#000C#, c_adr_width);	-- real address is multiplied by two
  constant  C_Bus_master_intern_Echo_1_Adr  : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#000E#, c_adr_width);	-- real address is multiplied by two
  constant  C_Sw_Tag_Low_Adr                : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0010#, c_adr_width);	-- real address is multiplied by two
  constant  C_Sw_Tag_High_Adr               : unsigned(c_adr_width-1 downto 0) := to_unsigned(16#0012#, c_adr_width);	-- real address is multiplied by two



  signal    s_reset         : std_logic;
  signal    S_First_Sync_Reset    : std_logic;

  signal    S_SCUB_Addr       : std_logic_vector(15 downto 0);
  signal    S_SCUB_RDnWR      : std_logic;
  signal    S_SCUB_DS       : std_logic;

  signal    S_Slave_Nr        : std_logic_vector(3 downto 0);
  signal    S_SCUB_Slave_Sel    : std_logic_vector(nSCUB_Slave_Sel'range);
  signal    S_Slave_Sel       : std_logic_vector(nSCUB_Slave_Sel'range);
  signal    S_Multi_Slave_Sel   : std_logic_vector(nSCUB_Slave_Sel'range);
  signal    S_Multi_Wr_Flag     : std_logic;

  signal    S_Start_Cycle     : std_logic;

  signal    S_Sel_Ext_Data_Drv    : std_logic;

  signal    ext_rd_data       : std_logic_vector(15 downto 0);
  signal    int_rd_data       : std_logic_vector(15 downto 0);

  signal    S_Start_SCUB_Rd     : std_logic;

  signal    S_Start_SCUB_Wr       : std_logic;
  signal    S_Wr_Data             : std_logic_vector(15 downto 0);  -- store write pattern

  signal    S_Ti_Cy               : std_logic_vector(1 downto 0);   -- shift reg to generate pulse
  signal    S_Start_Ti_Cy         : std_logic;

  signal    S_nSync_Dtack         : std_logic_vector(1 downto 0);
  signal    S_Last_Cycle_Timing   : std_logic;
  signal    S_SCUB_Timing_Cycle   : std_logic;

  signal    S_SCUB_Rd_Err_no_Dtack  : std_logic;
  signal    S_SCUB_Wr_Err_no_Dtack  : std_logic;

  signal    S_Ti_Cyc_Err      : std_logic;
  signal    S_Timing_In       : std_logic_vector(31 downto 0);  -- store input timing_in
  signal    S_SCUB_Ti_Fin     : std_logic;

  signal    S_SRQ_Ena       : std_logic_vector(nSCUB_SRQ_Slaves'range);
  signal    S_SRQ_Sync        : std_logic_vector(nSCUB_SRQ_Slaves'range);
  signal    S_SRQ_active      : std_logic_vector(nSCUB_SRQ_Slaves'range);
  signal    S_one_or_more_SRQs_act  : std_logic;

  signal    S_Status        : std_logic_vector(15 downto 0);

  signal    S_SCUB_Version      : std_logic_vector(7 downto 0);
  signal    S_SCUB_Revision     : std_logic_vector(7 downto 0);

  signal    S_SCU_Bus_Access_Active : std_logic;
  signal    s_stall     : std_logic;

  signal    S_Invalid_Slave_Nr    : std_logic;
  signal    S_Invalid_Intern_Acc  : std_logic;

  signal    S_Intern_Echo_1     : std_logic_vector(15 downto 0);

  signal    s_global_intr_ena   : std_logic_vector(15 downto 0);
  signal    s_sw_tag_low        : std_logic_vector(15 downto 0);
  signal    s_sw_tag_high       : std_logic_vector(15 downto 0);

  signal    s_int_ack           : std_logic;
  signal    s_ext_ack           : std_logic;
  signal    s_ext_read_err      : std_logic;
  signal    s_adr               : std_logic_vector(15 downto 0);
  signal    s_ack               : std_logic;
  signal    s_err               : std_logic;

  signal    wr_acc              : std_logic;
  signal    rd_acc              : std_logic;

  signal    tag_fifo_we         : std_logic;
  signal    tag_fifo_rd         : std_logic;
  signal    tag_fifo_empty      : std_logic;
  signal    tag_fifo_full       : std_logic;
  signal    tag_fifo_q          : std_logic_vector(31 downto 0);
  signal    tag_fifo_in         : std_logic_vector(31 downto 0);

  signal    s_sw_tag            : std_logic;

  type  T_SCUB_SM is  (
              Idle,
              S_Rd_Cyc,   -- start read SCU_Bus cycle
              Rd_Cyc,     -- read SCU_Bus read active
              E_Rd_Cyc,   -- end read SCU_Bus
              F_Rd_Cyc,   -- finish read SCU_Bus
              TO_Rd_Cyc,  -- time out read cycle
              S_Wr_Cyc,   -- start write SCU_Bus cycle
              Wr_Cyc,     -- write SCU_Bus active
              E_Wr_Cyc,   -- end write SCU_Bus
              F_Wr_Cyc,   -- finish write SCU_Bus
              TO_Wr_Cyc,  -- time out write cycle
              S_Ti_Cyc,   -- start Timing cycle
              Ti_Cyc,     -- Timing cycle active
              E_Ti_Cyc,   -- end Timing cycle
              F_Ti_Cyc    -- finish time cycle
              );

  signal  SCUB_SM : T_SCUB_SM;

  type wb_ctrl_type is ( idle, cyc_wait, cyc_start, int_acc, ext_stall, ext_err, ext_acc, invalid_slave);

  signal wb_state : wb_ctrl_type;

  constant  bit_scub_wr_err:    integer := 0;
  constant  bit_scub_rd_err:    integer := 1;
  constant  bit_ti_cyc_err:     integer := 2;
  constant  bit_inval_intern_acc: integer := 3;
  constant  bit_inval_slave_nr:   integer := 4;
  constant  bit_scub_srqs_active: integer := 5;

begin

-- mapping of the wishbone signals
Wr_Data                   <= slave_i.dat(15 downto 0) when slave_i.sel(0) = '1' else slave_i.dat(31 downto 16);
slave_o.dat(31 downto 16) <= Rd_Data;
slave_o.dat(15 downto 0)  <= Rd_Data;
Adr                       <= slave_i.adr(16 downto 1);
Slave_Nr                  <= slave_i.adr(20 downto 17);
Start_Cycle               <= slave_i.cyc and slave_i.stb;
Wr                        <= slave_i.we;
Rd                        <= not slave_i.we;
slave_o.stall             <= s_stall;
slave_o.ack               <= s_ack;
slave_o.err               <= s_err;
slave_o.rty               <= '0';





S_SCUB_Version  <= std_logic_vector(to_unsigned(C_SCUB_Version, S_SCUB_Version'length));  -- set the version of this macro
S_SCUB_Revision <= std_logic_vector(to_unsigned(C_SCUB_Revision, S_SCUB_Revision'length));  -- set the revision of this macro

assert (False)
  report "SCU_Bus_Master_Macro: Version --> " & integer'image(C_SCUB_Version)
      & ", Revision is --> " & integer'image(C_SCUB_Revision)
severity NOTE;


assert NOT (Clk_in_Hz < 100000000)
  report "Achtung Generic Clk_in_Hz ist auf " & integer'image(Clk_in_Hz)
      & " gesetzt. Mit der Periodendauer von " & integer'image(Clk_in_ns)
      & " ns lassen sich keine genauen Verzoegerungen erzeugen!"

severity Warning;

assert (c_dly_multicast_dt_cnt+2 <= C_time_out_cnt)
  report "Achtung der multicast delay count " & integer'image(c_dly_multicast_dt_cnt+2)
      & " muss um mindestens 2 kleiner sein als der time_out_cnt = " & integer'image(C_time_out_cnt+2)
severity Error;

assert (False)
  report "time_out_in_ns = " & integer'image(time_out_in_ns)
      & ",   Clk_in_ns = " & integer'image(Clk_in_ns)
      & ",   C_time_out_cnt = " & integer'image(C_time_out_cnt+2)
severity NOTE;

assert (False)
  report "Sel_dly_in_ns = " & integer'image(Sel_dly_in_ns)
      & ",   C_Sel_dly_cnt = " & integer'image(C_Sel_dly_cnt+2)
      & ",   Sel_release_in_ns = " & integer'image(Sel_release_in_ns)
      & ",   Sel_release_cnt = " & integer'image(C_Sel_release_cnt+2)
severity NOTE;

assert (False)
  report "Timing_str_in_ns = " & integer'image(Timing_str_in_ns)
      & ",   C_Timing_str_cnt = " & integer'image(C_Timing_str_cnt+2)
      & ",   D_Valid_to_DS_in_ns = " & integer'image(D_Valid_to_DS_in_ns)
      & ",   C_D_Valid_to_DS_cnt = " & integer'image(C_D_Valid_to_DS_cnt+2)
severity NOTE;


P_Reset:  process (clk, nrst)
  begin
    if rising_edge(clk) then
      S_First_Sync_Reset <= nrst;
      s_reset <= S_First_Sync_Reset;
    end if;
  end process P_Reset;



S_Status(15)  <= '0';
S_Status(14)  <= '0';
S_Status(13)  <= '0';
S_Status(12)  <= '0';
S_Status(11)  <= '0';
S_Status(10)  <= '0';
S_Status(9)   <= '0';
S_Status(8)   <= '0';
S_Status(7)   <= '0';
S_Status(6)   <= '0';
S_Status(bit_scub_srqs_active)  <= S_one_or_more_SRQs_act;
S_Status(bit_inval_slave_nr)    <= S_Invalid_Slave_Nr;
S_Status(bit_inval_intern_acc)  <= S_Invalid_Intern_Acc;
S_Status(bit_ti_cyc_err)        <= S_Ti_Cyc_Err;
S_Status(bit_scub_rd_err)       <= S_SCUB_Rd_Err_no_Dtack;
S_Status(bit_scub_wr_err)       <= S_SCUB_Wr_Err_no_Dtack;


tag_fifo_in <= s_sw_tag_high & s_sw_tag_low when s_sw_tag = '1' else timing_in;

tag_fifo: generic_sync_fifo
generic map (
              g_data_width  => 32,
              g_size        => 10)

port map (
            rst_n_i => s_reset,
            clk_i   => clk,
            d_i     => tag_fifo_in,
            we_i    => tag_fifo_we,
            q_o     => tag_fifo_q,
            rd_i    => tag_fifo_rd,

            empty_o => tag_fifo_empty,
            full_o  => tag_fifo_full);




p_wb_ctrl: process (clk, s_reset)
begin
  if s_reset = '0' then
    s_stall                 <= '0';
    s_ext_read_err          <= '0';
    s_ack                   <= '0';
    s_err                   <= '0';
    wb_state                <= idle;
    S_Multi_Wr_Flag         <= '0';
    S_Start_SCUB_Rd         <= '0';             -- reset start SCU_Bus read
    S_Start_SCUB_Wr         <= '0';             -- reset start SCU_Bus write

  elsif rising_edge(clk) then

    s_ext_read_err          <= '0';
    s_stall                 <= '0';
    s_ack                   <= '0';
    s_err                   <= '0';
    S_Multi_Wr_Flag         <= '0';
    S_Start_SCUB_Rd         <= '0';             -- reset start SCU_Bus read
    S_Start_SCUB_Wr         <= '0';             -- reset start SCU_Bus write



    case wb_state is

      when idle =>
        S_Multi_Wr_Flag <= '0';                                     -- clear signal multicast write
        if slave_i.cyc = '1' and slave_i.stb = '1' then             -- begin of wishbone cycle
          if slave_i.sel(0) = '1' and slave_i.adr(1) = '0' then     -- fix for LM32
            s_adr <= std_logic_vector(unsigned(adr) + 1);           -- register address and slave_nr
          else
            s_adr <= adr;
          end if;
          s_slave_nr <= slave_nr;
          if Wr = '1' then
            S_Wr_Data <= Wr_Data;                                   -- register data
          end if;
          s_stall <= '1';                                           -- no pipelining
          if tag_fifo_empty = '1' and SCUB_SM = idle then           -- no active or planned timing cycle
            wb_state <= cyc_start;
          else
            wb_state <= cyc_wait;
          end if;
        end if;

      when cyc_wait =>
          s_stall <= '1';
          if tag_fifo_empty = '1' and SCUB_SM = idle then           -- no active or planned timing cycle
            wb_state <= cyc_start;
          end if;

      when cyc_start =>
          s_stall <= '1';
          if s_slave_nr = x"0" then                               -- internal access
            wb_state <= int_acc;
          elsif s_slave_nr >= x"1" and s_slave_nr <= x"e" then    -- external bus access
            if Wr = '1' then
              S_Start_SCUB_Wr <= '1';                             -- store write request
              --S_Wr_Data <= Wr_Data;                               -- store write pattern
            elsif Rd = '1' then
              S_Start_SCUB_Rd <= '1';                             -- store read request
            end if;
            wb_state <= ext_stall;
          elsif s_slave_nr = c_multicast_slave_acc then
            if Wr = '1' then
              S_Start_SCUB_Wr <= '1';                             -- store write request
              --S_Wr_Data <= Wr_Data;                               -- store write pattern
              S_Multi_Wr_Flag <= '1';                             -- signal multicast write
              wb_state <= ext_stall;                              -- multicast read not allowed
            else
              wb_state <= ext_err;                                -- multicast read not allowed
            end if;
          else                                                    -- slave number invalid
            wb_state <= invalid_slave;
          end if;

      when int_acc =>                                             -- ack/err only for one clock cycle
        S_Multi_Wr_Flag <= '0';                                   -- clear signal multicast write
        if S_Invalid_Intern_Acc = '1' then
          Rd_Data <= x"dead";
          s_err <= '1';
        elsif s_int_ack = '1' then
          Rd_Data <= int_rd_data;
          s_ack <= '1';
        end if;
        wb_state <= idle;

      when ext_stall =>
        s_stall <= '1';                                           -- no pipelining, stall until dtack or timeout
        if S_SCUB_Rd_Err_no_Dtack = '1' or S_SCUB_Wr_Err_no_Dtack = '1' then
          wb_state <= ext_err;
        elsif s_ext_ack = '1' then
          Rd_Data <= ext_rd_data;
          wb_state <= ext_acc;
        end if;

      when ext_acc =>
        S_Multi_Wr_Flag <= '0';                                   -- clear signal multicast write
        s_ack <= '1';
        wb_state <= idle;

      when ext_err =>
        S_Multi_Wr_Flag <= '0';                                   -- clear signal multicast write
        Rd_Data <= x"dead";
        s_err <= '1';
        wb_state <= idle;

      when invalid_slave =>
        Rd_Data <= x"dead";
        s_err <= '1';
        wb_state <= idle;
    end case;

  end if;
end process p_wb_ctrl;


wr_acc <= '1' when Wr = '1' and wb_state = cyc_start else '0';
rd_acc <= '1' when Rd = '1' and wb_state = cyc_start else '0';

int_regs: process (clk, s_reset)
begin
  if s_reset = '0' then
    S_Multi_Slave_Sel   <= (others => '0');     -- clear Register which contains the bit_vector
                                                -- to address multible slaves during one SCU write access
    S_SRQ_Ena           <= (others => '0');     -- all SRQs[12..1] are disabled
    S_Intern_Echo_1     <= (others => '0');
    S_Global_Intr_Ena   <= (others => '0');
    s_sw_tag_low        <= (others => '0');
    s_sw_tag_high       <= (others => '0');

    S_SCUB_Rd_Err_no_Dtack  <= '0';             -- reset read timeout flag
    S_SCUB_Wr_Err_no_Dtack  <= '0';             -- reset write timeout flag
    S_Ti_Cyc_Err            <= '0';             -- reset timing error flag
    S_Start_Ti_Cy           <= '0';             -- reset start SCU_Bus timing cycle
    S_Ti_Cy(S_Ti_Cy'range)  <= (others => '0'); -- shift reg to generate pulse

    s_Invalid_Intern_Acc    <= '0';
    S_Invalid_Slave_Nr      <= '0';
    s_int_ack               <= '0';
    tag_fifo_we             <= '0';
    s_sw_tag                <= '0';

  elsif rising_edge(clk) then
    S_SCUB_Rd_Err_no_Dtack  <= '0';
    S_SCUB_Wr_Err_no_Dtack  <= '0';
    tag_fifo_we             <= '0';

    if wb_state = idle then             -- clear ack and err for next cycle
      s_int_ack <= '0';
      S_Invalid_Intern_Acc <= '0';
    end if;


    if SCUB_SM = TO_Rd_Cyc then
      S_SCUB_Rd_Err_no_Dtack <= '1';    -- SCU_Bus read error no dtack
    end if;

    if SCUB_SM = TO_Wr_Cyc then
      S_SCUB_Wr_Err_no_Dtack <= '1';    -- SCU_Bus write error no dtack
    end if;


    case unsigned(s_adr(c_adr_width-1 downto 0)) is
      when C_Status_Adr =>
        if wr_acc = '1' then
          s_int_ack <= '1';
          if Wr_Data(bit_scub_wr_err) = '1' then          -- look to the bit position in status
            S_SCUB_Wr_Err_no_Dtack <= '0';                -- reset SCU_Bus write error no dtack.
          end if;
          if Wr_Data(bit_scub_rd_err) = '1' then          -- look to the bit position in status!
            S_SCUB_Rd_Err_no_Dtack <= '0';                -- reset SCU_Bus read error no dtack
          end if;
          if Wr_Data(bit_ti_cyc_err) = '1' then           -- look to the bit position in status!
            S_Ti_Cyc_Err <= '0';                          -- reset SCU_Bus timing error
          end if;
          if Wr_Data(bit_inval_intern_acc) = '1' then     -- look to the bit position in status!
            S_Invalid_Intern_Acc <= '0';                  -- reset invalid internal register access error
          end if;
          if Wr_Data(bit_inval_slave_nr) = '1' then       -- look to the bit position in status!
            S_Invalid_Slave_Nr <= '0';                    -- reset invalid slave number error
          end if;
        elsif rd_acc = '1'  then
          s_int_ack <= '1';
          int_rd_data <= S_Status;
        end if;

      when C_Global_Intr_Ena_Adr =>
        if wr_acc = '1' then
            s_int_ack <= '1';
            S_Global_Intr_Ena(bit_scub_wr_err) <= Wr_Data(bit_scub_wr_err);
            S_Global_Intr_Ena(bit_scub_rd_err) <= Wr_Data(bit_scub_rd_err);
            S_Global_Intr_Ena(bit_ti_cyc_err) <= Wr_Data(bit_ti_cyc_err);
            S_Global_Intr_Ena(bit_inval_intern_acc) <= Wr_Data(bit_inval_intern_acc);
            S_Global_Intr_Ena(bit_inval_slave_nr) <= Wr_Data(bit_inval_slave_nr);
            S_Global_Intr_Ena(bit_scub_srqs_active) <= Wr_Data(bit_scub_srqs_active);
        elsif rd_acc = '1' then
            s_int_ack <= '1';
            int_rd_data <= S_Global_Intr_Ena;
        end if;

      when C_SRQ_Ena_Adr =>
        if wr_acc = '1'  then
              s_int_ack <= '1';
              S_SRQ_Ena <= Wr_Data(nSCUB_SRQ_Slaves'range);
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= ("000" & S_SRQ_Ena);
      end if;

      when C_Srq_active_Adr =>
        if wr_acc = '1' then
              S_Invalid_Intern_Acc <= '1';
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= ("000" & S_SRQ_Active);
            end if;

      when C_Srq_In_Adr =>
        if wr_acc = '1' then
              S_Invalid_Intern_Acc <= '1';
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= ("000" & S_SRQ_Sync);
            end if;

      when C_Vers_Revi_Adr =>
        if wr_acc = '1' then
              S_Invalid_Intern_Acc <= '1';
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= (S_SCUB_Version & S_SCUB_Revision);
            end if;

      when C_Wr_Multi_Slave_Sel_Adr =>
        if wr_acc = '1' then
              s_int_ack <= '1';
              S_Multi_Slave_Sel <= Wr_Data(nSCUB_Slave_Sel'range);
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= ("000" & S_Multi_Slave_Sel);
            end if;

      when C_Bus_master_intern_Echo_1_Adr =>
        if wr_acc = '1' then
              s_int_ack <= '1';
              S_Intern_Echo_1 <= Wr_Data;
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= S_Intern_Echo_1;
            end if;

      when C_Sw_Tag_Low_Adr =>
        if wr_acc = '1' then
              s_int_ack <= '1';
              s_sw_tag_low <= Wr_Data; -- store the low 16Bit of the software triggered SCUbus tag
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= s_sw_tag_low;
            end if;

      when C_Sw_Tag_High_Adr =>
        if wr_acc = '1' then
              s_int_ack <= '1';
              s_sw_tag <= '1';
              s_sw_tag_high <= Wr_Data; -- store the high 16Bit of the software triggered SCUbus tag
        elsif rd_acc = '1' then
              s_int_ack <= '1';
              int_rd_data <= s_sw_tag_high;
            end if;

      when others =>
        if wb_state = cyc_start then
            S_Invalid_Intern_Acc <= '1';
        end if;
    end case;

    S_Ti_Cy(S_Ti_Cy'range) <= (S_Ti_Cy(S_Ti_Cy'high-1 downto 0) & Start_Timing_Cycle);    -- shift reg to generate pulse


    if S_Ti_Cy = "01" or (s_sw_tag = '1' and s_ack = '1') then     -- positive edge off start_timing_cycle
      if tag_fifo_full = '1' then
        S_Ti_Cyc_Err <= '1';                     -- FifO full
      else
        S_Start_Ti_Cy <= '1';                    -- store timing request
        tag_fifo_we <= '1';                      -- store tag in fifo
      end if;
    end if;

    if SCUB_SM = E_Ti_Cyc then
         S_Start_Ti_Cy <= '0';
         s_sw_tag <= '0';
    end if;


  end if;
end process;





P_SCUB_SM:  process (clk, s_reset)
begin
  if s_reset = '0' then
    SCUB_SM             <= Idle;
    S_Last_Cycle_Timing <= '0';
    S_SCUB_Timing_Cycle <= '0';
    S_SCUB_RDnWR        <= '1';
    S_SCUB_DS           <= '0';
    S_SCUB_Slave_Sel    <= (others => '0');
    S_Sel_Ext_Data_Drv  <= '0';
    tag_fifo_rd         <= '0';

  elsif rising_edge(clk) then

    if Test = 0 then
      S_nSync_Dtack(0) <= nSCUB_Dtack;  -- SCU_Bus_Dtack is an asynchronous Signal. S_nSync_Dtack is the synchronized nSCU_Bus_Dtack
      s_nSync_Dtack(1) <= s_nSync_Dtack(0);
    else
      S_nSync_Dtack(0) <= not S_SCUB_DS; -- during test mode S_nSync_dtack is gererated with the S_SCUB_DS signal
    end if;

    if S_nSync_Dtack(1) = '0' and s_nSync_Dtack(0) = '1' then -- ack pulse from Dtack
      s_ext_ack <= '1';
    else
      s_ext_ack <= '0';
    end if;

    tag_fifo_rd <= '0';

    case SCUB_SM is         -- = SCU_Bus State Machine

      when Idle =>
        S_Sel_dly_cnt       <= to_unsigned(C_Sel_dly_cnt, S_Sel_dly_cnt'length);
        S_D_Valid_to_DS_cnt <= to_unsigned(C_D_Valid_to_DS_cnt, S_D_Valid_to_DS_cnt'length);
        S_Sel_release_cnt   <= to_unsigned(C_Sel_release_cnt, S_Sel_release_cnt'length);
        S_SCUB_Slave_Sel    <= (others => '0');
        S_SCUB_Addr         <= (others => '1');
        S_SCUB_RDnWR        <= '1';
        S_SCUB_Timing_Cycle <= '0';
        S_SCUB_DS           <= '0';
        S_Sel_Ext_Data_Drv  <= '0';


        --if ((S_Start_SCUB_Rd = '1') and (tag_fifo_empty = '1')) then
        if (S_Start_SCUB_Rd = '1') then
          S_SCUB_Addr <= s_adr;                   -- store slave address
          SCUB_SM <= S_Rd_Cyc;                    -- jump to start read cycle
        --elsif ((S_Start_SCUB_Wr = '1') and (tag_fifo_empty = '1')) then
        elsif (S_Start_SCUB_Wr = '1') then
          S_SCUB_Addr <= s_adr;                   -- store slave address
          S_SCUB_RDnWR <= '0';                    -- set master writes
          SCUB_SM <= S_Wr_Cyc;                    -- jump to start write cycle
        elsif (tag_fifo_empty = '0' and (wb_state = idle or wb_state = cyc_wait)) then
          S_SCUB_RDnWR  <= '0';                   -- set master writes
          tag_fifo_rd   <= '1';                   -- read tag from fifo
          SCUB_SM <= S_Ti_Cyc;                    -- jump to start Timing cycle
        else
          null;
        end if;

      when S_Rd_Cyc =>                            -- start read cycle
        S_Sel_Ext_Data_Drv <= '1';
        S_Last_Cycle_Timing <= '0';               -- last SCU_Bus cycle is a data transfer cycle
        if S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' then
          S_SCUB_Slave_Sel <= S_Slave_Sel(12 downto 0);   -- select slave
          SCUB_SM <= Rd_Cyc;                      -- jump to active read cycle
        end if;

      when Rd_Cyc =>                              -- read cycle active
        if S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' then
          S_SCUB_DS <= '1';
          if S_nSync_Dtack(0) = '0' then          -- wait for Dtack
            if Test = 0 then
              ext_rd_data <= SCUB_Data_In;           -- during production: read the SCUB_Data bidir buffer
            else
              ext_rd_data <= S_Wr_Data;           -- during test: return the last written data
            end if;
            S_SCUB_DS <= '0';
            S_SCUB_Slave_Sel <= (others => '0');
            SCUB_SM <= E_Rd_Cyc;                  -- jump to end read cycle
          elsif s_time_out_cnt(s_time_out_cnt'high) = '1' then
            S_SCUB_DS <= '0';
            S_SCUB_Slave_Sel <= (others => '0');
            SCUB_SM <= TO_Rd_Cyc;                 -- jump to read timeout
          end if;
        end if;

      when TO_Rd_Cyc =>                           -- read timeout
        SCUB_SM <= E_Rd_Cyc;                      -- jump to E_Rd_Cyc

      when E_Rd_Cyc =>                            -- end read cycle
        S_Sel_Ext_Data_Drv <= '0';
        if S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' then
          SCUB_SM <= F_Rd_Cyc;                    -- jump to finish read cycle
        end if;

      when F_Rd_Cyc =>
        SCUB_SM <= Idle;                          -- jump to Idle

      when S_Wr_Cyc =>                            -- start write cycle
        S_Last_Cycle_Timing <= '0';               -- last SCU_Bus cycle is a data transfer cycle
        S_Sel_Ext_Data_Drv <= '1';
        if S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' then
          S_SCUB_Slave_Sel <= S_Slave_Sel(12 downto 0);   -- select slave
          SCUB_SM <= Wr_Cyc;                      -- jump to active write cycle
        end if;

      when Wr_Cyc =>                              -- write cycle active
        if S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' then
          S_SCUB_DS <= '1';
          if    (S_Multi_Wr_Flag = '0' and S_nSync_Dtack(0) = '0')                      -- wait for indivdual slave dtack
            OR  (S_Multi_Wr_Flag = '1'                                                  -- wait for first slave dtack during multicast wr and delay it for slowlier slaves
                  and s_dly_multicast_dt_cnt(s_dly_multicast_dt_cnt'high) = '1'
                  and S_nSync_Dtack(0) = '0')
            OR  (s_time_out_cnt(s_time_out_cnt'high) = '1')                             -- if no dtack wait for timeout
          then
            S_SCUB_DS <= '0';
            S_Sel_Ext_Data_Drv <= '0';
            S_SCUB_Slave_Sel <= (others => '0');
            if s_time_out_cnt(s_time_out_cnt'high) = '0' then -- no timeout
              SCUB_SM <= E_Wr_Cyc;              -- jump to end write cycle
            else
              SCUB_SM <= TO_Wr_Cyc;             -- jump to write timeout
            end if;
          end if;
        end if;

      when TO_Wr_Cyc =>                     -- write timeout
          S_SCUB_RDnWR <= '1';                -- set master reades
          SCUB_SM <= E_Wr_Cyc;                -- jump to Idle

      when E_Wr_Cyc =>                      -- end write cycle
        if S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' then
          S_SCUB_RDnWR <= '1';                -- set master reades
          SCUB_SM <= F_Wr_Cyc;                -- jump to finish write cycle
        end if;

      when F_Wr_Cyc =>
          SCUB_SM <= Idle;                  -- jump to Idle

      when S_Ti_Cyc =>                                    -- start Timing cycle
        S_Last_Cycle_Timing <= '1';                       -- last SCU_Bus cycle is a timing cycle

        if S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' then

          S_Sel_Ext_Data_Drv <= '1';
          S_SCUB_Slave_Sel <= (others => '1');            -- in this version select all slaves.
          S_Timing_str_cnt <= to_unsigned(C_Timing_str_cnt, S_Timing_str_cnt'length);
          SCUB_SM <= Ti_Cyc;                              -- jump to active Timing cycle
        end if;

      when Ti_Cyc =>                                    -- Timing cycle active
        S_SCUB_Timing_Cycle <= '1';                     -- timing cycle signal active
        S_SCUB_Addr <= tag_fifo_q(31 downto 16);        -- Timing to S_SCUB_Addr
        if S_Timing_str_cnt(S_Timing_str_cnt'high) = '1' then
          S_SCUB_Timing_Cycle <= '0';                   -- timing cycle signal inactive
          S_SCUB_Slave_Sel <= (others => '0');          -- deselect all slaves.
          SCUB_SM <= E_Ti_Cyc;                          -- jump to end Timing cycle
        end if;

      when E_Ti_Cyc =>                      -- end Timing cycle
        if S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' then
          S_SCUB_RDnWR <= '1';                -- set master reades
          S_Sel_Ext_Data_Drv <= '0';
          SCUB_SM <= F_Ti_Cyc;                -- jump to finish time cycle
        end if;

      when F_Ti_Cyc =>
          SCUB_SM <= Idle;                  -- jump to Idle

      when others =>
        SCUB_SM <= Idle;

    end case;


    if ((SCUB_SM = S_Wr_Cyc) OR (SCUB_SM = S_Rd_Cyc) OR (SCUB_SM = S_Ti_Cyc)) AND S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '0' then
      S_Sel_dly_cnt <= S_Sel_dly_cnt - 1;
    end if;

    if ((SCUB_SM = Wr_Cyc) OR (SCUB_SM = Rd_Cyc)) AND S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '0' then
      S_D_Valid_to_DS_cnt <= S_D_Valid_to_DS_cnt - 1;
    end if;

    if ((SCUB_SM = E_Wr_Cyc) OR (SCUB_SM = E_Rd_Cyc) OR (SCUB_SM = E_Ti_Cyc)) AND S_Sel_release_cnt(S_Sel_release_cnt'high) = '0' then
      S_Sel_release_cnt <= S_Sel_release_cnt - 1;
    end if;

    if SCUB_SM = Ti_Cyc AND S_Timing_str_cnt(S_Timing_str_cnt'high) = '0' then
      S_Timing_str_cnt <= S_Timing_str_cnt - 1;
    end if;

  end if;
end process P_SCUB_SM;


p_board_sel:  process (clk, s_reset)
  begin
    if s_reset = '0' then
      S_Slave_Sel <= "0000000000000";            -- no board select
    elsif rising_edge(clk) then
      case S_Slave_Nr IS
        when X"0" =>  S_Slave_Sel <= "0000000000000";
        when X"1" =>  S_Slave_Sel <= "0000000000001";  -- select board 1
        when X"2" =>  S_Slave_Sel <= "0000000000010";
        when X"3" =>  S_Slave_Sel <= "0000000000100";
        when X"4" =>  S_Slave_Sel <= "0000000001000";
        when X"5" =>  S_Slave_Sel <= "0000000010000";
        when X"6" =>  S_Slave_Sel <= "0000000100000";
        when X"7" =>  S_Slave_Sel <= "0000001000000";
        when X"8" =>  S_Slave_Sel <= "0000010000000";
        when X"9" =>  S_Slave_Sel <= "0000100000000";
        when X"A" =>  S_Slave_Sel <= "0001000000000";
        when X"B" =>  S_Slave_Sel <= "0010000000000";
        when X"C" =>  S_Slave_Sel <= "0100000000000";  -- select board 12
        when c_multicast_slave_acc =>
                if S_Start_SCUB_Wr = '1' then -- select boardcast
                  S_Slave_Sel <= S_Multi_Slave_Sel;
                else
                  S_Slave_Sel <= "0000000000000";
                end if;
        when X"E" =>  S_Slave_Sel <= "1000000000000";  -- select virtual slave
        when others =>  S_Slave_Sel <= "0000000000000";  -- no board select
      end case;
    end if;
  end process p_board_sel;


irq_deglitch: process(clk, s_reset)
  type cnt_array is array (0 to 12) of integer range 0 to 5;
  variable cnt : cnt_array;
  type regarray is array (0 to 12) of std_logic_vector(4 downto 0);
  variable shiftreg : regarray;
begin
  if rising_edge(clk) then

    if s_reset = '0' then
      for i in 0 to 12 loop
        cnt(i) := 0;
        shiftreg(i) := (others => '0');
      end loop;
    else
      for i in 0 to 11 loop
        shiftreg(i) := shiftreg(i)(3 downto 0) & S_SRQ_active(i);

        if shiftreg(i)(0) = '1' then
          cnt(i) := cnt(i) + 1;
        end if;
        if shiftreg(i)(4) = '1' then
          cnt(i) := cnt(i) - 1;
        end if;

        if cnt(i) = 3 then
          srq_active(i) <= '1';
        elsif cnt(i) < 3 then
          srq_active(i) <= '0';
        end if;
      end loop;
    end if;
  end if;

end process;

p_intr: process (clk, s_reset)
  begin
    if s_reset = '0' then
      S_SRQ_Sync    <= "0000000000000";          -- clear synchronized SRQs
      S_SRQ_active  <= "0000000000000";          -- clear active SRQs
      S_one_or_more_SRQs_act <= '0';
      Intr      <= '0';

    elsif rising_edge(clk) then

      S_SRQ_Sync <= NOT nSCUB_SRQ_Slaves;         -- synchronize and change level of nSCUB_SRQ_Slave signals
                                -- S_SRQ_Sync(n) = '1' => nSCUB_SRQ_Slaves(n) is active
      for i IN nSCUB_SRQ_Slaves'range loop
        if S_SRQ_Ena(i) = '1' then
          if S_SRQ_Sync(i) = '1' then
            S_SRQ_active(i) <= '1';
          else
            S_SRQ_active(i) <= '0';
          end if;
        else
          S_SRQ_active(i) <= '0'; -- ???
        end if;
      end loop;

      if S_SRQ_active /= std_logic_vector(to_unsigned( 0, nSCUB_SRQ_Slaves'length)) then
        S_one_or_more_SRQs_act <= '1';
      else
        S_one_or_more_SRQs_act <= '0';
      end if;

      if    (S_SCUB_Wr_Err_no_Dtack = '1' AND S_Global_Intr_Ena(bit_scub_wr_err) = '1')
        OR  (S_SCUB_Rd_Err_no_Dtack = '1' AND S_Global_Intr_Ena(bit_scub_rd_err) = '1')
        OR  (S_Ti_Cyc_Err = '1' AND S_Global_Intr_Ena(bit_ti_cyc_err) = '1')
        OR  (S_Invalid_Intern_Acc = '1' AND S_Global_Intr_Ena(bit_inval_intern_acc) = '1')
        OR  (S_Invalid_Slave_Nr = '1' AND S_Global_Intr_Ena(bit_inval_slave_nr) = '1')
        OR  (S_one_or_more_SRQs_act = '1' AND  S_Global_Intr_Ena(bit_scub_srqs_active) = '1')
      then
        Intr <= '1';
      else
        Intr <= '0';
      end if;

    end if;
  end process p_intr;


P_SCUB_Tri_State: process (SCUB_SM, S_Wr_Data, tag_fifo_q)
  begin
    if (SCUB_SM = S_Wr_Cyc) OR (SCUB_SM = Wr_Cyc) then
      SCUB_Data_Out <= S_Wr_Data;
    elsif (SCUB_SM = Ti_Cyc) OR (SCUB_SM = E_Ti_Cyc) then
      SCUB_Data_Out <= tag_fifo_q(15 downto 0);
    else
      SCUB_Data_Out <= (others => '0');
    end if;
  end process P_SCUB_Tri_State;
  SCUB_Data_Tri_Out <= '1' when (SCUB_SM = S_Wr_Cyc) OR (SCUB_SM = Wr_Cyc) OR (SCUB_SM = Ti_Cyc) OR (SCUB_SM = E_Ti_Cyc) else '0';


p_time_out: process (Clk, s_reset)
  begin
    if s_reset = '0' then
      s_time_out_cnt <= to_unsigned(C_time_out_cnt, s_time_out_cnt'length);
    elsif rising_edge(Clk) then
      if NOT ((SCUB_SM = Rd_Cyc) OR (SCUB_SM = Wr_Cyc)) then
        s_time_out_cnt <= to_unsigned(C_time_out_cnt, s_time_out_cnt'length);
      elsif s_time_out_cnt(s_time_out_cnt'high) = '0' then                  -- no underflow
        s_time_out_cnt <= s_time_out_cnt - 1;                       -- count down
      end if;
    end if;
  end process p_time_out;

p_delay_multicast_dt: process (Clk, s_reset)
  begin
    if s_reset = '0' then
      s_dly_multicast_dt_cnt <= to_unsigned(c_dly_multicast_dt_cnt, s_dly_multicast_dt_cnt'length);
    elsif rising_edge(Clk) then
      if SCUB_SM /= Wr_Cyc or S_Multi_Wr_Flag = '0' then
        s_dly_multicast_dt_cnt <= to_unsigned(c_dly_multicast_dt_cnt, s_dly_multicast_dt_cnt'length);
      elsif s_dly_multicast_dt_cnt(s_dly_multicast_dt_cnt'high) = '0' then                  -- no underflow
        s_dly_multicast_dt_cnt <= s_dly_multicast_dt_cnt - 1;                       -- count down
      end if;
    end if;
  end process p_delay_multicast_dt;


SCUB_Addr               <= S_SCUB_Addr;
SCUB_RDnWR              <= S_SCUB_RDnWR;
nSCUB_DS                <= NOT S_SCUB_DS;
nSCUB_Slave_Sel         <= NOT S_SCUB_Slave_Sel;

nSCUB_Timing_Cycle      <= NOT S_SCUB_Timing_Cycle;

nSel_Ext_Data_Drv       <= NOT S_Sel_Ext_Data_Drv;

SCUB_Rd_active          <= S_Start_SCUB_Rd;
SCUB_Rd_Fin             <= '1' when SCUB_SM = F_Rd_Cyc else '0';
SCUB_Rd_Err_no_Dtack    <= S_SCUB_Rd_Err_no_Dtack;

SCUB_Wr_active          <= S_Start_SCUB_Wr;
SCUB_Wr_Fin             <= '1' when SCUB_SM = F_Wr_Cyc else '0';
SCUB_Wr_Err_no_Dtack    <= S_SCUB_Wr_Err_no_Dtack;

S_SCUB_Ti_Fin           <= '1' when SCUB_SM = F_Ti_Cyc else '0';
SCUB_Ti_Fin             <= S_SCUB_Ti_Fin;

SCUB_Ti_Cyc_Err         <= S_Ti_Cyc_Err;

S_SCU_Bus_Access_Active <= '1' when (S_Start_SCUB_Wr = '1') OR (S_Start_SCUB_Rd = '1') else '0';
SCU_Bus_Access_Active   <= S_SCU_Bus_Access_Active;

SCU_Wait_Request        <= s_stall;

--srq_active              <= S_SRQ_active;

end Arch_SCU_Bus_Master;
