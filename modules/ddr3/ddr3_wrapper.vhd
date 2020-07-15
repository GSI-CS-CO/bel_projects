------------------------------------------------------------------------------
-- Title      : DDR3 Wrapper
-- Project    : Wishbone
------------------------------------------------------------------------------
-- File       : DDR3 Wrapper.vhd
-- Author     : Karlheinz Kaisr
-- Company    : GSI
-- Created    : 2016-07-07
-- Last update: 2016-07-07
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Interfacing ALTMEMPHY  150 MHz Local I/F to 62.5MHz Wishbone I/F
--              Clocknets are separated by clock crossing bridge resp. FIFO 512x64
--              wb_irq_master block included (for msi registers see wb_irq_master)
-- Features:
--         * Wishbone 1 Interface for
--             DDR3 transparent access
--             Register access  Burst_StartAdr Reg and XFer_Cnt Reg
--         * Wishbone 2 Interface for
--             Xfer_Fifo read-out
--             MSI Control Registers access (Message signalled Interrupts)
--
-- DDR Transparent access:
-- Translates 32 bit WB Access to 64 bit DDR3 Access in two steps (low and high word)
--         WR to Address 4,C,14,1C etc moves DDR HW  from WB in a latch
--         WR to Address 0,8,10,18 etc moves DDR LW from WB and Latch data to DDR
--         RD from Address 0,8,10,18 etc gets DDR LW to WB and HW to Latch
--         RD from Address 4,C,14,1C etc gets DDR HW stored in Latch to WB
--
-- DDR FIFO Buffered Readout for fast DDR read accesses
--         (1) WR DDR Start Address to Burst_StartAdr_Reg (max.#16#2F FFF FE8)
--         (2) WR number of 64 bit DDR Transfers to Xfer_Cnt Register
--             Transfer from DDR to FIFO starts immediately after WR on Xfer_Cnt Reg
--             During Transfer any other WB Action on WB1 IF is stalled
--         (3) As long "Fifo not Empty", it can be read out. Signalling is done
--             3a) Polling the FIFO status register
--                (Bit 31=1: Fifo is empty, Bit30=1:Local init done, Bit9..0: Used Words)
--          or 3b)Or serving the message-signalled interrupt
--
--          (4) 64 bit FIFO Data is presented in two steps (LW and HW)
--             4a) RD fifo_lw_register moves LW to WB and HW to latch and moves FIFO Pointer
--             4b) RD fifo_hw_register moves HW to WB
-- Read of an empty fifo by using fifo_lw_register results in a WB error.
--
-- Additional Features on burst_start_adr and usedw register values:
--   As burst_start_adr is incremented, used-word shows the contents of fifo level.
--   After filling the fifo with one burst of data, it should be emptied before placement
--   of an new burst_transfer job.
--
--   Fifo-buffered reads on consecutive DDR addresses can be done without new load of the
--   burst_startadr because burst_startadr has appropriate value from last transfer job.
--
--   According local DDR3 Memory address on read out can be retrieved by
--   read-out of the burst-startadr and  read-out of the used_word value.
--   The last fifo readout of a job belongs to the (burst_startadr - 1).
--   The DDR3 Memory address for the access before the last access is
--               burst_startadr value - 1 - used-word value  (which is 1 too)
-------------------------------------------------------------------------------
-- Copyright (c) 2016 GSI
-------------------------------------------------------------------------------

--
-- Revisions  :
-- Date         Version  Author          Description
-- 2016-07-07   1.0      kkaiser         Created
-------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

use work.WISHBONE_PKG.all;
use work.GENRAM_PKG.all;
use work.ddr3_wrapper_pkg.all;
use work.wb_irq_pkg.all;

-------------------------------------------------------------------------------------------------------------------

