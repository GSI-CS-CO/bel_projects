library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.wb_dma_slave_auto_pkg.all;
use work.gencores_pkg.all;

entity wb_dma is
  generic(
    g_host_ram_size  : Integer := 16;
    g_dma_transfer_block_size : Integer := 4
  );
  port(
    clk_sys_i     : in std_logic;
    rstn_sys_i    : in std_logic;

    slave_i   : in t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;
    master_i  : in t_wishbone_master_in;
    master_o  : out t_wishbone_master_out
    );
end entity;

architecture rtl of wb_dma is

  -- -- HOST RAM SIGNALS
  -- ------------------------------------------
  -- signal s_host_ram_wea       : std_logic;
  -- signal s_host_ram_address_a : std_logic_vector(c_wishbone_address_width-1 downto 0);
  -- signal s_host_ram_data_a    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  -- signal s_host_ram_out_a     : std_logic_vector(c_wishbone_data_width-1 downto 0);
  -- signal s_host_ram_web       : std_logic;
  -- signal s_host_ram_address_b : std_logic_vector(c_wishbone_address_width-1 downto 0);
  -- signal s_host_ram_data_b    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  -- signal s_host_ram_out_b     : std_logic_vector(c_wishbone_data_width-1 downto 0);

  -- STATUS/CONTROL SIGNALS
  ------------------------------------------
  signal s_dma_active         : std_logic;
  signal s_descriptor_active  : std_logic;
  signal s_master_idle        : std_logic;
  signal s_start_transfer     : std_logic_vector(32-1 downto 0);
  signal r_start_transfer_once: std_logic;

  signal s_buffer_empty : std_logic;

  -- CONFIG SIGNALS
  ------------------------------------------
  signal s_start_address : t_wishbone_address;
  signal s_transfer_size : t_wishbone_data;

  -- WISHBONE SIGNALS
  ------------------------------------------
  signal s_master_o : t_wishbone_master_out;

  component wb_dma_wb_read_master is
    generic(
      g_block_size : integer
    );
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;
  
      -- config signals
      transfer_size_i : in std_logic_vector(log2_ceil(g_dma_transfer_block_size) downto 0);
      start_address_i : in t_wishbone_address;
  
      -- communication signals
      dma_active_i : in std_logic;
      descriptor_active_i : in std_logic;
      rd_buffer_ready_i   : in std_logic;
  
      master_idle_o : out std_logic;
  
      master_i  : in t_wishbone_master_in;
      master_o  : out t_wishbone_master_out
  );
  end component;

  component wb_dma_data_buffer is
    generic(
        g_block_size : integer
    );
    
    port(
        clk_i   : in std_logic;
        rstn_i  : in std_logic;
    
        buffer_empty_o  : out std_logic;
        buffer_full_o   : out std_logic;
        
        rd_master_i     : in t_wishbone_master_in;
        rd_master_snoop : in t_wishbone_slave_in;
    
        wr_master_i     : in t_wishbone_master_in;
        wr_master_snoop : in t_wishbone_slave_in;

        buffer_output   : out t_wishbone_data
    );
    end component;

