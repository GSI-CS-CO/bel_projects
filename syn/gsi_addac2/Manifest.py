target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "scu_addac2"
syn_project = "scu_addac2"

modules = {
  "local" : [ 
    "../../top/gsi_addac2/", 
  ]
}
syn_tool = "quartus"