entity ddr3_wrapper is
port (
  clk_sys                                               : in    std_logic                     := '0';
  rstn_sys                                              : in    std_logic                     := '0';
  -- Wishbone
  slave_i_1                                             : in    t_wishbone_slave_in;
  slave_o_1                                             : out   t_wishbone_slave_out;
  slave_i_2                                             : in    t_wishbone_slave_in;
  slave_o_2                                             : out   t_wishbone_slave_out;
  -- master interface for msi generator
  irq_mst_i                                             : in    t_wishbone_master_in;
  irq_mst_o                                             : out   t_wishbone_master_out;
  -- DDR3 Device pins
  altmemddr_0_memory_mem_odt                            : out   std_logic_vector(0 downto 0);
  altmemddr_0_memory_mem_clk                            : inout std_logic_vector(0 downto 0)  := (others => '0');
  altmemddr_0_memory_mem_clk_n                          : inout std_logic_vector(0 downto 0)  := (others => '0');
  altmemddr_0_memory_mem_cs_n                           : out   std_logic_vector(0 downto 0);
  altmemddr_0_memory_mem_cke                            : out   std_logic_vector(0 downto 0);
  altmemddr_0_memory_mem_addr                           : out   std_logic_vector(12 downto 0);
  altmemddr_0_memory_mem_ba                             : out   std_logic_vector(2 downto 0);
  altmemddr_0_memory_mem_ras_n                          : out   std_logic;
  altmemddr_0_memory_mem_cas_n                          : out   std_logic;
  altmemddr_0_memory_mem_we_n                           : out   std_logic;
  altmemddr_0_memory_mem_dq                             : inout std_logic_vector(15 downto 0) := (others => '0');
  altmemddr_0_memory_mem_dqs                            : inout std_logic_vector(1 downto 0)  := (others => '0');
  altmemddr_0_memory_mem_dqsn                           : inout std_logic_vector(1 downto 0)  := (others => '0');
  altmemddr_0_memory_mem_dm                             : out   std_logic_vector(1 downto 0);
  altmemddr_0_memory_mem_reset_n                        : out   std_logic;
  altmemddr_0_external_connection_local_refresh_ack     : out   std_logic;
  altmemddr_0_external_connection_local_init_done       : out   std_logic;
  altmemddr_0_external_connection_reset_phy_clk_n       : out   std_logic;
  altmemddr_0_external_connection_dll_reference_clk     : out   std_logic;
  altmemddr_0_external_connection_dqs_delay_ctrl_export : out   std_logic_vector(5 downto 0)
  );
end entity ddr3_wrapper;


architecture rtl_ddr3_wrapper of ddr3_wrapper is

constant xfer_fifo_width            : integer                      := 64;
constant xfer_fifo_size             : integer                      := 512;
constant wb_fifo_size               : natural                      := 8;
constant zero                       : std_logic_vector(31 downto 0):= (others =>'0');

-- Some Address Calculation stuff:
-- first wishbone interface
-- WB address range must be by power of 2 (minus 1) to be synthesizable.
--
-- DDR3 needs 24 bit on local interface for 1 GBit because 2exp24 = 16.777.216 (which is 1GBit in 64 bit words).
-- WB addresses bytes. So we have 3 additional address lines to cover 64 bit words instead of bytes(64bit/8bit = 2exp3)
-- On Wishbone this is 0...7 fff fff = 0.125 GigaByte (which is - again - 1.073.741.824 bit=1Gbit)
--
-- Available Address Space for SCU in bytes is x 30 000 000 - x 23 000 000 = x D 000 000 =  218 103 808 bytes=1.7 Gbit
-- Needed are 134 217 728 bytes which is  x 8 000 000 bytes or 27 address lines.
--
-- There is not enough space to separate the 2 registers with 28 address lines on same i/f  which would need 268 435 456 bytes
-- Therefore the both registers for burst_startadr_reg_adr and xfer_cnt_adr are included at the end of the ddr address space.
-- So the Gbit Ram is not fully usable for sake of these both register addresses.
--
-- Some LW and HW Adressing Stuff on DDR transparent access:
-- Addresses for lower 32 bit are 0x0 and 0x8 ...  --> only the lower addresses xfers the words in/out of the DDR
-- Addresses for upper 32 bit are 0x4 and 0xc ...
-- Writing LW on WB towards DDR needs preload of the HW before. When writing on LW addresses, all 64 bits are moved to DDR.
-- Reading of LW gets 64 bits out of DDR and LW is presented on WB. HW is latched and can be get on a second access on HW address.
--
-- Some LW and HW Addressing Stuff for FIFO Access:
-- LW read on fifo_lw_register gets 64 bit out of fifo and presenting lw on wishbone whilst latching HW in fifo_hw_register.
-- HW can be got in a second wb access. Only LW access moves the Fifo pointer.


