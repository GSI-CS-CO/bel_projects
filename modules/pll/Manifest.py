def __helper():
  dirs = []
  if syn_device[:1] == "5":    dirs.extend(["arria5"])
  if syn_device[:4] == "ep2a": dirs.extend(["arria2"])
  return dirs
  
files = [ "pll_pkg.vhd", "altera_butis.vhd", "altera_phase.vhd", "altera_reset.vhd" ]
modules = {"local": __helper() }
