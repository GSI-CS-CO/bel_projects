testcase = TestCase(
  "Upload_of_coupled_Schedules",
  [ Op("Init0_0", "dm-cmd", "halt"),
    Op("Init0_1", "dm-sched", "clear"),
  	Op("Test0_0_add_first", "dm-sched", "add", "0", "test0_sched.dot"),
    Op("Test0_1_add_2nd_and_coupling", "dm-sched", "add", "0", "test1_sched.dot"),
    Op("Test0_2_coupled", "dm-sched", "status", "0", None, None),
    Op("Test0_3_diff", "diff", "download.dot", "0", "comp.dot", "test0_0_exp.txt"),
  ]
)