--first wb interface
constant ddr_start_adr              : integer                      := 16#0#;
constant ddr_end_adr                : integer                      := 16#7fffff0#;
constant burst_startadr_reg_adr     : integer                      := 16#7fffff4#; --26..0
constant xfer_cnt_adr               : integer                      := 16#7fffff8#;


--second wb interface
constant msi_start_adr              : integer                      := 16#0#;
constant msi_end_adr                : integer                      := 16#28#;
constant fifo_lw_adr                : integer                      := 16#30#;
constant fifo_hw_adr                : integer                      := 16#34#;
constant fifo_status_adr            : integer                      := 16#38#; -- range of 2nd wb if 0x0 .. 0xf
--------------------------------------------------------------------
signal local_addr_o                 : std_logic_vector (23 downto 0);
signal local_write_req_o            : std_logic;
signal local_read_req_o             : std_logic;
signal local_burstbegin_o           : std_logic;
signal local_wdata_o                : std_logic_vector (63 downto 0);
signal local_be_o                   : std_logic_vector (7 downto 0);
signal local_size_o                 : std_logic_vector (2 downto 0);
signal local_global_reset_n_o       : std_logic;
signal phy_clk                      : std_logic;
signal local_ready_i                : std_logic;
signal local_rdata_i                : std_logic_vector (63 downto 0);
signal local_wdata_latch            : std_logic_vector (31 downto 0);
signal local_rdata_valid_i          : std_logic;
signal local_reset_phy_clk_n        : std_logic;
signal local_burstbegin_t           : std_logic;
signal local_write_req_o_tmp        : std_logic;
signal local_read_req_o_tmp         : std_logic;
signal local_init_done              : std_logic;

signal local_ready_i_d              : std_logic;

signal if1_adr_effective            : t_wishbone_address;
signal if2_adr_effective            : std_logic_vector(5 downto 0);
signal slave_o_2_int                : std_logic_vector(0 downto 0);

signal used_words                   : std_logic_vector(8 downto 0);
signal regs_ack                     : std_logic;
signal ddr_access                   : std_logic;
signal reg_access                   : std_logic;
signal msi_access                   : std_logic;
signal fifo_lw_access               : std_logic;
signal fifo_hw_access               : std_logic;
signal fifo_status_access           : std_logic;
signal rd_req                       : std_logic;
signal master_i                     : t_wishbone_master_in;  -- to Slave
signal master_o                     : t_wishbone_master_out; -- to WB
signal blockread_startadr           : t_wishbone_address;
signal xfer_cnt                     : std_logic_vector(7 downto 0); -- 2exp8=256
signal xfer_cnt_zero                : std_logic_vector(7 downto 0):= (others => '0');
signal regs_dat                     : t_wishbone_address;

signal cache_mode                   : std_logic;
signal cache_mode_d                 : std_logic;
signal cache_mode_dd                : std_logic;
signal cache_mode_pulse             : std_logic;

signal wr_access                    : std_logic;
signal rd_access                    : std_logic;
signal regs_err                     : std_logic;


signal xfer_fifo_dat_i              : std_logic_vector(63 downto 0);
signal local_rdata_i_d              : std_logic_vector(63 downto 0);
signal xfer_fifo_we                 : std_logic;
signal xfer_fifo_full               : std_logic;
signal xfer_xfer_fifo_empty         : std_logic;
signal xfer_fifo_empty              : std_logic;
signal xfer_fifo_rd                 : std_logic;
signal xfer_fifo_dat_o              : std_logic_vector(63 downto 0);
signal slave_o_2_ack                : std_logic;
signal slave_o_2_err                : std_logic;
signal slave_o_2_fifo_ack           : std_logic;
signal slave_o_2_fifo_err           : std_logic;
signal slave_o_2_fifo_dat           : std_logic_vector(31 downto 0);
signal slave_o_2_msi                : t_wishbone_slave_out;

