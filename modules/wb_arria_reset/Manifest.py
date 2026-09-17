def __helper():
    dirs = []
    dirs.extend(["arria10_reset/arria10_reset/altera_remote_update_231"])
    return dirs

files = [
  "arria_reset.vhd",
  "arria5_reset.vhd",
  "wb_arria_reset_pkg.vhd",
  "wb_arria_reset.vhd",
  "arria10_reset/arria10_reset/synth/arria10_reset.vhd"
]
modules = { "local": __helper() }
