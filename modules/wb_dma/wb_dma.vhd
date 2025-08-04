library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.wb_dma_slave_auto_pkg.all;
use work.gencores_pkg.all;
use work.wb_dma_pkg.all;
use work.wb_dma_RAM_pkg.all;

entity wb_dma is
  generic(
    g_host_ram_size  : Integer := 16;
    g_dma_transfer_block_size : Integer
  );
  port(
    clk_sys_i     : in std_logic;
    rstn_sys_i    : in std_logic;

    slave_i   : in t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;
    ram_slave_i : in t_wishbone_slave_in;
    ram_slave_o : out t_wishbone_slave_out;
    master1_i  : in t_wishbone_master_in;
    master1_o  : out t_wishbone_master_out;
    master2_i  : in t_wishbone_master_in;
    master2_o  : out t_wishbone_master_out
    );
end entity;

architecture rtl of wb_dma is

  -- HOST RAM SIGNALS
  ------------------------------------------
  signal s_host_ram_wea       : std_logic;
  signal s_host_ram_address_a : std_logic_vector(c_wishbone_address_width-1 downto 0);
  signal s_host_ram_data_a    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_out_a     : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_web       : std_logic;
  signal s_host_ram_address_b : std_logic_vector(c_wishbone_address_width-1 downto 0);
  signal s_host_ram_data_b    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_out_b     : std_logic_vector(c_wishbone_data_width-1 downto 0);

  -- STATUS/CONTROL SIGNALS
  ------------------------------------------
  signal s_dma_active         : std_logic;
  signal s_descriptor_active  : std_logic;
  signal s_rd_master_idle        : std_logic;
  signal s_start_transfer     : std_logic_vector(32-1 downto 0);
  signal r_start_transfer_once: std_logic;

  signal s_buffer_empty : std_logic;
  signal s_buffer_out   : t_wishbone_data;

  signal s_wr_buffer_ready : std_logic;
  signal s_buffer_we : std_logic;
  signal s_buffer_rd : std_logic;

  signal s_desc_write_select : std_logic_vector(1 downto 0);
  signal s_desc_write_enable : std_logic;

  signal s_descriptor_csr        : std_logic_vector(31 downto 0); -- unroll into the relevant data (i.e. transfer size)
  signal s_source_address        : t_wishbone_address;
  signal s_destination_address   : t_wishbone_address;
  signal s_next_descriptor       : std_logic_vector(31 downto 0);

  -- CONFIG SIGNALS
  ------------------------------------------
  signal s_read_address       : t_wishbone_address;
  signal s_write_address      : t_wishbone_address;
  signal s_transfer_size      : t_wishbone_data;
  signal s_init_pointer_addr  : std_logic_vector(31 downto 0) := (others => '0');

  -- WISHBONE SIGNALS
  ------------------------------------------
  signal s_rd_master_o : t_wishbone_master_out;
  signal s_wr_master_o : t_wishbone_master_out;

  component wb_dma_RAM is
    generic (
      g_size    : natural
    );

    port(
      clk_sys_i : in std_logic;
      rst_n_i   : in std_logic;

      wb_slave_i : in  t_wishbone_slave_in;
      wb_slave_o : out t_wishbone_slave_out;

      proc_slave_i : in  t_wishbone_slave_in;
      proc_slave_o : out t_wishbone_slave_out
    );
  end component;

  component wb_dma_engine is
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;

      desc_write_select_o : out std_logic_vector(1 downto 0);
      desc_write_enable_o : out std_logic;

      init_pointer_addr_i : in std_logic_vector(31 downto 0);
      next_pointer_i      : in std_logic_vector(31 downto 0);
      pointer_addr_o      : out std_logic_vector(31 downto 0);

      -- communication signals
      dma_active_i        : in std_logic; -- enabled
      dma_idle_i          : in std_logic; -- actively transferring data
      descriptor_active_i : in std_logic;
      init_transfer_i     : std_logic
    );
  end component;

  component wb_dma_channel is
    generic(
      g_max_transfer_size : integer
    );
    port(
      clk_i                   : in std_logic;
      rstn_i                  : in std_logic;

      data_i                  : in std_logic_vector(31 downto 0);
      
      write_select_i          : in std_logic_vector(1 downto 0);
      write_enable_i          : in std_logic;

      descriptor_csr_o        : out std_logic_vector(31 downto 0); -- unroll into the relevant data (i.e. transfer size)
      transfer_size_o         : out std_logic_vector(log2_floor(g_max_transfer_size) downto 0);
      source_address_o        : out t_wishbone_address;
      destination_address_o   : out t_wishbone_address;
      next_descriptor_o       : out std_logic_vector(31 downto 0);

      descriptor_active_o     : out std_logic
    );
  end component;

  component wb_dma_wb_read_master is
    generic(
      g_block_size : integer
    );
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;

      -- config signals
      transfer_size_i       : in std_logic_vector(log2_floor(g_block_size) downto 0);
      start_address_i       : in t_wishbone_address;
      descriptor_address_i  : in t_wishbone_address;

      -- communication signals
      dma_active_i        : in std_logic;
      descriptor_active_i : in std_logic;
      rd_buffer_ready_i   : in std_logic;
      buffer_we_o         : out std_logic;
      read_descriptor_i   : in std_logic;

      master_idle_o : out std_logic;

      master_i  : in t_wishbone_master_in;
      master_o  : out t_wishbone_master_out
  );
  end component;

  component wb_dma_wb_write_master is
    generic(
        g_block_size : integer
    );
    port(
        clk_i : in std_logic;
        rstn_i : in std_logic;
    
        -- config signals
        transfer_size_i : in std_logic_vector(log2_floor(g_dma_transfer_block_size) downto 0);
        start_address_i : in t_wishbone_address;
    
        -- communication signals
        dma_active_i : in std_logic;
        descriptor_active_i : in std_logic;
        wr_buffer_ready_i : in std_logic;
        buffer_rd_o       : out std_logic;
    
        master_idle_o : out std_logic;
    
        master_i  : in t_wishbone_master_in;
        master_o  : out t_wishbone_master_out;

        buffer_i  : in t_wishbone_data
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
    
        buffer_we_i : in std_logic;
        buffer_rd_i : in std_logic;
        
        rd_master_i     : in t_wishbone_master_in;
        rd_master_snoop : in t_wishbone_master_out;
    
        wr_master_i     : in t_wishbone_master_in;
        wr_master_snoop : in t_wishbone_master_out;

        buffer_output   : out t_wishbone_data
    );
  end component;

