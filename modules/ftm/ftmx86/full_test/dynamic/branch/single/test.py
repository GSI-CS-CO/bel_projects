testcase = TestCase(
  "Test_Basic_Branching",
  [ Op("Init0_0", "cmd", "reset all"),
    Op("Init0_1", "sched", "add", "0", "test_sched.dot"),
    Op("Test0_0_RunCPU0", "cmd", "startpattern IN_C0"),
    Op("Test0_0_CheckA", "sched", "rawvisited", "0.0", None, "test0_0_exp.txt"),
    Op("Test1_0_Flow", "cmd", "flowpattern IN_C0 B"),
    Op("Test1_1_Cmd_Pen", "cmd", "rawqueue BLOCK_IN0", "0", None, "test1_1_exp.txt"),
    Op("Test1_2_RunCPU0", "cmd", "startpattern IN_C0"),
    Op("Test1_3_CheckAB", "sched", "rawvisited", "0.0", None, "test1_3_exp.txt"),
    Op("Test1_4_Cmd_Exh", "cmd", "rawqueue BLOCK_IN0", "0", None, "test1_4_exp.txt"),
  ]
)