begin

    -- host_ram: generic_dpram
    -- generic map (
    --   g_data_width               => c_wishbone_data_width,
    --   g_size                     => g_host_ram_size,
    --   g_with_byte_enable         => false,
    --   g_dual_clock               => false
    -- )
    -- port map (
    --   rst_n_i => rstn_sys_i,      -- synchronous reset, active LO

    --   -- Port A
    --   clka_i => clk_sys_i,
    --   bwea_i => (others => '0'),
    --   wea_i  => s_host_ram_wea,
    --   aa_i   => s_host_ram_address_a(f_log2_size(g_host_ram_size)-1 downto 0),
    --   da_i   => s_host_ram_data_a,
    --   qa_o   => s_host_ram_out_a,
      
    --   -- Port B
    --   clkb_i => clk_sys_i,
    --   bweb_i => (others => '0'),
    --   web_i  => s_host_ram_web,
    --   ab_i   => s_host_ram_address_b(f_log2_size(g_host_ram_size)-1 downto 0),
    --   db_i   => s_host_ram_data_b,
    --   qb_o   => s_host_ram_out_b
    -- );

    read_master :  wb_dma_wb_read_master
      generic map(
          g_block_size => g_dma_transfer_block_size
      )
      port map(
          clk_i => clk_sys_i,
          rstn_i => rstn_sys_i,
      
          -- config signals
          transfer_size_i => s_transfer_size(log2_ceil(g_dma_transfer_block_size) downto 0),--std_logic_vector(to_unsigned(g_dma_transfer_block_size, log2_ceil(g_dma_transfer_block_size))),
          start_address_i => s_start_address,
      
          -- communication signals
          dma_active_i => s_dma_active,
          descriptor_active_i => s_descriptor_active,
          rd_buffer_ready_i   => s_buffer_empty,
      
          master_idle_o => s_master_idle,
      
          master_i  => master_i,
          master_o  => s_master_o
      );

    master_o <= s_master_o;

    data_buffer : wb_dma_data_buffer
      generic map(
        g_block_size => g_dma_transfer_block_size
      )
      port map(
        clk_i   => clk_sys_i,
        rstn_i  => rstn_sys_i,
    
        buffer_empty_o  => s_buffer_empty,
        buffer_full_o   => open,
        
        rd_master_i     => master_i,
        rd_master_snoop => s_master_o,
    
        wr_master_i     => c_DUMMY_WB_MASTER_IN, 
        wr_master_snoop => c_DUMMY_WB_MASTER_OUT,

        buffer_output   => open
      );
    
    -- data_buffer : wb_dma_data_buffer
    -- generic map(
    --   g_block_size <= g_dma_transfer_block_size
    -- )
    -- port map(
    --   clk_i   <= clk_sys_i,
    --   rstn_i  <= rstn_sys_i,
  
    --   read_block_done_i   <= 
    --   write_block_done_i  <=
  
    --   buffer_empty_o  <=
    --   buffer_full_o   <=
      
    --   master_i  <= master_i,
    --   master_o  <= master_o
    -- );

    wishbone_slave : wb_dma_slave_auto
    generic map (
      g_channels => 16 --Number of DMA channels
    )
    port map (
      clk_sys_i               => clk_sys_i,  -- Clock input for sys domain
      rst_sys_n_i             => rstn_sys_i, -- Reset input (active low) for sys domain
      error_i                 => (others => '0'),        -- Error control
      stall_i                 => (others => '0'),        -- flow control
      dma_csr_o               => s_transfer_size,      -- DMA controller control and status register
      start_address_o         => s_start_address, -- DMA start address, for testing only
      start_transfer_o        => s_start_transfer, -- start transfer, for testing only
      
      data_i                => slave_i,
      data_o                => slave_o
    );

    -- <for testing
    -- s_dma_active <= s_start_transfer(0);
    -- s_descriptor_active <= s_start_transfer(0);

    -- for testing. Normally the DMA Engine should be able turn off the transfer signal when there are no new descriptors.
    p_start_once : process(rstn_sys_i, s_dma_active, r_start_transfer_once, clk_sys_i)
    begin
      if(rstn_sys_i = '0') then
        r_start_transfer_once <= '0';
        s_dma_active <= '0';
        s_descriptor_active <= '0';
      else
        if rising_edge(clk_sys_i) then
          r_start_transfer_once <= s_start_transfer(0);
          if(r_start_transfer_once = '0' and s_start_transfer(0) = '1') then
            s_dma_active <= '1';
            s_descriptor_active <= '1';
          else
            s_dma_active <= '0';
            s_descriptor_active <= '0';
          end if;
        end if;
      end if;
    end process;
    -- for testing>

end architecture;
