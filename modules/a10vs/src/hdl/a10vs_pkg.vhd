library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
--use work.monster_pkg.all;

package a10vs_pkg is

    constant c_a10vs_sdb: t_sdb_device := (
        abi_class         => x"0000", -- undocumented device
        abi_ver_major     => x"00",
        abi_ver_minor     => x"00",
        wbd_endian        => c_sdb_endian_big,
        wbd_width         => x"7",             -- 8/16/32-bit port granularity
        sdb_component     => (
            addr_first    => x"0000000000000000",
            addr_last     => x"000000000000000f",
            product       => (
                vendor_id => x"0000000000000651",
                device_id => x"a1076000",
                version   => x"00000001",
                date      => x"20250214",
                name      => "ARRIA10_VS         "
            )
        )
    );

    component a10vs is
        generic (
            g_data_size : natural := 32
        );
        port (
            -- wishbone syscon
            clk_i   : in  std_logic := '0';
            rst_n_i : in  std_logic := '1';

            -- wishbone slave interface
            slave_i : in  t_wishbone_slave_in;
            slave_o : out t_wishbone_slave_out;

            -- voltage sensor (IP core) interface (Avalon-MM)
            vs_clk               : out std_logic;                     -- clk
            vs_rst               : out std_logic;                     -- reset
            vs_ctrl_csr_addr     : out std_logic;                     -- address
            vs_ctrl_csr_rd       : out std_logic;                     -- read
            vs_ctrl_csr_wr       : out std_logic;                     -- write
            vs_ctrl_csr_wrdata   : out std_logic_vector(31 downto 0); -- writedata
            vs_ctrl_csr_rddata   : in  std_logic_vector(31 downto 0); -- readdata
            vs_sample_csr_addr   : out std_logic_vector(3 downto 0);  -- address
            vs_sample_csr_rd     : out std_logic;                     -- read
            vs_sample_csr_wr     : out std_logic;                     -- write
            vs_sample_csr_wrdata : out std_logic_vector(31 downto 0); -- writedata
            vs_sample_csr_rddata : in  std_logic_vector(31 downto 0); -- readdata
            vs_sample_irq        : in  std_logic                      -- irq
        );
    end component;

    constant c_vs_reg_n        : integer := 11;                     -- number of the IP core registers
    constant c_vs_reg_size     : integer := 16;                     -- bits, size of the IP core registers

end package a10vs_pkg;