signal lif_ack                      : std_logic;
signal lif_err                      : std_logic;
signal lif_stall                    : std_logic;

----------------------------------------------------------------------------- -------------------------------------
begin

------------------------------------------------------------------------------DDR ALTMEMPHY Controller as generated
ddr_ctlr_generated: scu_ddr3
port map (
  local_address           => local_addr_o,
  local_write_req         => local_write_req_o,
  local_read_req          => local_read_req_o,
  local_burstbegin        => local_burstbegin_o,
  local_wdata             => local_wdata_o,
  local_be                => local_be_o,
  local_size              => local_size_o,
  global_reset_n          => rstn_sys,
  pll_ref_clk             => clk_sys,
  soft_reset_n            => rstn_sys,
  local_ready             => local_ready_i,
  local_rdata             => local_rdata_i,
  local_rdata_valid       => local_rdata_valid_i,
  local_refresh_ack       => open,
  local_init_done         => local_init_done,
  reset_phy_clk_n         => local_reset_phy_clk_n,
  dll_reference_clk       => open,
  dqs_delay_ctrl_export   => open,
  --
  mem_odt                 => altmemddr_0_memory_mem_odt,
  mem_cs_n                => altmemddr_0_memory_mem_cs_n,
  mem_cke                 => altmemddr_0_memory_mem_cke,
  mem_addr                => altmemddr_0_memory_mem_addr,
  mem_ba                  => altmemddr_0_memory_mem_ba,
  mem_ras_n               => altmemddr_0_memory_mem_ras_n,
  mem_cas_n               => altmemddr_0_memory_mem_cas_n,
  mem_we_n                => altmemddr_0_memory_mem_we_n,
  mem_dm                  => altmemddr_0_memory_mem_dm,
  mem_reset_n             => altmemddr_0_memory_mem_reset_n,
  phy_clk                 => phy_clk,
  aux_full_rate_clk       => open,
  aux_half_rate_clk       => open,
  reset_request_n         => open,
  mem_clk                 => altmemddr_0_memory_mem_clk,
  mem_clk_n               => altmemddr_0_memory_mem_clk_n,
  mem_dq                  => altmemddr_0_memory_mem_dq,
  mem_dqs                 => altmemddr_0_memory_mem_dqs,
  mem_dqsn                => altmemddr_0_memory_mem_dqsn
  );--ddr_ctlr_generated;

--------------------------------------------------------------------------------------Local Interface Clock Domain

xwb_cross : xwb_clock_crossing
generic map(
  g_size => wb_fifo_size)
port map(
  -- Slave control port ( WB I/F)
  slave_clk_i    => clk_sys,
  slave_rst_n_i  => rstn_sys,
  slave_i        => slave_i_1,
  slave_o        => slave_o_1,
  -- Master reader port (Local I/F of DDR3)
  master_clk_i   => phy_clk,
  master_rst_n_i => local_reset_phy_clk_n,
  master_i       => master_i,
  master_o       => master_o
  );--xwb_cross
---------------------------------------------------------------------------------------------------burst xfer fifo
xfer_fifo : generic_async_fifo
generic map(
  g_data_width      => xfer_fifo_width,
  g_size            => xfer_fifo_size)
port map(
  rst_n_i           => local_reset_phy_clk_n,
  clk_wr_i          => phy_clk,
  d_i               => xfer_fifo_dat_i,
  we_i              => xfer_fifo_we,
  wr_empty_o        => open,
  wr_full_o         => xfer_fifo_full,
  wr_almost_empty_o => open,
  wr_almost_full_o  => open,
  wr_count_o        => open,
  clk_rd_i          => clk_sys,
  q_o               => xfer_fifo_dat_o,
  rd_i              => xfer_fifo_rd,
  rd_empty_o        => xfer_fifo_empty,
  rd_full_o         => open,
  rd_almost_empty_o => open,
  rd_almost_full_o  => open,
  rd_count_o        => used_words
  );--xfer_fifo
 --------------------------------------------------------------------------------------message signalled interrupts
