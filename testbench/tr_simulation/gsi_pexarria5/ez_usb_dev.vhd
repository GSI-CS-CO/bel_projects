package ez_usb_dev is

  procedure ez_usb_dev_init(stop_unitl_connected : boolean);
  attribute foreign of ez_usb_dev_init : procedure is "VHPIDIRECT ez_usb_dev_init";

  -- if the function returns a positive integer, it is a valid value
  -- if the function returns a negative value it  is either
  --     TIMEOUT, meaning that nothing was read
  -- or  HANGUP, meaning that the client disconnected
  function ez_usb_dev_read(timeout_value : integer) return integer;
  attribute foreign of ez_usb_dev_read : function is "VHPIDIRECT ez_usb_dev_read";

  procedure ez_usb_dev_write(x : integer);
  attribute foreign of ez_usb_dev_write : procedure is "VHPIDIRECT ez_usb_dev_write";

  procedure ez_usb_dev_flush;
  attribute foreign of ez_usb_dev_flush : procedure is "VHPIDIRECT ez_usb_dev_flush";


  shared variable my_var : integer := 43;
  shared variable TIMEOUT : integer := -1;
  shared variable HANGUP : integer := -2;
end package;

package body ez_usb_dev is

  procedure ez_usb_dev_init(stop_unitl_connected : boolean) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  function ez_usb_dev_read(timeout_value : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
    return 0;
  end function;

  procedure ez_usb_dev_write(x : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure ez_usb_dev_flush is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

end package body;
