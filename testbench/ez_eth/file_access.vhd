package file_access is

  procedure file_access_init(stop_until_1st_packet: boolean);
  attribute foreign of file_access_init : procedure is "VHPIDIRECT file_access_init";

  -- if the function returns a positive integer, it is a valid value
  -- if the function returns a negative value it  is either

  function  file_access_pending return integer;
  attribute foreign of file_access_pending : function is "VHPIDIRECT file_access_pending";

  function  file_access_fetch_packet return integer;
  attribute foreign of file_access_fetch_packet : function is "VHPIDIRECT file_access_fetch_packet";

  function file_access_read return integer;
  attribute foreign of file_access_read : function is "VHPIDIRECT file_access_read";

  function file_access_write(x : integer) return integer;
  attribute foreign of file_access_write : function is "VHPIDIRECT file_access_write";

  procedure file_access_flush;
  attribute foreign of file_access_flush : procedure is "VHPIDIRECT file_access_flush";


  shared variable my_var : integer := 43;
  shared variable PACKET_EMPTY : integer := -1;
  shared variable FIFO_EMPTY : integer := -2;
end package;

package body file_access is

  procedure file_access_init(stop_until_1st_packet : boolean) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  function file_access_pending return integer is
  begin
    assert false report "VHPI" severity failure;
    return 0;
  end function;

  function file_access_fetch_packet return integer is
  begin
    assert false report "VHPI" severity failure;
    return 0;
  end function;

  function file_access_read return integer is
  begin
    assert false report "VHPI" severity failure;
    return 0;
  end function;

  function file_access_write(x : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
    return 0;
  end function;

  procedure file_access_flush is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

end package body;