fg_irq_master: wb_irq_master
  generic map (
    g_channels => 1,
    g_round_rb => true,
    g_det_edge => true)
  port map (
    clk_i   => clk_sys,
    rst_n_i => rstn_sys,
    -- msi if
    irq_master_o => irq_mst_o,
    irq_master_i => irq_mst_i,
    -- ctrl if
    ctrl_slave_o => slave_o_2_msi,
    ctrl_slave_i => slave_i_2 ,
    -- irq lines
    irq_i        => slave_o_2_int
  );
------------------------------------------------------------------------------------------------------------------

master_i.rty       <= '0';                           

slave_o_2.rty      <= '0';
slave_o_2.stall    <= '0';

slave_o_2_int(0)   <= not xfer_fifo_empty;

local_wdata_o      <= local_wdata_latch & master_o.dat;

local_write_req_o  <= local_write_req_o_tmp;
local_read_req_o   <= local_read_req_o_tmp;
local_burstbegin_o <= local_burstbegin_t;

local_be_o         <= x"ff";                        --byte enables,  x"ff" for 64 bit access (all 8 bytes enabled)
local_size_o       <= "001";                        --the default for HPC II Controller

if1_adr_effective  <= "00000" & master_o.adr (26 downto 0);
if2_adr_effective  <= slave_i_2.adr (5 downto 0);

xfer_cnt_zero      <= (others => '0');

cache_mode         <= '0' when (xfer_cnt = xfer_cnt_zero) else '1';
ddr_access         <= '1' when (to_integer(unsigned(if1_adr_effective)) >= ddr_start_adr         and to_integer(unsigned(if1_adr_effective)) <= ddr_end_adr) else '0';
reg_access         <= '1' when (to_integer(unsigned(if1_adr_effective)) =  burst_startadr_reg_adr or to_integer(unsigned(if1_adr_effective)) = xfer_cnt_adr) else '0';
msi_access         <= '1' when (to_integer(unsigned(if2_adr_effective)) >= msi_start_adr         and to_integer(unsigned(if2_adr_effective)) <= msi_end_adr) else '0';
fifo_lw_access     <= '1' when (to_integer(unsigned(if2_adr_effective)) =  fifo_lw_adr) else '0';
fifo_hw_access     <= '1' when (to_integer(unsigned(if2_adr_effective)) =  fifo_hw_adr) else '0';
fifo_status_access <= '1' when (to_integer(unsigned(if2_adr_effective)) =  fifo_status_adr)  else '0';

------------------------------------------------------------------------------------------Mux  for local interface
wr_access          <= master_o.cyc and master_o.stb and     master_o.we;
rd_access          <= master_o.cyc and master_o.stb and not master_o.we;

LIF_access : process(cache_mode,cache_mode_dd,ddr_access,rd_req,blockread_startadr,
                     master_o,local_ready_i,rd_access,wr_access,xfer_fifo_full)
begin
  if cache_mode = '1' and cache_mode_dd='1' then     --burst read from DDR3 -> no ddr access possible during cache mode
    local_burstbegin_t    <= local_ready_i and  rd_req and not xfer_fifo_full;  -- todo maybe wait on local ready before second request
    local_read_req_o_tmp  <= local_ready_i and  rd_req and not xfer_fifo_full;
    local_write_req_o_tmp <= '0';
    local_addr_o          <= blockread_startadr(23 downto 0);
  elsif (ddr_access = '1') and                        -- transparent direct DDR wr/rd only on low word accesses
    ((master_o.adr(3 downto 0) = "0000")  or (master_o.adr(3 downto 0) = "1000")) then
    local_burstbegin_t    <= local_ready_i and master_o.stb and master_o.cyc;
    local_read_req_o_tmp  <= local_ready_i and rd_access;
    local_write_req_o_tmp <= local_ready_i and wr_access;
    local_addr_o          <= master_o.adr(26 downto 3);
  else
    local_burstbegin_t    <= '0';
    local_read_req_o_tmp  <= '0';
    local_write_req_o_tmp <= '0';
    local_addr_o          <= (others => '0');
  end if;
