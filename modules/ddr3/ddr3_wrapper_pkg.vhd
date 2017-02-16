library IEEE;
use IEEE.STD_LOGIC_1164.all;

library work;
use work.wishbone_pkg.all;


package ddr3_wrapper_pkg is

component scu_ddr3
	PORT (
		local_address	: IN STD_LOGIC_VECTOR (23 DOWNTO 0);
		local_write_req	: IN STD_LOGIC;
		local_read_req	: IN STD_LOGIC;
		local_burstbegin	: IN STD_LOGIC;
		local_wdata	: IN STD_LOGIC_VECTOR (63 DOWNTO 0);
		local_be	: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		local_size	: IN STD_LOGIC_VECTOR (2 DOWNTO 0);
		global_reset_n	: IN STD_LOGIC;
		pll_ref_clk	: IN STD_LOGIC;
		soft_reset_n	: IN STD_LOGIC;
		local_ready	: OUT STD_LOGIC;
		local_rdata	: OUT STD_LOGIC_VECTOR (63 DOWNTO 0);
		local_rdata_valid	: OUT STD_LOGIC;
		local_refresh_ack	: OUT STD_LOGIC;
		local_init_done	: OUT STD_LOGIC;
		reset_phy_clk_n	: OUT STD_LOGIC;
		dll_reference_clk	: OUT STD_LOGIC;
		dqs_delay_ctrl_export	: OUT STD_LOGIC_VECTOR (5 DOWNTO 0);
		mem_odt	: OUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		mem_cs_n	: OUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		mem_cke	: OUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		mem_addr	: OUT STD_LOGIC_VECTOR (12 DOWNTO 0);
		mem_ba	: OUT STD_LOGIC_VECTOR (2 DOWNTO 0);
		mem_ras_n	: OUT STD_LOGIC;
		mem_cas_n	: OUT STD_LOGIC;
		mem_we_n	: OUT STD_LOGIC;
		mem_dm	: OUT STD_LOGIC_VECTOR (1 DOWNTO 0);
		mem_reset_n	: OUT STD_LOGIC;
		phy_clk	: OUT STD_LOGIC;
		aux_full_rate_clk	: OUT STD_LOGIC;
		aux_half_rate_clk	: OUT STD_LOGIC;
		reset_request_n	: OUT STD_LOGIC;
		mem_clk	: INOUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		mem_clk_n	: INOUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		mem_dq	: INOUT STD_LOGIC_VECTOR (15 DOWNTO 0);
		mem_dqs	: INOUT STD_LOGIC_VECTOR (1 DOWNTO 0);
		mem_dqsn	: INOUT STD_LOGIC_VECTOR (1 DOWNTO 0)
	);
end component;


