library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package remote_update_pkg is

component remote_update
	port 	(
		clock:            in std_logic;
		data_in:          in std_logic_vector(23 downto 0);
		param:            in std_logic_vector(2 downto 0);
		read_param:       in std_logic;
		reconfig:         in std_logic;
		reset:            in std_logic;
		reset_timer:      in std_logic;
		write_param:      in std_logic;
		busy:             out std_logic;
		data_out:         out std_logic_vector(23 downto 0)
	);
end component;
component altasmi IS
	PORT
	(
		addr		: IN STD_LOGIC_VECTOR (23 DOWNTO 0);
		asmi_dataout		: IN STD_LOGIC_VECTOR (0 DOWNTO 0);
		clkin		: IN STD_LOGIC ;
		datain		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		fast_read		: IN STD_LOGIC ;
		rden		: IN STD_LOGIC ;
		read_rdid		: IN STD_LOGIC ;
		read_status		: IN STD_LOGIC ;
		reset		: IN STD_LOGIC ;
		sector_erase		: IN STD_LOGIC ;
		shift_bytes		: IN STD_LOGIC ;
		write		: IN STD_LOGIC ;
		asmi_dataoe		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		asmi_dclk		: OUT STD_LOGIC ;
		asmi_scein		: OUT STD_LOGIC ;
		asmi_sdoin		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0);
		busy		: OUT STD_LOGIC ;
		data_valid		: OUT STD_LOGIC ;
		dataout		: OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		illegal_erase		: OUT STD_LOGIC ;
		illegal_write		: OUT STD_LOGIC ;
		rdid_out		: OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		read_address		: OUT STD_LOGIC_VECTOR (23 DOWNTO 0);
		status_out		: OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
	);
      END component altasmi;

component asmi5 is
        port (
                clkin         : in  std_logic                     := 'X';             -- clk
                rden          : in  std_logic                     := 'X';             -- rden
                addr          : in  std_logic_vector(31 downto 0) := (others => 'X'); -- addr
                reset         : in  std_logic                     := 'X';             -- reset
                dataout       : out std_logic_vector(7 downto 0);                     -- dataout
                busy          : out std_logic;                                        -- busy
                data_valid    : out std_logic;                                        -- data_valid
                wren          : in  std_logic                     := 'X';             -- wren
                en4b_addr     : in  std_logic                     := 'X';             -- en4b_addr
                read_rdid     : in  std_logic                     := 'X';             -- read_rdid
                rdid_out      : out std_logic_vector(7 downto 0);                     -- rdid_out
                read_status   : in  std_logic                     := 'X';             -- read_status
                status_out    : out std_logic_vector(7 downto 0);                     -- status_out
                read_address  : out std_logic_vector(31 downto 0);                    -- read_address
                fast_read     : in  std_logic                     := 'X';             -- fast_read
                read_dummyclk : in  std_logic                     := 'X';             -- read dummyclock
                write         : in  std_logic                     := 'X';             -- write
                datain        : in  std_logic_vector(7 downto 0)  := (others => 'X'); -- datain
                illegal_write : out std_logic;                                        -- illegal_write
                shift_bytes   : in  std_logic                     := 'X';             -- shift_bytes
                sector_erase  : in  std_logic                     := 'X';             -- sector_erase
                bulk_erase    : in  std_logic                     := 'X';             -- bulk_erase
                illegal_erase : out std_logic;                                        -- illegal_erase
                ex4b_addr     : in  std_logic                     := 'X'              -- ex4b_addr
        );
end component asmi5;
component asmi10 is
	port (
		addr          : in  std_logic_vector(31 downto 0) := (others => '0'); --          addr.addr
		busy          : out std_logic;                                        --          busy.busy
		clkin         : in  std_logic                     := '0';             --         clkin.clk
		data_valid    : out std_logic;                                        --    data_valid.data_valid
		datain        : in  std_logic_vector(7 downto 0)  := (others => '0'); --        datain.datain
		dataout       : out std_logic_vector(7 downto 0);                     --       dataout.dataout
		en4b_addr     : in  std_logic                     := '0';             --     en4b_addr.en4b_addr
		ex4b_addr     : in  std_logic                     := '0';             --     en4b_addr.en4b_addr
		fast_read     : in  std_logic                     := '0';             --     fast_read.fast_read
                read_dummyclk : in  std_logic                     := '0';             -- read dummyclock
		illegal_erase : out std_logic;                                        -- illegal_erase.illegal_erase
		illegal_write : out std_logic;                                        -- illegal_write.illegal_write
		rden          : in  std_logic                     := '0';             --          rden.rden
		rdid_out      : out std_logic_vector(7 downto 0);                     --      rdid_out.rdid_out
		read_address  : out std_logic_vector(31 downto 0);                    --  read_address.read_address
		read_rdid     : in  std_logic                     := '0';             --     read_rdid.read_rdid
		read_status   : in  std_logic                     := '0';             --   read_status.read_status
		reset         : in  std_logic                     := '0';             --         reset.reset
		sce           : in  std_logic_vector(2 downto 0)  := (others => '0'); --           sce.sce
		sector_erase  : in  std_logic                     := '0';             --  sector_erase.sector_erase
		shift_bytes   : in  std_logic                     := '0';             --   shift_bytes.shift_bytes
		status_out    : out std_logic_vector(7 downto 0);                     --    status_out.status_out
		wren          : in  std_logic                     := '0';             --          wren.wren
		write         : in  std_logic                     := '0'              --         write.write
	);
