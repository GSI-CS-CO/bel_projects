library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package remote_update_pkg is

component remote_update
	port 	(
		--asmi_busy:        in std_logic  := '0';
		--asmi_data_valid:  in std_logic  := '0';
		--asmi_dataout:     in std_logic_vector(7 downto 0) :=  (others => '0');
		clock:            in std_logic;
		data_in:          in std_logic_vector(23 downto 0);
		param:            in std_logic_vector(2 downto 0);
		read_param:       in std_logic;
		reconfig:         in std_logic;
		reset:            in std_logic;
		reset_timer:      in std_logic;
		write_param:      in std_logic;
		--asmi_addr:        out std_logic_vector(23 downto 0);
		--asmi_rden:        out std_logic;
		--asmi_read:        out std_logic;
		busy:             out std_logic;
		data_out:         out std_logic_vector(23 downto 0)
		--pof_error:        out std_logic 
	);
end component;

component altasmi
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
end component;

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
  generic ( PAGESIZE : integer );
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
   addr_last     => x"000000000fffffff",
   product => (
   vendor_id     => x"0000000000000651",
   device_id     => x"48526423",
   version       => x"00000001",
   date          => x"20150824",
   name          => "wb asmi parallel   ")));

end package remote_update_pkg;
