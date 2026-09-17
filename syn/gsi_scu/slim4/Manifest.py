target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "29"
syn_device = "10ax027e3f"
syn_top = "scu4slim"
syn_project = "scu4slim"
syn_family = "Arria 10"

quartus_preflow = "scu4slim.tcl"

modules = {
  "local" : [
    "../../../top/gsi_scu/slim4",
  ]
}
