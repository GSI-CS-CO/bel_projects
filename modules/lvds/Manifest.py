def __helper():
  dirs = []
  if syn_device[:1] == "5":    dirs.extend(["arria5"])
  return dirs
  
modules = {"local": __helper() }

files = [ 
  "altera_lvds.vhd",
  "altera_lvds_pkg.vhd", 
  "arria5_lvds_pkg.vhd",
  "altera_lvds_ibuf.vhd",
  "altera_lvds_obuf.vhd",
  "altera_lvds_rx.vhd",
  "altera_lvds_tx.vhd",
  "eca_lvds_channel.vhd"
  ]
