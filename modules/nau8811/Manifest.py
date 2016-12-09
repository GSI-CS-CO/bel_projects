def __audio_check():
  syn_files = []
  if syn_device[:1] == "5": syn_files = [ "src/hdl/generic_iis_master_pkg.vhd", "src/hdl/generic_iis_master.vhd", "src/hdl/wb_nau8811_audio_driver_pkg.vhd", "src/hdl/wb_nau8811_audio_driver.vhd", "src/hdl/altera_pll/audio_pll_ref.qip" ]
  else:                     syn_files = [ "src/hdl/wb_nau8811_audio_driver_pkg.vhd" ]
  return syn_files

files = __audio_check()