end component asmi10;
component asmi_arriaII is
            port (
              clkin         : in  std_logic                     := 'X';             -- clk
              rden          : in  std_logic                     := 'X';             -- rden
              addr          : in  std_logic_vector(23 downto 0) := (others => 'X'); -- addr
              reset         : in  std_logic                     := 'X';             -- reset
              dataout       : out std_logic_vector(7 downto 0);                     -- dataout
              busy          : out std_logic;                                        -- busy
              data_valid    : out std_logic;                                        -- data_valid
              wren          : in  std_logic                     := 'X';             -- wren
              read_rdid     : in  std_logic                     := 'X';             -- read_rdid
              rdid_out      : out std_logic_vector(7 downto 0);                     -- rdid_out
              read_status   : in  std_logic                     := 'X';             -- read_status
              status_out    : out std_logic_vector(7 downto 0);                     -- status_out
              read_address  : out std_logic_vector(23 downto 0);                    -- read_address
              fast_read     : in  std_logic                     := 'X';             -- fast_read
              write         : in  std_logic                     := 'X';             -- write
              datain        : in  std_logic_vector(7 downto 0)  := (others => 'X'); -- datain
              illegal_write : out std_logic;                                        -- illegal_write
              shift_bytes   : in  std_logic                     := 'X';             -- shift_bytes
              sector_erase  : in  std_logic                     := 'X';             -- sector_erase
              illegal_erase : out std_logic                                         -- illegal_erase
            );
end component asmi_arriaII;

component wb_remote_update is
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;

    -- asmi interface, needed for pof check
    asmi_busy       : in std_logic;
    asmi_data_valid : in std_logic;
    asmi_dataout    : in std_logic_vector(7 downto 0);
    asmi_addr       : out std_logic_vector(23 downto 0);
    asmi_rden       : out std_logic;
    asmi_read       : out std_logic;
    asmi_to_aru     : out std_logic

  );
end component;

component wb_asmi is
  generic (
    pagesize : integer;
    g_family : string := "none"
  );
  port (
    clk_flash_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out
  );
end component;

component wb_asmi_slave is
  generic ( PAGESIZE : INTEGER );
  port (
    clk_flash_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;

    -- asmi interface, needed for pof check
    asmi_busy       : out std_logic;
    asmi_data_valid : out std_logic;
    asmi_dataout    : out std_logic_vector(7 downto 0);
    asmi_addr_ext   : in std_logic_vector(23 downto 0);
    asmi_rden_ext   : in std_logic;
    asmi_read_ext   : in std_logic;
    -- needed for multiplexing
    asmi_to_ext     : in std_logic


  );
end component;

constant c_wb_rem_upd_sdb : t_sdb_device := (
   abi_class     => x"0000", -- undocumented device
   abi_ver_major => x"01",
   abi_ver_minor => x"00",
   wbd_endian    => c_sdb_endian_big,
   wbd_width     => x"4", -- 8/16/32-bit port granularity
   sdb_component => (
   addr_first    => x"0000000000000000",
   addr_last     => x"000000000000003f",
   product => (
   vendor_id     => x"0000000000000651",
   device_id     => x"38956271",
   version       => x"00000001",
   date          => x"20150812",
   name          => "wb remote update   ")));

constant c_wb_asmi_sdb : t_sdb_device := (
   abi_class     => x"0000", -- undocumented device
   abi_ver_major => x"01",
   abi_ver_minor => x"00",
   wbd_endian    => c_sdb_endian_big,
   wbd_width     => x"7", -- 8/16/32-bit port granularity
   sdb_component => (
   addr_first    => x"0000000000000000",
   addr_last     => x"00000000000000ff",
   product => (
   vendor_id     => x"0000000000000651",
   device_id     => x"48526424",
   version       => x"00000001",
   date          => x"20200130",
   name          => "wb asmi parallel   ")));

constant c_wb_asmi_slave_sdb : t_sdb_device := (
   abi_class     => x"0000", -- undocumented device
   abi_ver_major => x"01",
   abi_ver_minor => x"00",
   wbd_endian    => c_sdb_endian_big,
   wbd_width     => x"7", -- 8/16/32-bit port granularity
   sdb_component => (
   addr_first    => x"0000000000000000",
   addr_last     => x"000000000fffffff",
   product => (
   vendor_id     => x"0000000000000651",
   device_id     => x"48526423",
   version       => x"00000002",
   date          => x"20150824",
   name          => "wb asmi slave par  ")));

end package remote_update_pkg;
