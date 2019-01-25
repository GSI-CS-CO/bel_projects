testcase = TestCase(
  "Test0_Basic_Run_CPU0",
  [ Op("Init0_1", "cmd", "halt"),
    Op("Init0_1", "sched", "clear"),
    Op("Init0_2", "sched", "add", "0", "test_sched.dot"),
    Op("Test0_0_CheckNone", "sched", "rawvisited", "0.0", None, "test0_0_exp.txt"),
    Op("Test1_0_RunIN0", "cmd", "startpattern IN0"),
    Op("Test1_1_CheckIN0", "sched", "rawvisited", "0.1", None, "test1_1_exp.txt"),
    Op("Test2_0_RunIN1", "cmd", "startpattern IN1"),
    Op("Test2_1_CheckIN01", "sched", "rawvisited", "0.1", None, "test2_1_exp.txt"),
  ]
)
