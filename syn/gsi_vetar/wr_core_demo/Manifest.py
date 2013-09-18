target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "vetar_top"
syn_project = "vetar

modules = {"local" : [ "../../../", "../../../top/gsi_vetar/wr_core_demo"]}
