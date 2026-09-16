def __helper():
  dirs = []
  # asmi10 is Quartus 23.1 / Arria 10 IP; Quartus 18.1 cannot analyse it.
  if syn_device[:2] == "10":
    dirs.extend(["asmi10/asmi10/altera_asmi_parallel_231"])
  return dirs

def __files():
  syn_files = [
    "remote_update_pkg.vhd",
    "remote_update.vhd",
    "wb_remote_update.vhd",
    "wb_asmi.vhd",
    "wb_asmi_slave.vhd",
    "asmi5/asmi5/synthesis/asmi5.vhd",
    "asmi5/asmi5/synthesis/submodules/asmi5_asmi_parallel_0.v",
    "asmi_arriaII/asmi_arriaII/synthesis/asmi_arriaII.vhd",
    "asmi_arriaII/asmi_arriaII/synthesis/submodules/asmi_arriaII_asmi_parallel_0.v",
    "altasmi.vhd"
  ]
  if syn_device[:2] == "10":
    syn_files.append("asmi10/asmi10/synth/asmi10.vhd")
  return syn_files

files = __files()
modules = {"local": __helper() }
