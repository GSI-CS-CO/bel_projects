def __helper():
  dirs = []
  dirs.extend(["asmi10/asmi10/altera_asmi_parallel_181"])
  return dirs

files = [ 
  "remote_update_pkg.vhd",
  "remote_update.vhd",
  "wb_remote_update.vhd",
  "wb_asmi.vhd",
  "asmi10/asmi10/synth/asmi10.vhd",
  "asmi10/asmi10/altera_asmi_parallel_181/synth/asmi10_pkg.vhd",
  "asmi10/asmi10/altera_asmi_parallel_181/synth/asmi10_altera_asmi_parallel_181_svbigkq.v",
  "asmi5/asmi5/synthesis/asmi5.vhd",
  "asmi5/asmi5/synthesis/submodules/asmi5_asmi_parallel_0.v",
  "asmi_arriaII/asmi_arriaII/synthesis/asmi_arriaII.vhd",
  "asmi_arriaII/asmi_arriaII/synthesis/submodules/asmi_arriaII_asmi_parallel_0.v"
]

modules = {"local": __helper() }
