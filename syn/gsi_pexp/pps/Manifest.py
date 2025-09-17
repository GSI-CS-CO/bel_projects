target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "pexp_pps"
syn_project = "pexp_pps"

quartus_preflow = "pexp_pps.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexp/pps",
  ]
}

syn_tool = "quartus"