end process LIF_access;


-----------------------------------------------------------------------------------------Mux for XWB Bridge inputs

wb_bridge_mux: process(cache_mode,cache_mode_dd,reg_access,ddr_access,master_o,regs_dat,local_rdata_i_d,lif_stall)
begin
  if cache_mode = '1' and cache_mode_dd='1' then      --ddr access during cache_mode results in stall for wb access
    master_i.dat     <= (others => '0');
    master_i.stall   <= '1';
  elsif reg_access = '1' then                                      -- write/read to regs will never result in stall
    master_i.dat     <= regs_dat;
    master_i.stall   <= '0';
  elsif ddr_access = '1' then                     -- transparent direct DDR wr/rd may result in stall for DDR ready
    if    (master_o.adr(3 downto 0) = "0000")  or (master_o.adr(3 downto 0) = "1000" ) then
      master_i.dat   <= local_rdata_i_d(31 downto 0);
      master_i.stall <= lif_stall;
    elsif (master_o.adr(3 downto 0) = "0100")  or (master_o.adr(3 downto 0) = "1100" ) then
      master_i.dat   <= local_rdata_i_d(63 downto 32);
      master_i.stall <= '0';
    else
      master_i.dat   <= (others => '0');
      master_i.stall <= '0';
    end if;
  else
     master_i.dat     <= (others => '0');
     master_i.stall   <= '0';
  end if;
end process wb_bridge_mux;
--------------------------------------------------------------------------------------------Mux for XWB Ack and Err

ack_mux: process(ddr_access,cache_mode,reg_access,lif_err,lif_ack,regs_err,regs_ack)
begin
  if ddr_access='1' and cache_mode='0' then
    master_i.err   <= lif_err;
    master_i.ack   <= lif_ack ;
  elsif reg_access ='1' then
    master_i.err   <= regs_err;
    master_i.ack   <= regs_ack;
  else
    master_i.err   <= regs_err;  --access on all other addresses gives error
    master_i.ack   <= '0';
  end if;
end process ack_mux;
-------------------------------------------------------------------------------Registered Logic for Local Interface
local_if_ack: process(phy_clk)
begin
  if (rising_edge(phy_clk)) then
    if (local_reset_phy_clk_n = '0') then
        lif_ack   <= '0';
        lif_err   <= '0';
        lif_stall <= '0';
        local_rdata_i_d   <= (others => '0');
        local_wdata_latch <= (others => '0');
    else
      if local_rdata_valid_i ='1' then
        local_rdata_i_d <= local_rdata_i;
      else
        null; --latch data from DDR
      end if;

      if wr_access ='1' then
        local_wdata_latch <= master_o.dat;
      else
        null; --latch data from cross bridge
      end if;


      if ddr_access = '1'   and cache_mode ='0' then
          --------------------------------------------------------------------------------------------write to ddr
          if (master_o.cyc='1' and (master_o.stb='1' or lif_stall='1') and master_o.we = '1' ) then
            if   (master_o.adr(3 downto 0) = "0000")  or (master_o.adr(3 downto 0) = "1000" ) then
              if  (local_ready_i='1') then
                lif_ack   <= master_o.cyc and ( master_o.stb  or (local_ready_i and not local_ready_i_d ));
                lif_err   <= '0';
                lif_stall <= '0';
              else                                                      -- write when "not ready" results in stall
                lif_ack   <= '0';
                lif_err   <= '0';
                lif_stall <= '1';
              end if;
            elsif   (master_o.adr(3 downto 0) = "0100")  or (master_o.adr(3 downto 0) = "1100" ) then
                lif_ack   <= master_o.cyc and master_o.stb;        -- for addresses 0x4 and 0xC give immediate ack
                lif_err   <= '0';
                lif_stall <= '0';
            else                                                      -- all other writes not modulo 4 causing err
                lif_ack   <= '0';
                lif_err   <= master_o.cyc and  master_o.stb;
                lif_stall <= '0';
            end if;
          ---------------------------------------------------------------------- ---------------------read from ddr
        elsif (master_o.cyc='1' and (master_o.stb='1' or lif_stall='1') and master_o.we = '0') then
            if   (master_o.adr(3 downto 0) = "0000")  or (master_o.adr(3 downto 0) = "1000" ) then
              if local_rdata_valid_i ='1' then                     -- for addresses 0x0 and 0x8 wait on rdata_valid
                lif_ack   <= local_rdata_valid_i;
                lif_err   <= '0';
                lif_stall <= '0';
              else                                                     --read in "not rdata_valid" results in stall
                lif_ack   <= '0';
                lif_err   <= '0';
                lif_stall <= '1';
              end if;
            elsif (master_o.adr(3 downto 0) = "0100")  or (master_o.adr(3 downto 0) = "1100" ) then
                lif_ack   <= master_o.cyc and  master_o.stb;        -- for addresses 0x4 and 0xC give immediate ack
                lif_err   <= '0';
                lif_stall <= '0';
            else                                                      -- all other addresses not modulo 4 cause err
                lif_ack   <= '0';
                lif_err   <= master_o.cyc and  master_o.stb;
                lif_stall <= '0';
            end if;
          else
            lif_ack     <= '0';
            lif_err     <= '0';
            lif_stall   <= '0';
          end if;
      ------------------------------------------------------------ -----write on ddr in cache_mode results in error
      elsif (ddr_access='1' and cache_mode ='1') then
            lif_ack     <= '0';
            lif_err     <= master_o.cyc and  master_o.stb;
            lif_stall   <= '0';
      else
            lif_ack     <= '0';
            lif_err     <= '0';
            lif_stall   <= '0';
      end if;
    end if;
  end if;
