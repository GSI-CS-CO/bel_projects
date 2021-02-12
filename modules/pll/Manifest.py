def __helper():
  dirs = []
  if syn_device[:4] == "10as":      dirs.extend(["arria10"])
  if syn_device[:9] == "10ax027e3": dirs.extend(["arria10_scu4"])
  if syn_device[:9] == "10ax027h2": dirs.extend(["arria10_pex10"])
  if syn_device[:9] == "10ax066h2": dirs.extend(["arria10_ftm10"])
  if syn_device[:7] == "10ax066":   dirs.extend(["arria10"])
  if syn_device[:7] == "10ax115":   dirs.extend(["arria10_e3p1"])
  if syn_device[:1] == "5":         dirs.extend(["arria5"])
  if syn_device[:4] == "ep2a":      dirs.extend(["arria2"])
  return dirs

files = [ "pll_pkg.vhd", "altera_butis.vhd", "altera_phase.vhd", "altera_reset.vhd" ]
modules = {"local": __helper() }

