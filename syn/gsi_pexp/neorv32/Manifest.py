target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "pexp_neorv32"
syn_project = "pexp_neorv32"

quartus_preflow = "pexp_neorv32.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexp/neorv32",
  ]
}

syn_tool = "quartus"