end process local_if_ack;

------------------------------------------------------------------- --------------------cache_mode registered logic

cache_mode_pulse <= cache_mode and not cache_mode_d;

cache_ctrl_regs : process(phy_clk)
begin
  if (rising_edge(phy_clk)) then
    if (local_reset_phy_clk_n = '0') then
      blockread_startadr     <= (others => '0');
      xfer_cnt               <= (others => '0');
      regs_dat               <= (others => '0');
      regs_ack               <= '0';
      regs_err               <= '0';
      cache_mode_d           <= '0';
      cache_mode_dd          <= '0';
      local_ready_i_d        <= '0';
    else
      cache_mode_d           <= cache_mode;
      cache_mode_dd          <= cache_mode_d;                                  --delayed for last xfer_cnt transfer
      local_ready_i_d        <= local_ready_i;

      if (reg_access = '1') then
        regs_ack <= master_o.cyc and master_o.stb;
        regs_err <= '0';
      else
        regs_ack <= '0';
        regs_err <= master_o.cyc and master_o.stb;
      end if;

      if reg_access = '1' and rd_access = '1' then
        case to_integer(unsigned(master_o.adr(26 downto 0))) is
          when burst_startadr_reg_adr =>
            regs_dat <= blockread_startadr;
          when xfer_cnt_adr           =>
            regs_dat <= zero(31 downto xfer_cnt'length) & xfer_cnt ;
          when others                 =>
            regs_dat <= (others => '0');
        end case;
      end if;

      if reg_access = '1' and wr_access = '1' then
        -- due to cache_mode=1 no chance to clear  regs after entering cache_mode
        case to_integer(unsigned(master_o.ADR(26 downto 0))) is
          when burst_startadr_reg_adr =>
            blockread_startadr <= master_o.dat;
          when xfer_cnt_adr           =>
            xfer_cnt           <= master_o.dat(7 downto 0);
          when others                 =>
            null;
        end case;
      end if;

      if cache_mode = '1' and local_rdata_valid_i = '1' then
        xfer_cnt           <= std_logic_vector(unsigned(xfer_cnt) - 1);
        blockread_startadr <= std_logic_vector(unsigned(blockread_startadr) + 1);
      end if;

      if (cache_mode_pulse='1') or (local_rdata_valid_i='1' and cache_mode='1') then
        rd_req <= '1';
       elsif  (cache_mode='1' and  local_burstbegin_t ='1') or (cache_mode='0') then
        rd_req <= '0';
      end if;
    end if;
  end if;
end process cache_ctrl_regs;

----------------------------------------------------------------------------------  ------------------xfer_fifo mux

feed_xfer_fifo: process(cache_mode, cache_mode_dd,local_rdata_i,local_rdata_valid_i)
begin
  if cache_mode = '1' or cache_mode_dd='1' then                -- signal must be delayed for last xfer_cnt transfer
    xfer_fifo_dat_i <= local_rdata_i(63 downto 0);             -- fifo gets the whole 64 bit as coming out from ddr
    xfer_fifo_we    <= local_rdata_valid_i;
  else
    xfer_fifo_dat_i <= (others => '0');
    xfer_fifo_we    <= '0';
  end if;
end process feed_xfer_fifo;

------------------------------------------------------------------------------------ -----------WB I/F 2 Output mux
slave_o_2_mux: process (fifo_lw_access,xfer_fifo_empty,xfer_fifo_dat_o,fifo_hw_access,fifo_status_access,used_words,local_init_done)
begin
  if    (fifo_lw_access = '1' )     then
    slave_o_2_fifo_dat <= xfer_fifo_dat_o(31 downto 0);
  elsif (fifo_hw_access = '1' )     then
    slave_o_2_fifo_dat <= xfer_fifo_dat_o(63 downto 32);
  elsif (fifo_status_access = '1' ) then
    slave_o_2_fifo_dat <=  xfer_fifo_empty & local_init_done & zero(29 downto used_words'length) & used_words ;
  else
    slave_o_2_fifo_dat <= (others => '0');
  end if;
end process slave_o_2_mux;


----------------------------------------------------------------------------------------xfer fifo registered logic
read_xfer_fifo: process(clk_sys)
begin
  if (rising_edge(clk_sys)) then
    if (rstn_sys = '0') then
      slave_o_2_fifo_ack <= '0';
      slave_o_2_fifo_err <= '0';
      xfer_fifo_rd  <= '0';
      slave_o_2_ack <= '0';
      slave_o_2_err <= '0';
    else
      slave_o_2_fifo_ack <= slave_o_2_ack;
      slave_o_2_fifo_err <= slave_o_2_err;
      if    slave_i_2.we = '0' and (fifo_hw_access='1' or fifo_status_access='1') then
        slave_o_2_ack <= slave_i_2.cyc and slave_i_2.stb;
        slave_o_2_err <= '0';
        xfer_fifo_rd  <= '0';
      ------------------------------------------------------------------- only lw reads are moving the fifo pointer
      elsif slave_i_2.we = '0' and (( fifo_lw_access='1' and xfer_fifo_empty='0' ))
        then
        slave_o_2_ack <= slave_i_2.cyc and slave_i_2.stb;
        slave_o_2_err <= '0';
        xfer_fifo_rd  <= slave_i_2.cyc and slave_i_2.stb;
      else
        slave_o_2_ack <= '0';
        slave_o_2_err <= slave_i_2.cyc and slave_i_2.stb;             -- error on wr and on lw_read when fifo empty
        xfer_fifo_rd  <= '0';
      end if;
    end if;
  end if;
end process read_xfer_fifo;
--------------------------------------------------------------------------------------mux between msi and xfer_fifo


mux_slave_o_2: process (msi_access,slave_o_2_msi,slave_o_2_fifo_ack,slave_o_2_fifo_err,slave_o_2_fifo_dat)
begin
  if (msi_access='1') then
    slave_o_2.ack <= slave_o_2_msi.ack;
    slave_o_2.err <= slave_o_2_msi.err;
    slave_o_2.dat <= slave_o_2_msi.dat;
  else
    slave_o_2.ack <= slave_o_2_fifo_ack;
    slave_o_2.err <= slave_o_2_fifo_err;
    slave_o_2.dat <= slave_o_2_fifo_dat;
  end if;
end process mux_slave_o_2;


end architecture rtl_ddr3_wrapper;
