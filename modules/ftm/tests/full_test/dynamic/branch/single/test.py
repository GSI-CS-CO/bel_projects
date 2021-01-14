testcase = TestCase(
  "Basic_Branching_single_execution",
  [ Op("Init0_0", "dm-cmd", "reset all"),
    Op("Init0_1", "dm-sched", "add", "0", "test_sched.dot"),
    Op("Test0_0_RunCPU0", "dm-cmd", "startpattern IN_C0"),
    Op("Test0_0_CheckA", "dm-sched", "rawvisited", "0.0", None, "test0_0_exp.txt"),
    Op("Test1_0_Flow", "dm-cmd", "flowpattern IN_C0 B"),
    Op("Test1_1_Cmd_Pen", "dm-cmd", "rawqueue BLOCK_IN0", "0", None, "test1_1_exp.txt"),
    Op("Test1_2_RunCPU0", "dm-cmd", "startpattern IN_C0"),
    Op("Test1_3_CheckAB", "dm-sched", "rawvisited", "0.0", None, "test1_3_exp.txt"),
    Op("Test1_4_Cmd_Exh", "dm-cmd", "rawqueue BLOCK_IN0", "0", None, "test1_4_exp.txt"),
  ]
)