component ddr3_wrapper is
  PORT (
    clk_sys                                               : in    std_logic                     := '0';             --                             clk.clk
    rstn_sys                                              : in    std_logic                     := '0';             --                           reset.reset_n
    -- Wishbone
    slave_i_1                                             : in    t_wishbone_slave_in;                              -- to WB Slave
    slave_o_1                                             : out   t_wishbone_slave_out;                             -- to WB Master
    slave_i_2                                             : in    t_wishbone_slave_in;                              -- to WB Slave
    slave_o_2                                             : out   t_wishbone_slave_out;                             -- to WB Master
    -- control interface for msi generator
    --ctrl_irq_i                                            : in    t_wishbone_slave_in;
    --ctrl_irq_o                                            : out   t_wishbone_slave_out;
    -- master interface for msi generator
    irq_mst_i                                             : in    t_wishbone_master_in;
    irq_mst_o                                             : out   t_wishbone_master_out;
  -- DDR3 Device pins
    altmemddr_0_memory_mem_odt                            : out   std_logic_vector(0 downto 0);                     --              altmemddr_0_memory.mem_odt
    altmemddr_0_memory_mem_clk                            : inout std_logic_vector(0 downto 0)  := (others => '0'); --                                .mem_clk
    altmemddr_0_memory_mem_clk_n                          : inout std_logic_vector(0 downto 0)  := (others => '0'); --                                .mem_clk_n
    altmemddr_0_memory_mem_cs_n                           : out   std_logic_vector(0 downto 0);                     --                                .mem_cs_n
    altmemddr_0_memory_mem_cke                            : out   std_logic_vector(0 downto 0);                     --                                .mem_cke
    altmemddr_0_memory_mem_addr                           : out   std_logic_vector(12 downto 0);                    --                                .mem_addr
    altmemddr_0_memory_mem_ba                             : out   std_logic_vector(2 downto 0);                     --                                .mem_ba
    altmemddr_0_memory_mem_ras_n                          : out   std_logic;                                        --                                .mem_ras_n
    altmemddr_0_memory_mem_cas_n                          : out   std_logic;                                        --                                .mem_cas_n
    altmemddr_0_memory_mem_we_n                           : out   std_logic;                                        --                                .mem_we_n
    altmemddr_0_memory_mem_dq                             : inout std_logic_vector(15 downto 0) := (others => '0'); --                                .mem_dq
    altmemddr_0_memory_mem_dqs                            : inout std_logic_vector(1 downto 0)  := (others => '0'); --                                .mem_dqs
    altmemddr_0_memory_mem_dqsn                           : inout std_logic_vector(1 downto 0)  := (others => '0'); --                                .mem_dqsn
    altmemddr_0_memory_mem_dm                             : out   std_logic_vector(1 downto 0);                     --                                .mem_dm
    altmemddr_0_memory_mem_reset_n                        : out   std_logic;                                        --                                .mem_reset_n
    altmemddr_0_external_connection_local_refresh_ack     : out   std_logic;                                        -- altmemddr_0_external_connection.local_refresh_ack
    altmemddr_0_external_connection_local_init_done       : out   std_logic;                                        --                                .local_init_done
    altmemddr_0_external_connection_reset_phy_clk_n       : out   std_logic;                                        --                                .reset_phy_clk_n
    altmemddr_0_external_connection_dll_reference_clk     : out   std_logic;                                        --                                .dll_reference_clk
    altmemddr_0_external_connection_dqs_delay_ctrl_export : out   std_logic_vector(5 downto 0)                      --                                .dqs_delay_ctrl_export
  );
end component;


constant c_wb_DDR3_if1_sdb : t_sdb_device := (         -- defined in wishbone_pkg
    abi_class        => x"0000",                       -- undocumented device
    abi_ver_major    => x"01",
    abi_ver_minor    => x"00",
    wbd_endian       => c_sdb_endian_big,
    wbd_width        => x"7",                          -- x2=16bit port granularity (x1=8bit,x4=32bit, x7=8/16/32bit)  
    sdb_component => (
    addr_first       => x"0000000000000000",           -- this is relativ, absolute will be calculated 
    addr_last        => x"0000000007ffffff",           -- Address space for DDR and a two registers
    product => (                                       -- 
    vendor_id        => x"0000000000000651",           -- ID for GSI
    device_id        => x"20150828",                   -- ID can be chosen, but no collision with existing ones
                                                       -- Same ID to be used with find_device in C-Code for mini.sdb
    version          => x"00000001",
    date             => x"20160525",
    name             => "WB_DDR3_if1        ")));

constant c_wb_DDR3_if2_sdb : t_sdb_device := (         -- defined in wishbone_pkg
    abi_class        => x"0000",                       -- undocumented device
    abi_ver_major    => x"01",
    abi_ver_minor    => x"00",
    wbd_endian       => c_sdb_endian_big,
    wbd_width        => x"7",                          -- x2=16bit port granularity (x1=8bit,x4=32bit, x7=8/16/32bit)  
    sdb_component => ( 
    addr_first       => x"0000000000000000",           -- this is relativ, absolute will be calculated
    addr_last        => x"00000000000000ff",           -- Address space 2exp 25 bytes = 2 exp 24  Words
    product => (                                       -- 0  07ff ffff= 0.125 GigaByte= 1073741824 bit for SCU 1Gb DDR3
    vendor_id        => x"0000000000000651",           -- ID for GSI
    device_id        => x"20160525",                   -- ID can be chosen, but no collision with existing ones
                                                       -- Same ID to be used with find_device in C-Code for mini.sdb
    version          => x"00000001",
    date             => x"20160526",
    name             => "WB_DDR3_if2        ")));


end package;--ddr3_wrapper_pkg
