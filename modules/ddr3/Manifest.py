def __helper():
  dirs = []
  if syn_device[:4] == "ep2a":    dirs.extend(["arria2"])
  return dirs

files = [ "ddr3_wrapper_pkg.vhd", "ddr3_wrapper.vhd" ]

modules = {"local": __helper() }
