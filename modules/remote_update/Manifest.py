def __helper():
  dirs = []
  dirs.extend(["asmi10/asmi10/altera_asmi_parallel_181"])
  return dirs

files = [ 
  "remote_update_pkg.vhd",
  "remote_update.vhd",
  "altasmi.vhd",
  "wb_remote_update.vhd",
  "wb_asmi.vhd"
]

modules = {"local": __helper() }
