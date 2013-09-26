target = "altera"
action = "synthesis"



syn_device = "ep2agx125ef"
syn_grade = "c6"
syn_package = "25"
syn_top = "scu_addac"
syn_project = "scu_addac"

modules = {
  "local" : [
    "../../top/gsi_addac",
  ]
}
