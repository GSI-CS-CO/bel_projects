target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "vetar2a_top"
syn_project = "vetar2a"

quartus_preflow = "vetar2a_top.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_vetar2a/wr_core_demo",
  ]
}
syn_tool = "quartus"