begin

    wb_RAM : wb_dma_RAM
    generic map(
      g_size => g_host_ram_size
    )

    port map(
      clk_sys_i   => clk_sys_i,
      rst_n_i     => rstn_sys_i,

      wb_slave_i  => ram_slave_i,
      wb_slave_o  => ram_slave_o,

      proc_slave_i => c_DUMMY_WB_SLAVE_IN,
      proc_slave_o => open
    );

    host_ram: generic_dpram
    generic map (
      g_data_width               => c_wishbone_data_width,
      g_size                     => g_host_ram_size,
      g_with_byte_enable         => false,
      g_dual_clock               => false,
      g_init_file                => " ../../../modules/wb_dma/descriptor.mif"
    )
    port map (
      rst_n_i => rstn_sys_i,      -- synchronous reset, active LO

      -- Port A
      clka_i => clk_sys_i,
      bwea_i => (others => '0'),
      wea_i  => s_host_ram_wea,
      aa_i   => s_host_ram_address_a(f_log2_size(g_host_ram_size)-1 downto 0),
      da_i   => s_host_ram_data_a,
      qa_o   => s_host_ram_out_a,
      
      -- Port B
      clkb_i => clk_sys_i,
      bweb_i => (others => '0'),
      web_i  => s_host_ram_web,
      ab_i   => s_host_ram_address_b(f_log2_size(g_host_ram_size)-1 downto 0),
      db_i   => s_host_ram_data_b,
      qb_o   => s_host_ram_out_b
    );

    dma_engine : wb_dma_engine
      port map(
        clk_i => clk_sys_i,
        rstn_i => rstn_sys_i,

        desc_write_select_o => s_desc_write_select,
        desc_write_enable_o => s_desc_write_enable, 

        init_pointer_addr_i => s_init_pointer_addr,
        next_pointer_i      => s_next_descriptor,
        pointer_addr_o      => s_host_ram_address_b,

        -- communication signals
        dma_active_i        => s_dma_active, -- enabled
        dma_idle_i          => s_dma_active, -- actively transferring data
        descriptor_active_i => s_descriptor_active,
        init_transfer_i     => s_dma_active
      );

    dma_singular_channel : wb_dma_channel
      generic map(
        g_max_transfer_size => g_dma_transfer_block_size
      )
      port map(
        clk_i                   => clk_sys_i,
        rstn_i                  => rstn_sys_i,

        data_i                  => s_host_ram_out_b,
        
        write_select_i          => s_desc_write_select,
        write_enable_i          => s_desc_write_enable,

        descriptor_csr_o        => s_descriptor_csr, -- unroll into the relevant data (i.e. transfer size)
        transfer_size_o         => s_transfer_size(4 downto 0),
        source_address_o        => s_source_address,
        destination_address_o   => s_destination_address,
        next_descriptor_o       => s_next_descriptor,

        descriptor_active_o     => open
      );

    read_master :  wb_dma_wb_read_master
      generic map(
          g_block_size => g_dma_transfer_block_size
      )
      port map(
          clk_i => clk_sys_i,
          rstn_i => rstn_sys_i,
      
          -- config signals
          transfer_size_i => s_transfer_size(4 downto 0),--std_logic_vector(to_unsigned(g_dma_transfer_block_size, log2_floor(g_block_size))),
          start_address_i => s_source_address,
          descriptor_address_i => (others => '0'),
      
          -- communication signals
          dma_active_i => s_dma_active,
          descriptor_active_i => s_descriptor_active,
          rd_buffer_ready_i   => s_buffer_empty,
          buffer_we_o         => s_buffer_we,
          read_descriptor_i   => '0',

          master_idle_o => s_rd_master_idle,
      
          master_i  => master1_i,
          master_o  => s_rd_master_o
      );

    s_wr_buffer_ready <= not s_buffer_empty and s_rd_master_idle; -- only for testing. Normally this signal gets generated by the buffer after switching the two FIFOs

    write_master : wb_dma_wb_write_master
      generic map(
          g_block_size => g_dma_transfer_block_size
      )
      port map(
          clk_i   => clk_sys_i,
          rstn_i  => rstn_sys_i,
      
          -- config signals
          transfer_size_i => s_transfer_size(4 downto 0),--std_logic_vector(to_unsigned(g_dma_transfer_block_size, log2_floor(g_dma_transfer_block_size))),
          start_address_i => s_destination_address,
      
          -- communication signals
          dma_active_i => s_dma_active,
          descriptor_active_i => s_descriptor_active,
          wr_buffer_ready_i => s_wr_buffer_ready,
          buffer_rd_o       => s_buffer_rd,
      
          master_idle_o => open,
      
          master_i  => master2_i,
          master_o  => s_wr_master_o,

          buffer_i => s_buffer_out
      );

    master1_o <= s_rd_master_o;
    master2_o <= s_wr_master_o;

    data_buffer : wb_dma_data_buffer
      generic map(
        g_block_size => g_dma_transfer_block_size
      )
      port map(
        clk_i   => clk_sys_i,
        rstn_i  => rstn_sys_i,
    
        buffer_empty_o  => s_buffer_empty,
        buffer_full_o   => open,

        buffer_we_i => s_buffer_we,
        buffer_rd_i => s_buffer_rd,
        
        rd_master_i     => master1_i,
        rd_master_snoop => s_rd_master_o,
    
        wr_master_i     => master2_i, 
        wr_master_snoop => s_wr_master_o,

        buffer_output   => s_buffer_out
      );

    wishbone_slave : wb_dma_slave_auto
    generic map (
      g_channels => 16 --Number of DMA channels
    )
    port map (
      clk_sys_i               => clk_sys_i,  -- Clock input for sys domain
      rst_sys_n_i             => rstn_sys_i, -- Reset input (active low) for sys domain
      error_i                 => (others => '0'),        -- Error control
      stall_i                 => (others => '0'),        -- flow control
      dma_csr_o               => open,      -- DMA controller control and status register
      read_address_o          => s_init_pointer_addr, -- DMA start address, for testing only
      start_transfer_o        => s_start_transfer, -- start transfer, for testing only
      write_address_o         => s_write_address,  -- DMA write address, for testing only
      
      data_i                => slave_i,
      data_o                => slave_o
    );

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
